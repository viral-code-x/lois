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

static int at(TokenType type)
{
    return current()->type == type;
}

static int word_is(
    const char *word
)
{
    return
        current()->type ==
            TOKEN_WORD &&
        strcasecmp(
            current()->text,
            word
        ) == 0;
}

static void advance(void)
{
    if (!at(TOKEN_EOF))
        position++;
}

static void skip_newlines(void)
{
    while (at(TOKEN_NEWLINE))
        advance();
}

static char *copy_string(
    const char *text
)
{
    char *result =
        strdup(text);

    if (!result)
    {
        fprintf(stderr,
                "LOIS: out of memory\n");
        exit(1);
    }

    return result;
}

static Expr *new_expr(
    ExprType type
)
{
    Expr *expr =
        calloc(1, sizeof(Expr));

    if (!expr)
    {
        fprintf(stderr,
                "LOIS: out of memory\n");
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
    Expr *value,
    TokenType operator
)
{
    Expr *expr =
        new_expr(EXPR_UNARY);

    expr->left = value;

    expr->operator = operator;

    return expr;
}

/*
 * primary
 *
 * number
 * string
 * variable
 * true
 * false
 * function(...)
 * (...)
 */
static Expr *parse_primary(void);

static Expr *parse_expression(void);

static Expr *parse_primary(void)
{
    /*
     * Number
     */
    if (at(TOKEN_NUMBER))
    {
        Expr *expr =
            new_expr(EXPR_NUMBER);

        expr->number =
            current()->number;

        advance();

        return expr;
    }

    /*
     * String
     */
    if (at(TOKEN_STRING))
    {
        Expr *expr =
            new_expr(EXPR_LITERAL);

        expr->text =
            copy_string(
                current()->text
            );

        advance();

        return expr;
    }

    /*
     * Parentheses
     */
    if (at(TOKEN_LPAREN))
    {
        advance();

        Expr *expr =
            parse_expression();

        if (at(TOKEN_RPAREN))
            advance();

        return expr;
    }

    /*
     * Word
     */
    if (at(TOKEN_WORD))
    {
        char *name =
            copy_string(
                current()->text
            );

        advance();

        /*
         * Boolean literals.
         */
        if (
            strcasecmp(
                name,
                "true"
            ) == 0
        )
        {
            Expr *expr =
                new_expr(
                    EXPR_BOOLEAN
                );

            expr->boolean = 1;

            free(name);

            return expr;
        }

        if (
            strcasecmp(
                name,
                "false"
            ) == 0
        )
        {
            Expr *expr =
                new_expr(
                    EXPR_BOOLEAN
                );

            expr->boolean = 0;

            free(name);

            return expr;
        }

        /*
         * Function call.
         */
        if (at(TOKEN_LPAREN))
        {
            advance();

            Expr *expr =
                new_expr(EXPR_CALL);

            expr->text = name;

            if (!at(TOKEN_RPAREN))
            {
                while (1)
                {
                    expr->args =
                        realloc(
                            expr->args,
                            sizeof(Expr *) *
                            (
                                expr->arg_count + 1
                            )
                        );

                    expr->args[
                        expr->arg_count++
                    ] =
                        parse_expression();

                    if (at(TOKEN_COMMA))
                    {
                        advance();
                        continue;
                    }

                    break;
                }
            }

            if (at(TOKEN_RPAREN))
                advance();

            return expr;
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
    if (
        at(TOKEN_MINUS) ||
        at(TOKEN_BANG)
    )
    {
        TokenType operator =
            current()->type;

        advance();

        return make_unary(
            parse_unary(),
            operator
        );
    }

    if (word_is("not"))
    {
        advance();

        return make_unary(
            parse_unary(),
            TOKEN_BANG
        );
    }

    return parse_primary();
}

/*
 * Power is right associative:
 *
 * 2^3^2
 *
 * = 2^(3^2)
 */
static Expr *parse_power(void)
{
    Expr *left =
        parse_unary();

    if (at(TOKEN_CARET))
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

    while (
        at(TOKEN_STAR) ||
        at(TOKEN_SLASH) ||
        at(TOKEN_PERCENT)
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

    while (
        at(TOKEN_PLUS) ||
        at(TOKEN_MINUS)
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

    while (
        at(TOKEN_GREATER) ||
        at(TOKEN_LESS) ||
        at(TOKEN_GREATER_EQUAL) ||
        at(TOKEN_LESS_EQUAL)
    )
    {
        TokenType operator =
            current()->type;

        advance();

        Expr *right =
            parse_term();

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

    while (
        at(TOKEN_EQUAL_EQUAL) ||
        at(TOKEN_NOT_EQUAL)
    )
    {
        TokenType operator =
            current()->type;

        advance();

        Expr *right =
            parse_comparison();

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

    while (
        at(TOKEN_AND_AND) ||
        word_is("and")
    )
    {
        advance();

        Expr *right =
            parse_equality();

        left =
            make_binary(
                left,
                right,
                TOKEN_AND_AND
            );
    }

    return left;
}

static Expr *parse_or(void)
{
    Expr *left =
        parse_and();

    while (
        at(TOKEN_OR_OR) ||
        word_is("or")
    )
    {
        advance();

        Expr *right =
            parse_and();

        left =
            make_binary(
                left,
                right,
                TOKEN_OR_OR
            );
    }

    return left;
}

static Expr *parse_expression(void)
{
    return parse_or();
}

static int expression_end(void)
{
    return
        at(TOKEN_NEWLINE) ||
        at(TOKEN_DOT) ||
        at(TOKEN_EOF);
}

static int control_word(void)
{
    return
        word_is("then") ||
        word_is("else") ||
        word_is("do") ||
        word_is("till") ||
        word_is("upto");
}

/*
 * Output is special because:
 *
 * output is hello name
 *
 * means:
 *
 * hello + name
 *
 * while:
 *
 * output is "hello     name"
 *
 * preserves the spaces.
 */
static Expr *parse_output(void)
{
    Expr *left =
        parse_expression();

    if (!left)
        return NULL;

    while (
        !expression_end() &&
        !control_word()
    )
    {
        /*
         * space*6
         */
        if (
            word_is("space") &&
            tokens->tokens[position + 1].type
                == TOKEN_STAR &&
            tokens->tokens[position + 2].type
                == TOKEN_NUMBER
        )
        {
            advance();
            advance();

            Expr *amount =
                parse_primary();

            Expr *space =
                new_expr(EXPR_CALL);

            space->text =
                copy_string("space");

            space->args =
                malloc(sizeof(Expr *));

            space->args[0] =
                amount;

            space->arg_count = 1;

            left =
                make_binary(
                    left,
                    space,
                    TOKEN_PLUS
                );

            continue;
        }

        if (
            at(TOKEN_WORD) ||
            at(TOKEN_STRING) ||
            at(TOKEN_NUMBER) ||
            at(TOKEN_LPAREN)
        )
        {
            Expr *right =
                parse_expression();

            if (!right)
                break;

            left =
                make_binary(
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

static void consume_end(void)
{
    if (at(TOKEN_DOT))
        advance();

    if (at(TOKEN_NEWLINE))
        advance();
}

static Statement *new_statement(
    StatementType type
)
{
    Statement *statement =
        calloc(
            1,
            sizeof(Statement)
        );

    if (!statement)
    {
        fprintf(stderr,
                "LOIS: out of memory\n");
        exit(1);
    }

    statement->type = type;

    return statement;
}

static Statement *parse_statement(void);

static Statement *parse_output_statement(void)
{
    advance();

    if (word_is("is"))
        advance();

    Statement *statement =
        new_statement(
            STMT_OUTPUT
        );

    statement->expression =
        parse_output();

    /*
     * output is 17/3 upto 3
     */
    if (word_is("upto"))
    {
        advance();

        if (at(TOKEN_NUMBER))
        {
            statement->precision =
                (int)current()->number;

            advance();
        }
    }

    consume_end();

    return statement;
}

static Statement *parse_input_statement(void)
{
    advance();

    if (word_is("is"))
        advance();

    Statement *statement =
        new_statement(
            STMT_INPUT
        );

    if (at(TOKEN_WORD))
    {
        statement->name =
            copy_string(
                current()->text
            );

        advance();
    }

    consume_end();

    return statement;
}

/*
 * Numeric assignment:
 *
 * age = 13
 *
 * Natural assignment:
 *
 * age is 13
 *
 * The latter remains string unless age
 * was declared num.
 */
static Statement *parse_assignment_statement(void)
{
    if (!at(TOKEN_WORD))
        return NULL;

    char *name =
        copy_string(
            current()->text
        );

    advance();

    int numeric_assignment = 0;

    if (at(TOKEN_ASSIGN))
    {
        numeric_assignment = 1;
        advance();
    }
    else if (word_is("is"))
    {
        advance();
    }
    else
    {
        free(name);
        return NULL;
    }

    Statement *statement =
        new_statement(
            STMT_ASSIGN
        );

    statement->name = name;

    statement->numeric_assignment =
        numeric_assignment;

    /*
     * Explicit type declarations.
     */
    if (word_is("num"))
    {
        statement->extra =
            copy_string("num");

        advance();

        consume_end();

        return statement;
    }

    if (word_is("string"))
    {
        statement->extra =
            copy_string("string");

        advance();

        consume_end();

        return statement;
    }

    if (
        word_is("boolean") ||
        word_is("bool")
    )
    {
        statement->extra =
            copy_string("boolean");

        advance();

        consume_end();

        return statement;
    }

    /*
     * "=" always parses a numeric expression.
     */
    if (numeric_assignment)
    {
        statement->expression =
            parse_expression();

        consume_end();

        return statement;
    }

    /*
     * "is" with a string literal.
     */
    if (at(TOKEN_STRING))
    {
        statement->expression =
            parse_primary();

        consume_end();

        return statement;
    }

    /*
     * "is 13"
     *
     * The interpreter will decide whether
     * this is a string or number based on
     * the previous type declaration.
     */
    if (at(TOKEN_NUMBER))
    {
        statement->expression =
            parse_primary();

        consume_end();

        return statement;
    }

    /*
     * "is Alex"
     *
     * A bare word is treated as literal text.
     */
    if (at(TOKEN_WORD))
    {
        statement->expression =
            new_expr(EXPR_LITERAL);

        statement->expression->text =
            copy_string(
                current()->text
            );

        advance();

        consume_end();

        return statement;
    }

    statement->expression =
        parse_expression();

    consume_end();

    return statement;
}

/*
 * if age > 18 then ...
 *
 * else if ...
 *
 * else ...
 */
static Statement *parse_if_statement(void)
{
    advance();

    Statement *statement =
        new_statement(STMT_IF);

    statement->condition =
        parse_expression();

    if (word_is("then"))
        advance();

    if (at(TOKEN_NEWLINE))
    {
        advance();

        skip_newlines();

        statement->body =
            parse_statement();
    }
    else
    {
        statement->body =
            parse_statement();
    }

    /*
     * else
     */
    if (word_is("else"))
    {
        advance();

        /*
         * else if
         */
        if (word_is("if"))
        {
            statement->else_body =
                parse_if_statement();
        }
        else if (at(TOKEN_NEWLINE))
        {
            advance();

            skip_newlines();

            statement->else_body =
                parse_statement();
        }
        else
        {
            statement->else_body =
                parse_statement();
        }
    }

    return statement;
}

/*
 * while condition output ...
 */
static Statement *parse_while_statement(void)
{
    advance();

    Statement *statement =
        new_statement(
            STMT_WHILE
        );

    statement->condition =
        parse_expression();

    if (word_is("do"))
        advance();

    if (at(TOKEN_NEWLINE))
    {
        advance();

        skip_newlines();

        statement->body =
            parse_statement();
    }
    else
    {
        statement->body =
            parse_statement();
    }

    return statement;
}

/*
 * for x = 1 till x < 10 do 1
 */
static Statement *parse_for_statement(void)
{
    advance();

    Statement *statement =
        new_statement(STMT_FOR);

    if (at(TOKEN_WORD))
    {
        statement->name =
            copy_string(
                current()->text
            );

        advance();
    }

    if (
        at(TOKEN_ASSIGN) ||
        word_is("is")
    )
    {
        advance();
    }

    statement->expression =
        parse_expression();

    if (word_is("till"))
        advance();

    statement->condition =
        parse_expression();

    if (word_is("do"))
        advance();

    statement->step =
        parse_expression();

    if (at(TOKEN_NEWLINE))
    {
        advance();

        skip_newlines();

        statement->body =
            parse_statement();
    }
    else
    {
        statement->body =
            parse_statement();
    }

    return statement;
}

/*
 * p is function of x*x
 */
static Statement *parse_function_statement(void)
{
    char *name =
        copy_string(
            current()->text
        );

    advance();

    if (!word_is("is"))
    {
        free(name);
        return NULL;
    }

    advance();

    if (!word_is("function"))
    {
        free(name);
        return NULL;
    }

    advance();

    if (word_is("of"))
        advance();

    Statement *statement =
        new_statement(
            STMT_FUNCTION
        );

    statement->name = name;

    /*
     * We currently support the simple
     * and useful form:
     *
     * p is function of x*x
     *
     * x is the parameter.
     */
    if (at(TOKEN_WORD))
    {
        statement->params =
            malloc(sizeof(char *));

        statement->params[0] =
            copy_string(
                current()->text
            );

        statement->param_count = 1;
    }

    /*
     * Parse the entire expression from
     * the parameter itself:
     *
     * x*x
     *
     * instead of accidentally treating the
     * first x as only the parameter.
     */
    int body_position = position;

    position = body_position;

    statement->expression =
        parse_expression();

    consume_end();

    return statement;
}

static Statement *parse_statement(void)
{
    skip_newlines();

    if (at(TOKEN_EOF))
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

    if (at(TOKEN_WORD))
    {
        /*
         * Detect function:
         *
         * p is function ...
         */
        int save = position;

        advance();

        if (word_is("is"))
        {
            advance();

            if (word_is("function"))
            {
                position = save;

                return parse_function_statement();
            }
        }

        position = save;

        return parse_assignment_statement();
    }

    fprintf(
        stderr,
        "LOIS parser: unexpected token at %d:%d\n",
        current()->line,
        current()->column
    );

    advance();

    return NULL;
}

Statement *parser_parse(
    TokenList *token_list
)
{
    tokens = token_list;

    position = 0;

    Statement *head = NULL;
    Statement *tail = NULL;

    while (!at(TOKEN_EOF))
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

static void free_expr(
    Expr *expr
)
{
    if (!expr)
        return;

    free_expr(expr->left);

    free_expr(expr->right);

    for (
        int i = 0;
        i < expr->arg_count;
        i++
    )
    {
        free_expr(
            expr->args[i]
        );
    }

    free(expr->args);

    free(expr->text);

    free(expr);
}

void parser_free(
    Statement *statement
)
{
    while (statement)
    {
        Statement *next =
            statement->next;

        free(statement->name);

        free(statement->extra);

        for (
            int i = 0;
            i < statement->param_count;
            i++
        )
        {
            free(statement->params[i]);
        }

        free(statement->params);

        free_expr(
            statement->expression
        );

        free_expr(
            statement->condition
        );

        free_expr(
            statement->step
        );

        parser_free(
            statement->body
        );

        parser_free(
            statement->else_body
        );

        free(statement);

        statement = next;
    }
}
