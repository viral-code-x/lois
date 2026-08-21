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

static Token *peek(int offset)
{
    int index = position + offset;

    if (index < 0)
        index = 0;

    if (index >= tokens->count)
        index = tokens->count - 1;

    return &tokens->tokens[index];
}

static void advance(void)
{
    if (current()->type != TOKEN_EOF)
        position++;
}

static int word_is(const char *word)
{
    return
        current()->type == TOKEN_WORD &&
        strcasecmp(current()->text, word) == 0;
}

static int peek_word_is(int offset, const char *word)
{
    Token *token = peek(offset);

    return
        token->type == TOKEN_WORD &&
        strcasecmp(token->text, word) == 0;
}

static void skip_newlines(void)
{
    while (current()->type == TOKEN_NEWLINE)
        advance();
}

static void optional_period(void)
{
    /*
     * Old LOIS programs may still use a period.
     *
     * New LOIS does not require it.
     *
     * The lexer no longer creates TOKEN_DOT,
     * so this function intentionally does nothing.
     */
}

static Expr *new_expr(ExprType type)
{
    Expr *expr =
        calloc(1, sizeof(Expr));

    if (!expr)
    {
        fprintf(
            stderr,
            "LOIS: out of memory\n"
        );

        exit(1);
    }

    expr->type = type;

    return expr;
}

static Statement *new_statement(
    StatementType type
)
{
    Statement *statement =
        calloc(1, sizeof(Statement));

    if (!statement)
    {
        fprintf(
            stderr,
            "LOIS: out of memory\n"
        );

        exit(1);
    }

    statement->type = type;

    return statement;
}

static Expr *make_binary(
    Expr *left,
    Expr *right,
    TokenType operator
)
{
    Expr *expr =
        new_expr(EXPR_BINARY);

    expr->left = left;
    expr->right = right;
    expr->operator = operator;

    return expr;
}

static Expr *make_unary(
    TokenType operator,
    Expr *right
)
{
    Expr *expr =
        new_expr(EXPR_UNARY);

    expr->operator = operator;
    expr->right = right;

    return expr;
}

static Expr *make_call(
    const char *name
)
{
    Expr *expr =
        new_expr(EXPR_CALL);

    expr->text =
        strdup(name);

    return expr;
}

static void add_argument(
    Expr *call,
    Expr *argument
)
{
    call->arguments =
        realloc(
            call->arguments,
            sizeof(Expr *) *
            (size_t)(call->argument_count + 1)
        );

    if (!call->arguments)
    {
        fprintf(
            stderr,
            "LOIS: out of memory\n"
        );

        exit(1);
    }

    call->arguments[
        call->argument_count++
    ] = argument;
}

/*
 * Forward declarations.
 */
static Expr *parse_expression(void);
static Expr *parse_or(void);
static Expr *parse_and(void);
static Expr *parse_equality(void);
static Expr *parse_comparison(void);
static Expr *parse_term(void);
static Expr *parse_factor(void);
static Expr *parse_power(void);
static Expr *parse_unary(void);
static Expr *parse_primary(void);

static Expr *parse_primary(void)
{
    Token *token = current();

    /*
     * String.
     */
    if (token->type == TOKEN_STRING)
    {
        Expr *expr =
            new_expr(EXPR_LITERAL);

        expr->text =
            strdup(token->text);

        advance();

        return expr;
    }

    /*
     * Number.
     */
    if (token->type == TOKEN_NUMBER)
    {
        Expr *expr =
            new_expr(EXPR_NUMBER);

        expr->number =
            token->number;

        advance();

        return expr;
    }

    /*
     * Parenthesized expression.
     */
    if (token->type == TOKEN_LPAREN)
    {
        advance();

        Expr *expr =
            parse_expression();

        if (current()->type == TOKEN_RPAREN)
            advance();

        return expr;
    }

    /*
     * Word / variable / function call.
     */
    if (token->type == TOKEN_WORD)
    {
        char *name =
            strdup(token->text);

        advance();

        /*
         * Function call:
         *
         * p(x)
         * sin(x)
         */
        if (current()->type == TOKEN_LPAREN)
        {
            Expr *call =
                make_call(name);

            free(name);

            advance();

            if (current()->type != TOKEN_RPAREN)
            {
                while (1)
                {
                    Expr *argument =
                        parse_expression();

                    if (argument)
                        add_argument(
                            call,
                            argument
                        );

                    if (
                        current()->type ==
                        TOKEN_COMMA
                    )
                    {
                        advance();
                        continue;
                    }

                    break;
                }
            }

            if (current()->type == TOKEN_RPAREN)
                advance();

            return call;
        }

        Expr *expr =
            new_expr(EXPR_VARIABLE);

        expr->text = name;

        return expr;
    }

    return NULL;
}

