#define _GNU_SOURCE

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static TokenList *tokens;
static int position;

static Token *current(void)
{
    return &tokens->tokens[position];
}

static void advance(void)
{
    if (current()->type != TOKEN_EOF)
        position++;
}

static int word_is(const char *word)
{
    return current()->type == TOKEN_WORD &&
           strcasecmp(current()->text, word) == 0;
}

static void skip_newlines(void)
{
    while (current()->type == TOKEN_NEWLINE)
        advance();
}

static void consume_dot(void)
{
    if (current()->type == TOKEN_DOT)
        advance();
}

static Expr *new_expr(ExprType type)
{
    Expr *expr = calloc(1, sizeof(Expr));

    if (!expr)
    {
        fprintf(stderr, "LOIS: out of memory\n");
        exit(1);
    }

    expr->type = type;

    return expr;
}

static Expr *parse_primary(void)
{
    Token *token = current();

    if (token->type == TOKEN_STRING)
    {
        Expr *expr = new_expr(EXPR_LITERAL);

        expr->text = strdup(token->text);

        advance();

        return expr;
    }

    if (token->type == TOKEN_NUMBER)
    {
        Expr *expr = new_expr(EXPR_NUMBER);

        expr->number = token->number;

        advance();

        return expr;
    }

    if (token->type == TOKEN_WORD)
    {
        Expr *expr = new_expr(EXPR_VARIABLE);

        expr->text = strdup(token->text);

        advance();

        return expr;
    }

    return NULL;
}

static int is_operator(TokenType type)
{
    return
        type == TOKEN_PLUS ||
        type == TOKEN_MINUS ||
        type == TOKEN_STAR ||
        type == TOKEN_SLASH;
}

static Expr *make_binary(
    Expr *left,
    Expr *right,
    TokenType operator
)
{
    Expr *expr = new_expr(EXPR_BINARY);

    expr->left = left;
    expr->right = right;
    expr->operator = operator;

    return expr;
}

/*
 * Parses:
 *
 * age + 1
 * age - 1
 * age * 2
 * age / 2
 */
static Expr *parse_expression(void)
{
    Expr *left = parse_primary();

    if (!left)
        return NULL;

    while (is_operator(current()->type))
    {
        TokenType operator = current()->type;

        advance();

        Expr *right = parse_primary();

        if (!right)
            break;

        left = make_binary(
            left,
            right,
            operator
        );
    }

    return left;
}

/*
 * Output can contain multiple pieces:
 *
 * hello name
 *
 * hello     name
 *
 * age + 1
 *
 * A quoted string remains one literal.
 */
static Expr *parse_output(void)
{
    Expr *left = parse_expression();

    if (!left)
        return NULL;

    while (
        current()->type != TOKEN_DOT &&
        current()->type != TOKEN_NEWLINE &&
        current()->type != TOKEN_EOF
    )
    {
        /*
         * Math operator.
         */
        if (is_operator(current()->type))
        {
            TokenType operator = current()->type;

            advance();

            Expr *right = parse_primary();

            if (!right)
                break;

            left = make_binary(
                left,
                right,
                operator
            );

            continue;
        }

        /*
         * Adjacent output pieces.
         *
         * hello name
         *
         * becomes:
         *
         * hello + name
         *
         * The interpreter later inserts
         * a space between them.
         */
        if (
            current()->type == TOKEN_WORD ||
            current()->type == TOKEN_STRING ||
            current()->type == TOKEN_NUMBER
        )
        {
            Expr *right = parse_primary();

            left = make_binary(
                left,
                right,
                TOKEN_PLUS
            );

            continue;
        }

        break;
    }

    return left;
}

static Statement *new_statement(StatementType type)
{
    Statement *statement =
        calloc(1, sizeof(Statement));

    if (!statement)
    {
        fprintf(stderr, "LOIS: out of memory\n");
        exit(1);
    }

    statement->type = type;

    return statement;
}

/*
 * output is ...
 */
static Statement *parse_output_statement(void)
{
    advance(); /* output */

    if (word_is("is"))
        advance();

    Statement *statement =
        new_statement(STMT_OUTPUT);

    statement->expression =
        parse_output();

    consume_dot();

    return statement;
}

/*
 * input name.
 *
 * input is name.
 */
static Statement *parse_input_statement(void)
{
    advance(); /* input */

    if (word_is("is"))
        advance();

    Statement *statement =
        new_statement(STMT_INPUT);

    if (current()->type == TOKEN_WORD)
    {
        statement->name =
            strdup(current()->text);

        advance();
    }

    consume_dot();

    return statement;
}

