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
    return
        peek(offset)->type == TOKEN_WORD &&
        strcasecmp(peek(offset)->text, word) == 0;
}

static void skip_newlines(void)
{
    while (current()->type == TOKEN_NEWLINE)
        advance();
}

static Expr *new_expr(ExprType type)
{
    Expr *expr =
        calloc(1, sizeof(Expr));

    if (!expr)
    {
        fprintf(stderr, "LOIS: out of memory\n");
        exit(1);
    }

    expr->type = type;

    return expr;
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
    Expr *right,
    TokenType operator
)
{
    Expr *expr =
        new_expr(EXPR_UNARY);

    expr->right = right;
    expr->operator = operator;

    return expr;
}

static int precedence(TokenType type)
{
    switch (type)
    {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 10;

        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
            return 20;

        case TOKEN_CARET:
            return 30;

        case TOKEN_GREATER:
        case TOKEN_LESS:
        case TOKEN_GREATER_EQUAL:
        case TOKEN_LESS_EQUAL:
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_NOT_EQUAL:
            return 5;

        case TOKEN_AND:
            return 3;

        case TOKEN_OR:
            return 2;

        default:
            return -1;
    }
}

static Expr *parse_expression(int minimum_precedence);
static Expr *parse_primary(void);

static int word_exact(const char *word)
{
    return
        current()->type == TOKEN_WORD &&
        strcmp(current()->text, word) == 0;
}

static Expr *parse_primary(void)
{
    Token *token =
        current();

    /*
     * Unary Boolean NOT.
     *
     * "not" is lexed as TOKEN_NOT, so it must be
     * handled before TOKEN_WORD.
     */
    if (token->type == TOKEN_NOT)
    {
        advance();

        Expr *right =
            parse_primary();

        if (!right)
            return NULL;

        return make_unary(
            right,
            TOKEN_NOT
        );
    }

    /*
     * String literal.
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
     * Number literal.
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
     * Word / variable.
     *
     * "not expression"
     */
    if (token->type == TOKEN_WORD)
    {
        /*
         * Boolean literals are deliberately case-sensitive.
         *
         * True / False  -> boolean
         * true / false  -> ordinary words
         */
        if (word_exact("True"))
        {
            Expr *expr =
                new_expr(EXPR_BOOLEAN);

            expr->number = 1;

            advance();

            return expr;
        }

        if (word_exact("False"))
        {
            Expr *expr =
                new_expr(EXPR_BOOLEAN);

            expr->number = 0;

            advance();

            return expr;
        }

        Expr *expr =
            new_expr(EXPR_VARIABLE);

        expr->text =
            strdup(token->text);

        advance();

        return expr;
    }

    /*
     * Unary minus.
     */
    if (token->type == TOKEN_MINUS)
    {
        advance();

        Expr *right =
            parse_primary();

        if (!right)
            return NULL;

        return make_unary(
            right,
            TOKEN_MINUS
        );
    }

    /*
     * Parenthesized expression.
     */
    if (token->type == TOKEN_LPAREN)
    {
        advance();

        Expr *expr =
            parse_expression(0);

        if (current()->type == TOKEN_RPAREN)
            advance();

        return expr;
    }

    return NULL;
}