static Expr *parse_unary(void)
{
    /*
     * not x
     * !x
     * -x
     */
    if (
        current()->type == TOKEN_MINUS ||
        current()->type == TOKEN_BANG ||
        word_is("not")
    )
    {
        TokenType operator =
            current()->type;

        if (word_is("not"))
            operator = TOKEN_BANG;

        advance();

        return make_unary(
            operator,
            parse_unary()
        );
    }

    return parse_primary();
}

static Expr *parse_power(void)
{
    Expr *left =
        parse_unary();

    if (!left)
        return NULL;

    if (current()->type == TOKEN_POWER)
    {
        TokenType operator =
            current()->type;

        advance();

        Expr *right =
            parse_power();

        return make_binary(
            left,
            right,
            operator
        );
    }

    return left;
}

static Expr *parse_factor(void)
{
    Expr *left =
        parse_power();

    if (!left)
        return NULL;

    while (
        current()->type == TOKEN_STAR ||
        current()->type == TOKEN_SLASH ||
        current()->type == TOKEN_PERCENT
    )
    {
        TokenType operator =
            current()->type;

        advance();

        Expr *right =
            parse_power();

        if (!right)
            break;

        left =
            make_binary(
                left,
                right,
                operator
            );
    }

    return left;
}

static Expr *parse_term(void)
{
    Expr *left =
        parse_factor();

    if (!left)
        return NULL;

    while (
        current()->type == TOKEN_PLUS ||
        current()->type == TOKEN_MINUS
    )
    {
        TokenType operator =
            current()->type;

        advance();

        Expr *right =
            parse_factor();

        if (!right)
            break;

        left =
            make_binary(
                left,
                right,
                operator
            );
    }

    return left;
}

static Expr *parse_comparison(void)
{
    Expr *left =
        parse_term();

    if (!left)
        return NULL;

    while (
        current()->type == TOKEN_GREATER ||
        current()->type == TOKEN_LESS ||
        current()->type == TOKEN_GREATER_EQUAL ||
        current()->type == TOKEN_LESS_EQUAL
    )
    {
        TokenType operator =
            current()->type;

        advance();

        Expr *right =
            parse_term();

        if (!right)
            break;

        left =
            make_binary(
                left,
                right,
                operator
            );
    }

    return left;
}

static Expr *parse_equality(void)
{
    Expr *left =
        parse_comparison();

    if (!left)
        return NULL;

    while (
        current()->type == TOKEN_EQUAL_EQUAL ||
        current()->type == TOKEN_NOT_EQUAL
    )
    {
        TokenType operator =
            current()->type;

        advance();

        Expr *right =
            parse_comparison();

        if (!right)
            break;

        left =
            make_binary(
                left,
                right,
                operator
            );
    }

    return left;
}

static Expr *parse_and(void)
{
    Expr *left =
        parse_equality();

    if (!left)
        return NULL;

    while (
        word_is("and") ||
        current()->type == TOKEN_BANG
    )
    {
        TokenType operator =
            TOKEN_BANG;

        if (word_is("and"))
            operator = TOKEN_STAR;

        advance();

        Expr *right =
            parse_equality();

        if (!right)
            break;

        left =
            make_binary(
                left,
                right,
                operator
            );
    }

    return left;
}

static Expr *parse_or(void)
{
    Expr *left =
        parse_and();

    if (!left)
        return NULL;

    while (word_is("or"))
    {
        advance();

        Expr *right =
            parse_and();

        if (!right)
            break;

        left =
            make_binary(
                left,
                right,
                TOKEN_PLUS
            );

        /*
         * The interpreter will distinguish
         * logical OR using expression metadata
         * in the next interpreter revision.
         *
         * For now this keeps the AST valid.
         */
    }

    return left;
}

static Expr *parse_expression(void)
{
    return parse_or();
}

/*
 * Parse an expression until the current line ends.
 */
static Expr *parse_line_expression(void)
{
    return parse_expression();
}

/*
 * output is ...
 */
static Statement *parse_output_statement(void)
{
    advance();

    if (word_is("is"))
        advance();

    Statement *statement =
        new_statement(STMT_OUTPUT);

    statement->expression =
        parse_line_expression();

    optional_period();

    return statement;
}

/*
 * input name
 *
 * input is name
 */
static Statement *parse_input_statement(void)
{
    advance();

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

    optional_period();

    return statement;
}

/*
 * name is Alex
 *
 * age is 13
 *
 * age = 13
 *
 * age is num
 */