/*
 * name is Alex.
 *
 * age is 19.
 *
 * age is num.
 */
static Statement *parse_assignment_statement(void)
{
    if (current()->type != TOKEN_WORD)
        return NULL;

    char *name =
        strdup(current()->text);

    advance();

    if (!word_is("is"))
    {
        free(name);
        return NULL;
    }

    advance(); /* is */

    Statement *statement =
        new_statement(STMT_ASSIGN);

    statement->name = name;

    /*
     * Type declaration:
     *
     * age is num.
     */
    if (word_is("num"))
    {
        statement->extra =
            strdup("num");

        advance();

        consume_dot();

        return statement;
    }

    /*
     * Assignment value.
     *
     * IMPORTANT:
     *
     * If the value is a word such as Alex,
     * it becomes a literal string.
     *
     * So:
     *
     * name is Alex.
     *
     * means name = "Alex"
     */
    if (current()->type == TOKEN_STRING)
    {
        statement->expression =
            new_expr(EXPR_LITERAL);

        statement->expression->text =
            strdup(current()->text);

        advance();
    }
    else if (current()->type == TOKEN_NUMBER)
    {
        statement->expression =
            new_expr(EXPR_NUMBER);

        statement->expression->number =
            current()->number;

        advance();
    }
    else if (current()->type == TOKEN_WORD)
    {
        statement->expression =
            new_expr(EXPR_LITERAL);

        statement->expression->text =
            strdup(current()->text);

        advance();
    }

    consume_dot();

    return statement;
}

/*
 * if age is >18
 * then output is "adult".
 */
static Statement *parse_if_statement(void)
{
    advance(); /* if */

    Statement *statement =
        new_statement(STMT_IF);

    /*
     * Left side.
     */
    Expr *left = parse_primary();

    if (!left)
        return statement;

    /*
     * "is"
     */
    if (word_is("is"))
        advance();

    TokenType operator = current()->type;

    /*
     * Comparison operator.
     */
    if (
        operator == TOKEN_GREATER ||
        operator == TOKEN_LESS ||
        operator == TOKEN_GREATER_EQUAL ||
        operator == TOKEN_LESS_EQUAL ||
        operator == TOKEN_EQUAL_EQUAL ||
        operator == TOKEN_NOT_EQUAL
    )
    {
        advance();
    }
    else
    {
        fprintf(
            stderr,
            "LOIS: expected comparison operator at %d:%d\n",
            current()->line,
            current()->column
        );

        return statement;
    }

    Expr *right = parse_primary();

    statement->condition =
        make_binary(
            left,
            right,
            operator
        );

    /*
     * The condition ends at newline.
     */
    while (
        current()->type != TOKEN_NEWLINE &&
        current()->type != TOKEN_EOF
    )
    {
        advance();
    }

    skip_newlines();

    /*
     * then
     */
    if (word_is("then"))
        advance();

    skip_newlines();

    /*
     * Parse the statement after then.
     */
    if (word_is("output"))
    {
        statement->body =
            parse_output_statement();
    }

    return statement;
}

static Statement *parse_statement(void)
{
    skip_newlines();

    if (current()->type == TOKEN_EOF)
        return NULL;

    if (word_is("output"))
        return parse_output_statement();

    if (word_is("input"))
        return parse_input_statement();

    if (word_is("if"))
        return parse_if_statement();

    if (current()->type == TOKEN_WORD)
        return parse_assignment_statement();

    fprintf(
        stderr,
        "LOIS parser: unexpected token at %d:%d\n",
        current()->line,
        current()->column
    );

    advance();

    return NULL;
}

Statement *parser_parse(TokenList *token_list)
{
    tokens = token_list;
    position = 0;

    Statement *head = NULL;
    Statement *tail = NULL;

    while (current()->type != TOKEN_EOF)
    {
        Statement *statement =
            parse_statement();

        if (!statement)
            continue;

        if (!head)
        {
            head = statement;
            tail = statement;
        }
        else
        {
            tail->next = statement;
            tail = statement;
        }
    }

    return head;
}

static void free_expr(Expr *expr)
{
    if (!expr)
        return;

    free_expr(expr->left);
    free_expr(expr->right);

    free(expr->text);

    free(expr);
}

void parser_free(Statement *statement)
{
    while (statement)
    {
        Statement *next =
            statement->next;

        free(statement->name);
        free(statement->extra);

        free_expr(statement->expression);
        free_expr(statement->condition);

        parser_free(statement->body);

        free(statement);

        statement = next;
    }
}