static Expr *parse_expression(int minimum_precedence)
{
    Expr *left =
        parse_primary();

    if (!left)
        return NULL;

    while (1)
    {
        TokenType operator =
            current()->type;

        int p =
            precedence(operator);

        /*
         * Logical operators are words.
         *
         * They deliberately have lower precedence
         * than comparison operators.
         *
         * "and" binds tighter than "or".
         */
        if (word_is("and"))
        {
            operator = TOKEN_AND;
            p = 3;
        }
        else if (word_is("or"))
        {
            operator = TOKEN_OR;
            p = 2;
        }

        if (p < minimum_precedence)
            break;

        advance();

        /*
         * ^ is right associative.
         */
        int next_precedence =
            operator == TOKEN_CARET
                ? p
                : p + 1;

        Expr *right =
            parse_expression(next_precedence);

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

static Expr *parse_output(void)
{
    Expr *left =
        parse_expression(0);

    if (!left)
        return NULL;

    /*
     * Adjacent words/numbers/strings become
     * output concatenation.
     */
    while (
        current()->type == TOKEN_WORD ||
        current()->type == TOKEN_STRING ||
        current()->type == TOKEN_NUMBER
    )
    {
        /*
         * Don't consume the beginning of another
         * statement.
         */
        if (word_is("then") ||
            word_is("else") ||
            word_is("but") ||
            word_is("if"))
        {
            break;
        }

        Expr *right =
            parse_primary();

        if (!right)
            break;

        left =
            make_binary(
                left,
                right,
                TOKEN_PLUS
            );
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

static Statement *parse_output_statement(void)
{
    advance();

    if (word_is("is"))
        advance();

    Statement *statement =
        new_statement(STMT_OUTPUT);

    statement->expression =
        parse_output();

    return statement;
}

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

    return statement;
}

static Statement *parse_assignment_statement(void)
{
    if (current()->type != TOKEN_WORD)
        return NULL;

    char *name =
        strdup(current()->text);

    advance();

    Statement *statement =
        new_statement(STMT_ASSIGN);

    statement->name = name;

    /*
     * name = expression
     *
     * = means numeric assignment.
     */
    if (current()->type == TOKEN_ASSIGN)
    {
        advance();

        statement->extra =
            strdup("num");

        statement->expression =
            parse_expression(0);

        return statement;
    }

    /*
     * name is ...
     */
    if (!word_is("is"))
    {
        free(statement->name);
        free(statement);
        return NULL;
    }

    advance();

    /*
     * name is num
     */
    if (word_is("num"))
    {
        statement->extra =
            strdup("num");

        advance();

        return statement;
    }

    /*
     * name is expression
     *
     * For a bare word, preserve the old LOIS
     * behavior: it is a literal string.
     */
    if (current()->type == TOKEN_WORD)
    {
        statement->expression =
            new_expr(EXPR_LITERAL);

        statement->expression->text =
            strdup(current()->text);

        advance();

        return statement;
    }

    if (current()->type == TOKEN_STRING)
    {
        statement->expression =
            new_expr(EXPR_LITERAL);

        statement->expression->text =
            strdup(current()->text);

        advance();

        return statement;
    }

    if (current()->type == TOKEN_NUMBER)
    {
        statement->expression =
            new_expr(EXPR_LITERAL);

        char buffer[64];

        snprintf(
            buffer,
            sizeof(buffer),
            "%.15g",
            current()->number
        );

        statement->expression->text =
            strdup(buffer);

        advance();

        return statement;
    }

    statement->expression =
        parse_expression(0);

    return statement;
}

static Statement *parse_single_body(void)
{
    skip_newlines();

    if (word_is("output"))
        return parse_output_statement();

    if (word_is("input"))
        return parse_input_statement();

    if (current()->type == TOKEN_WORD)
        return parse_assignment_statement();

    return NULL;
}

static Statement *parse_if_statement(void)
{
    /*
     * LOIS conditional chain:
     *
     * if condition
     * then execution
     *
     * if condition
     * then execution
     * else execution
     *
     * if condition
     * then execution
     * but if condition
     * then execution
     * else execution
     */

    if (!word_is("if"))
        return NULL;

    advance();

    Statement *statement =
        new_statement(STMT_IF);

    /*
     * Parse the condition.
     */
    statement->condition =
        parse_expression(0);

    /*
     * "then" is normally on the next line:
     *
     *     if age >= 18
     *     then output is adult
     *
     * Move across the newline before looking for it.
     */
    skip_newlines();

    if (!word_is("then"))
    {
        fprintf(
            stderr,
            "LOIS: expected 'then' after if condition\n"
        );
        parser_free(statement);
        return NULL;
    }

    advance();

    /*
     * Parse the execution after then.
     */
    statement->body =
        parse_single_body();

    /*
     * Consume the rest of this execution line.
     */
    while (
        current()->type != TOKEN_NEWLINE &&
        current()->type != TOKEN_EOF
    )
    {
        advance();
    }

    /*
     * Move to the next logical line.
     */
    skip_newlines();

    /*
     * ELSE belongs to this completed chain.
     */
    if (word_is("else"))
    {
        advance();

        statement->else_body =
            parse_single_body();

        /*
         * Consume the rest of the else line.
         */
        while (
            current()->type != TOKEN_NEWLINE &&
            current()->type != TOKEN_EOF
        )
        {
            advance();
        }

        return statement;
    }

    /*
     * BUT IF creates another conditional branch.
     *
     * Important:
     * consume "but" and "if" here, then build the
     * nested condition manually. This prevents the
     * nested parser from accidentally stealing an
     * else belonging to the current chain.
     */
    if (
        word_is("but") &&
        peek_word_is(1, "if")
    )
    {
        /*
         * Consume "but".
         */
        advance();

        /*
         * Consume "if".
         */
        advance();

        Statement *else_if =
            new_statement(STMT_IF);

        /*
         * Parse the else-if condition.
         */
        else_if->condition =
            parse_expression(0);

        /*
         * "then" follows the else-if condition on
         * the next logical line.
         */
        skip_newlines();

        if (!word_is("then"))
        {
            fprintf(
                stderr,
                "LOIS: expected 'then' after but-if condition\n"
            );

            parser_free(else_if);
            parser_free(statement);
            return NULL;
        }

        advance();

        /*
         * Parse else-if execution.
         */
        else_if->body =
            parse_single_body();

        /*
         * Consume the execution line.
         */
        while (
            current()->type != TOKEN_NEWLINE &&
            current()->type != TOKEN_EOF
        )
        {
            advance();
        }

        /*
         * Move to the possible final else.
         */
        skip_newlines();

        /*
         * Final else belongs to the else-if branch.
         */
        if (word_is("else"))
        {
            advance();

            else_if->else_body =
                parse_single_body();

            while (
                current()->type != TOKEN_NEWLINE &&
                current()->type != TOKEN_EOF
            )
            {
                advance();
            }
        }

        statement->else_body =
            else_if;

        return statement;
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

    advance();

    return NULL;
}

Statement *parser_parse(TokenList *token_list)
{
    tokens = token_list;
    position = 0;

    Statement *head = NULL;
    Statement *tail = NULL;

    while (
        current()->type != TOKEN_EOF
    )
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
        parser_free(statement->else_body);

        free(statement);

        statement = next;
    }
}