static Statement *parse_assignment_statement(void)
{
    if (current()->type != TOKEN_WORD)
        return NULL;

    char *name =
        strdup(current()->text);

    advance();

    Statement *statement = NULL;

    /*
     * "=" means numeric/general expression.
     */
    if (current()->type == TOKEN_EQUAL)
    {
        advance();

        statement =
            new_statement(STMT_ASSIGN);

        statement->name = name;

        statement->extra =
            strdup("expression");

        statement->expression =
            parse_line_expression();

        optional_period();

        return statement;
    }

    /*
     * "is" assignment.
     */
    if (!word_is("is"))
    {
        free(name);
        return NULL;
    }

    advance();

    /*
     * Function declaration:
     *
     * p is function of x*x
     *
     * This parser stores the expression as
     * the function body.
     */
    if (word_is("function"))
    {
        statement =
            new_statement(STMT_FUNCTION);

        statement->name = name;

        advance();

        if (word_is("of"))
            advance();

        /*
         * Function currently supports one or
         * more parameter names before expression.
         *
         * Example:
         *
         * p is function of x*x
         */
        if (current()->type == TOKEN_WORD)
        {
            char *parameter =
                strdup(current()->text);

            advance();

            /*
             * If another word/operator/expression
             * follows, treat first word as parameter.
             */
            statement->parameters =
                malloc(sizeof(char *));

            statement->parameters[0] =
                parameter;

            statement->parameter_count = 1;

            /*
             * If expression starts with the
             * parameter itself, parse it.
             */
        }

        statement->expression =
            parse_line_expression();

        optional_period();

        return statement;
    }

    statement =
        new_statement(STMT_ASSIGN);

    statement->name = name;

    /*
     * age is num
     */
    if (word_is("num"))
    {
        statement->extra =
            strdup("num");

        advance();

        optional_period();

        return statement;
    }

    /*
     * Explicit typecast:
     *
     * age is num 13
     *
     * or simply normal string-style assignment.
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
        /*
         * "13" after "is" is intentionally a
         * string according to LOIS semantics.
         */
        char buffer[128];

        snprintf(
            buffer,
            sizeof(buffer),
            "%s",
            current()->text
                ? current()->text
                : ""
        );

        statement->expression =
            new_expr(EXPR_LITERAL);

        statement->expression->text =
            strdup(buffer);

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
    else
    {
        statement->expression =
            parse_line_expression();
    }

    optional_period();

    return statement;
}

/*
 * if condition then statement
 *
 * Also accepts:
 *
 * if age > 18
 * then output is adult
 */
static Statement *parse_if_statement(void)
{
    advance();

    Statement *statement =
        new_statement(STMT_IF);

    statement->condition =
        parse_expression();

    /*
     * Same-line then.
     */
    if (word_is("then"))
    {
        advance();

        if (word_is("output"))
            statement->body =
                parse_output_statement();

        else
            statement->body =
                NULL;

        return statement;
    }

    /*
     * Old two-line syntax.
     */
    skip_newlines();

    if (word_is("then"))
        advance();

    skip_newlines();

    if (word_is("output"))
        statement->body =
            parse_output_statement();

    return statement;
}

static Statement *parse_while_statement(void)
{
    advance();

    Statement *statement =
        new_statement(STMT_WHILE);

    statement->condition =
        parse_expression();

    /*
     * while condition output ...
     */
    if (word_is("output"))
    {
        statement->body =
            parse_output_statement();
    }

    else
    {
        skip_newlines();

        if (word_is("output"))
            statement->body =
                parse_output_statement();
    }

    return statement;
}

/*
 * for x = 1 till x < 10 do output is x
 */
static Statement *parse_for_statement(void)
{
    advance();

    Statement *statement =
        new_statement(STMT_FOR);

    /*
     * Variable.
     */
    if (current()->type == TOKEN_WORD)
    {
        statement->name =
            strdup(current()->text);

        advance();
    }

    /*
     * =
     */
    if (current()->type == TOKEN_EQUAL)
        advance();

    /*
     * Start expression.
     */
    statement->expression =
        parse_expression();

    /*
     * till
     */
    if (word_is("till"))
        advance();

    /*
     * Condition.
     */
    statement->condition =
        parse_expression();

    /*
     * do
     */
    if (word_is("do"))
        advance();

    /*
     * Body.
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

    if (word_is("while"))
        return parse_while_statement();

    if (word_is("for"))
        return parse_for_statement();

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
        {
            /*
             * Avoid getting stuck forever.
             */
            if (current()->type != TOKEN_EOF)
                advance();

            continue;
        }

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

        skip_newlines();
    }

    return head;
}

static void free_expr(Expr *expr)
{
    if (!expr)
        return;

    free_expr(expr->left);
    free_expr(expr->right);

    for (int i = 0;
         i < expr->argument_count;
         i++)
    {
        free_expr(
            expr->arguments[i]
        );
    }

    free(expr->arguments);
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

        for (int i = 0;
             i < statement->parameter_count;
             i++)
        {
            free(statement->parameters[i]);
        }

        free(statement->parameters);

        free_expr(statement->expression);
        free_expr(statement->condition);

        parser_free(statement->body);

        if (statement->next_branch)
            parser_free(
                statement->next_branch
            );

        free(statement);

        statement = next;
    }
}
