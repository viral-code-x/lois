#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TokenList *tokens;
static int position;

static Token *current(void)
{
    return &tokens->tokens[position];
}

static int is_word(const char *word)
{
    return current()->type == TOKEN_WORD &&
           strcmp(current()->text, word) == 0;
}

static void advance(void)
{
    if (current()->type != TOKEN_EOF)
        position++;
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

/*
 * Normal math expression:
 *
 * age + 1
 * age - 1
 * age * 2
 * age / 2
 */
static Expr *parse_math_expression(void)
{
    Expr *left = parse_primary();

    while (
        current()->type == TOKEN_PLUS ||
        current()->type == TOKEN_MINUS ||
        current()->type == TOKEN_STAR ||
        current()->type == TOKEN_SLASH
    )
    {
        TokenType operator = current()->type;

        advance();

        Expr *right = parse_primary();

        if (!right)
            break;

        Expr *binary = new_expr(EXPR_BINARY);

        binary->operator = operator;
        binary->left = left;
        binary->right = right;

        left = binary;
    }

    return left;
}

/*
 * Output expression.
 *
 * This allows:
 *
 * output is hello name.
 *
 * and:
 *
 * output is age + 1.
 *
 * Adjacent words are treated like concatenation.
 */
static Expr *parse_output_expression(void)
{
    Expr *left = parse_primary();

    if (!left)
        return NULL;

    while (current()->type != TOKEN_DOT &&
           current()->type != TOKEN_EOF)
    {
        /*
         * Explicit math operator.
         */
        if (
            current()->type == TOKEN_PLUS ||
            current()->type == TOKEN_MINUS ||
            current()->type == TOKEN_STAR ||
            current()->type == TOKEN_SLASH
        )
        {
            TokenType operator = current()->type;

            advance();

            Expr *right = parse_primary();

            if (!right)
                break;

            Expr *binary = new_expr(EXPR_BINARY);

            binary->operator = operator;
            binary->left = left;
            binary->right = right;

            left = binary;

            continue;
        }

        /*
         * Another word/string directly after the
         * previous expression means concatenation.
         *
         * Example:
         *
         * hello name
         *
         * becomes:
         *
         * hello + name
         */
        if (
            current()->type == TOKEN_WORD ||
            current()->type == TOKEN_STRING ||
            current()->type == TOKEN_NUMBER
        )
        {
            Expr *right = parse_primary();

            Expr *binary = new_expr(EXPR_BINARY);

            binary->operator = TOKEN_PLUS;
            binary->left = left;
            binary->right = right;

            left = binary;

            continue;
        }

        break;
    }

    return left;
}

static void consume_dot(void)
{
    if (current()->type == TOKEN_DOT)
        advance();
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
 * output is hello name.
 *
 * output is "hello name".
 *
 * output is age + 1.
 */
static Statement *parse_output(void)
{
    advance(); /* output */

    if (is_word("is"))
        advance();

    Statement *statement =
        new_statement(STMT_OUTPUT);

    statement->expression =
        parse_output_expression();

    consume_dot();

    return statement;
}

/*
 * input is name.
 */
static Statement *parse_input(void)
{
    advance(); /* input */

    if (is_word("is"))
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
 * age is 18.
 *
 * name is aaron.
 *
 * age is num.
 */
static Statement *parse_assignment(void)
{
    char *name =
        strdup(current()->text);

    advance();

    if (!is_word("is"))
    {
        free(name);
        return NULL;
    }

    advance(); /* is */

    Statement *statement =
        new_statement(STMT_ASSIGN);

    statement->name = name;

    /*
     * age is num.
     */
    if (is_word("num"))
    {
        statement->extra =
            strdup("num");

        advance();

        consume_dot();

        return statement;
    }

    /*
     * Everything else is an expression.
     */
    statement->expression =
        parse_math_expression();

    consume_dot();

    return statement;
}

/*
 * if age is >18
 * then output is "you are an adult".
 */
static Statement *parse_if(void)
{
    advance(); /* if */

    Statement *statement =
        new_statement(STMT_IF);

    /*
     * Left side.
     *
     * age
     */
    Expr *left = parse_primary();

    /*
     * LOIS:
     *
     * age is >18
     */
    if (is_word("is"))
        advance();

    TokenType operator =
        current()->type;

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
            "LOIS: expected comparison operator after 'is'\n"
        );

        parser_free(statement);

        return NULL;
    }

    Expr *right = parse_primary();

    Expr *comparison =
        new_expr(EXPR_BINARY);

    comparison->operator = operator;
    comparison->left = left;
    comparison->right = right;

    statement->condition = comparison;

    consume_dot();

    /*
     * then output is ...
     */
    if (is_word("then"))
    {
        advance();

        statement->body =
            parse_output();
    }

    return statement;
}

static Statement *parse_statement(void)
{
    if (is_word("output"))
        return parse_output();

    if (is_word("input"))
        return parse_input();

    if (is_word("if"))
        return parse_if();

    if (current()->type == TOKEN_WORD)
        return parse_assignment();

    fprintf(
        stderr,
        "LOIS parser: unexpected token\n"
    );

    advance();

    return NULL;
}

Statement *parser_parse(TokenList *token_list)
{
    tokens = token_list;
    position = 0;

    Statement *first = NULL;
    Statement *last = NULL;

    while (current()->type != TOKEN_EOF)
    {
        Statement *statement =
            parse_statement();

        if (!statement)
            continue;

        if (!first)
        {
            first = statement;
        }
        else
        {
            last->next = statement;
        }

        last = statement;
    }

    return first;
}

static void free_expression(Expr *expr)
{
    if (!expr)
        return;

    free_expression(expr->left);
    free_expression(expr->right);

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

        free_expression(
            statement->expression
        );

        free_expression(
            statement->condition
        );

        parser_free(
            statement->body
        );

        free(statement);

        statement = next;
    }
}
