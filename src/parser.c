#define _GNU_SOURCE

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

static Token *current(void);

static int parser_error_count = 0;

int parser_had_error(void)
{
    return parser_error_count > 0;
}

static void parser_error(const char *format, ...)
{
    va_list args;

    parser_error_count++;

    fprintf(stderr, "in line number %d\n", current()->line);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}

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


static int is_reserved_word(const char *word)
{
    static const char *reserved[] =
    {
        "is",
        "num",
        "function",
        "if",
        "then",
        "else",
        "but",
        "while",
        "for",
        "repeat",
        "return",
        "output",
        "input",
        "and",
        "or",
        "not",
        "True",
        "False"
    };

    for (
        size_t i = 0;
        i < sizeof(reserved) / sizeof(reserved[0]);
        i++
    )
    {
        if (strcasecmp(word, reserved[i]) == 0)
            return 1;
    }

    return 0;
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
    expr->line = current()->line;

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

    if (left)
        expr->line = left->line;

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

static Expr *make_call(
    const char *name,
    Expr **arguments,
    int argument_count
)
{
    Expr *expr =
        new_expr(EXPR_CALL);

    expr->call_name =
        strdup(name);

    expr->call_argument_count =
        argument_count;

    for (int i = 0; i < argument_count; i++)
    {
        expr->call_arguments[i] =
            arguments[i];
    }

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


/*
 * Postfix operators.
 *
 * Factorial is deliberately postfix:
 *
 *     5!
 *     x!
 *     (2 + 3)!
 *
 * It is NOT logical NOT.
 * Logical negation remains the word "not".
 */
static Expr *parse_primary(void);

static Expr *parse_postfix(void)
{
    Expr *expr = parse_primary();

    if (!expr)
        return NULL;

    while (current()->type == TOKEN_FACTORIAL)
    {
        advance();

        expr = make_unary(
            expr,
            TOKEN_FACTORIAL
        );
    }

    return expr;
}

static Expr *parse_expression(int minimum_precedence);
static Expr *parse_postfix(void);
static Statement *parse_statement(void);


/*
 * Conditional expression:
 *
 *     if condition then value else value
 *
 * Example:
 *
 *     if n <= 1 then 1 else n * factorial(n-1)
 */
static Expr *parse_conditional_expression(void)
{
    if (!word_is("if"))
        return NULL;

    advance(); /* if */

    Expr *condition =
        parse_expression(0);

    if (!condition)
    {
        parser_error(
            "LOIS: expected condition after 'if'"
        );

        return NULL;
    }

    if (!word_is("then"))
    {
        parser_error(
            "LOIS: expected 'then' in conditional expression"
        );

        return NULL;
    }

    advance(); /* then */

    Expr *then_expr =
        parse_expression(0);

    if (!then_expr)
    {
        parser_error(
            "LOIS: expected expression after 'then'"
        );

        return NULL;
    }

    if (!word_is("else"))
    {
        parser_error(
            "LOIS: expected 'else' in conditional expression"
        );

        return NULL;
    }

    advance(); /* else */

    Expr *else_expr =
        parse_expression(0);

    if (!else_expr)
    {
        parser_error(
            "LOIS: expected expression after 'else'"
        );

        return NULL;
    }

    Expr *expr =
        new_expr(EXPR_CONDITIONAL);

    expr->condition = condition;
    expr->left = then_expr;
    expr->right = else_expr;

    return expr;
}


static int word_exact(const char *word)
{
    return
        current()->type == TOKEN_WORD &&
        strcmp(current()->text, word) == 0;
}

static Expr *parse_set_literal(void)
{
    /*
     * LOIS set literal:
     *
     *     {1,2,3,4}
     *
     * Elements are expressions separated by commas.
     */
    if (current()->type != TOKEN_LBRACE)
        return NULL;

    advance(); /* { */

    Expr *set = new_expr(EXPR_SET);

    set->left = NULL;
    set->right = NULL;

    /*
     * Store the elements as a linked expression chain:
     *
     * set->left = first element
     * element->right = next element
     */
    Expr *tail = NULL;

    if (current()->type == TOKEN_RBRACE)
    {
        advance();
        return set;
    }

    while (1)
    {
        Expr *element = parse_expression(0);

        if (!element)
        {
            fprintf(stderr, "LOIS: invalid set element\n");
            return set;
        }

        if (!set->left)
            set->left = element;
        else
            tail->right = element;

        tail = element;

        if (current()->type == TOKEN_COMMA)
        {
            advance();
            continue;
        }

        if (current()->type == TOKEN_RBRACE)
        {
            advance();
            break;
        }

        fprintf(stderr, "LOIS: expected ',' or '}' in set\n");
        break;
    }

    return set;
}

static Expr *parse_primary(void)
{
    Token *token =
        current();

    /*
     * Conditional expression:
     *
     *     if condition then value else value
     */
    if (word_is("if"))
        return parse_conditional_expression();

    /*
     * Prefix ! is logical NOT.
     *
     *     !True
     *     !False
     *     !x
     *     !(3 > 2)
     *
     * Postfix ! is factorial and is handled by parse_postfix().
     *
     * The lexer gives both forms TOKEN_FACTORIAL because
     * the parser determines whether ! is prefix or postfix
     * based on where it appears.
     */
    if (token->type == TOKEN_FACTORIAL)
    {
        advance();

        Expr *right =
            parse_postfix();

        if (!right)
            return NULL;

        return make_unary(
            right,
            TOKEN_NOT
        );
    }

    /*
     * Set literal.
     */
    if (token->type == TOKEN_LBRACE)
        return parse_set_literal();

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
            parse_postfix();

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

        /*
         * Mathematical constant.
         *
         * pi is a numeric constant, not a variable.
         */
        if (word_exact("pi"))
        {
            Expr *expr =
                new_expr(EXPR_NUMBER);

            expr->number = acos(-1.0);

            advance();

            return expr;
        }

        /*
         * Built-in/user function call.
         *
         * Example:
         *
         *     root(25)
         *     sin(0)
         *     root(sin(0))
         */
        if (peek(1)->type == TOKEN_LPAREN)
        {
            char *name =
                strdup(token->text);

            advance(); /* function name */
            advance(); /* '(' */

            Expr *arguments[16];
            int argument_count = 0;

            /*
             * Empty call:
             *
             *     greet()
             */
            if (current()->type != TOKEN_RPAREN)
            {
                while (1)
                {
                    if (argument_count >= 16)
                    {
                        fprintf(
                            stderr,
                            "LOIS: too many function arguments\\n"
                        );

                        free(name);
                        return NULL;
                    }

                    Expr *argument =
                        parse_expression(0);

                    if (!argument)
                    {
                        free(name);
                        return NULL;
                    }

                    arguments[argument_count++] =
                        argument;

                    if (current()->type != TOKEN_COMMA)
                        break;

                    advance(); /* comma */

                    if (current()->type == TOKEN_RPAREN)
                    {
                        fprintf(
                            stderr,
                            "LOIS: expected expression after ','\\n"
                        );

                        free(name);
                        return NULL;
                    }
                }
            }

            if (current()->type != TOKEN_RPAREN)
            {
                fprintf(
                    stderr,
                    "LOIS: expected ')' after function arguments\\n"
                );

                free(name);
                return NULL;
            }

            advance(); /* ')' */

            Expr *call =
                make_call(
                    name,
                    arguments,
                    argument_count
                );

            free(name);

            return call;
        }

        /*
         * Set indexing:
         *
         *     numbers1
         *     numbers2
         *     numbers3
         *
         * A trailing positive integer is interpreted as
         * a 1-based set index.
         */
        {
            const char *word = token->text;
            size_t length = strlen(word);
            size_t split = length;

            while (split > 0 &&
                   word[split - 1] >= '0' &&
                   word[split - 1] <= '9')
            {
                split--;
            }

            if (split > 0 &&
                split < length &&
                word[split] != '0')
            {
                int index = atoi(word + split);

                if (index > 0)
                {
                    Expr *target =
                        new_expr(EXPR_VARIABLE);

                    target->text =
                        malloc(split + 1);

                    memcpy(
                        target->text,
                        word,
                        split
                    );

                    target->text[split] = '\0';

                    advance();

                    Expr *expr =
                        new_expr(EXPR_INDEX);

                    expr->index_target = target;
                    expr->index = index;

                    return expr;
                }
            }
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
            parse_postfix();

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
    /*
     * Conditional expressions are complete expressions.
     *
     *     if condition then value else value
     */
    if (word_is("if"))
        return parse_conditional_expression();

    Expr *left =
        parse_postfix();

    if (!left)
        return NULL;

    while (1)
    {
        /*
         * Stop expressions at statement boundaries.
         *
         * Example:
         *
         *     if True
         *     then output is hello
         *
         * "then" is not part of the condition.
         */
        if (
            current()->type == TOKEN_NEWLINE ||
            word_is("then") ||
            word_is("else") ||
            word_is("but")
        )
        {
            break;
        }

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
    /*
     * Output supports normal expressions.
     *
     * Examples:
     *     output is age
     *     output is "Age: " + age
     *     output is x + 5
     *     output is root(25)
     *
     * A multi-word unquoted output such as:
     *     output is hello world
     *
     * remains literal text.
     */

    if (
        current()->type == TOKEN_STRING ||
        current()->type == TOKEN_NUMBER ||
        current()->type == TOKEN_LPAREN ||
        current()->type == TOKEN_MINUS
    )
    {
        /*
         * If this starts an expression, parse the expression.
         * This also handles:
         *
         *     "Age: " + age
         *     10 + 20
         *     (x + 1) * 2
         */
        return parse_expression(0);
    }

    if (current()->type == TOKEN_WORD)
    {
        /*
         * Conditional expressions begin with the reserved word "if".
         *
         *     output is if x > 0 then 1 else 2
         *
         * "if" is normally reserved from literal output, but here
         * it is the beginning of a valid expression.
         */
        if (word_is("if"))
            return parse_expression(0);

        /*
         * A word followed by an operator is an expression.
         */
        if (
            peek(1)->type == TOKEN_PLUS ||
            peek(1)->type == TOKEN_MINUS ||
            peek(1)->type == TOKEN_STAR ||
            peek(1)->type == TOKEN_SLASH ||
            peek(1)->type == TOKEN_PERCENT ||
            peek(1)->type == TOKEN_CARET ||
            peek(1)->type == TOKEN_GREATER ||
            peek(1)->type == TOKEN_LESS ||
            peek(1)->type == TOKEN_GREATER_EQUAL ||
            peek(1)->type == TOKEN_LESS_EQUAL ||
            peek(1)->type == TOKEN_EQUAL_EQUAL ||
            peek(1)->type == TOKEN_NOT_EQUAL ||
            peek(1)->type == TOKEN_AND ||
            peek(1)->type == TOKEN_OR
        )
        {
            return parse_expression(0);
        }

        /*
         * A word followed by "(" is a function call.
         *
         * Function calls in LOIS are ALWAYS parenthesized:
         *
         *     square(5)
         *     add(3, 6)
         *
         * Bare calls such as:
         *
         *     square 5
         *
         * are invalid.
         */
        if (peek(1)->type == TOKEN_LPAREN)
            return parse_expression(0);

        if (
            peek(1)->type == TOKEN_NUMBER ||
            peek(1)->type == TOKEN_STRING ||
            peek(1)->type == TOKEN_LPAREN
        )
        {
            parser_error(
                "LOIS: function calls require parentheses, e.g. square(5)"
            );

            /*
             * Consume the malformed output so the parser does
             * not reinterpret it as literal text.
             */
            advance();

            return NULL;
        }

        /*
         * A single word is a variable.
         */
        if (
            peek(1)->type == TOKEN_NEWLINE ||
            peek(1)->type == TOKEN_EOF
        )
        {
            return parse_expression(0);
        }
    }

    /*
     * Otherwise collect ordinary words as literal output.
     *
     *     output is hello world
     */
    Expr *left = NULL;

    while (
        current()->type == TOKEN_WORD ||
        current()->type == TOKEN_STRING ||
        current()->type == TOKEN_NUMBER
    )
    {
        Expr *right = NULL;

        if (
            current()->type == TOKEN_STRING ||
            current()->type == TOKEN_NUMBER
        )
        {
            right = parse_primary();
        }
        else
        {
            if (is_reserved_word(current()->text))
            {
                char message[256];

                snprintf(
                    message,
                    sizeof(message),
                    "\"%s\" is a reserved LOIS word; use quotes for literal output",
                    current()->text
                );

                parser_error(message);
                advance();
                continue;
            }

            /*
             * Bare words in output are literal text.
             *
             *     output is hello name
             *
             * "hello" and "name" are initially text. Variable
             * resolution is handled by the output evaluator.
             */
            right = new_expr(EXPR_LITERAL);

            right->text =
                strdup(current()->text);

            advance();
        }

        if (!right)
            break;

        if (!left)
        {
            left = right;
        }
        else
        {
            left =
                make_binary(
                    left,
                    right,
                    TOKEN_PLUS
                );
        }
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
    advance(); /* output */

    /*
     * output is ...
     * output = ...
     *
     * Both forms produce output. The distinction between
     * is and = is handled by the construct that needs it.
     */
    if (word_is("is"))
        advance();
    else if (current()->type == TOKEN_ASSIGN)
        advance();
    else
    {
        parser_error(
            "LOIS: expected 'is' or '=' after 'output'"
        );

        return NULL;
    }

    Statement *statement =
        new_statement(STMT_OUTPUT);

    statement->expression =
        parse_output();

    return statement;
}

static Statement *parse_input_statement(void)
{
    advance(); /* input */

    Statement *statement =
        new_statement(STMT_INPUT);

    /*
     * input is age
     * input = age
     *
     * is  -> string input
     * =   -> numeric input
     */
    if (word_is("is"))
    {
        advance();

        if (current()->type != TOKEN_WORD)
        {
            parser_error("LOIS: expected variable name after 'input is'");
            return statement;
        }

        statement->name =
            strdup(current()->text);

        statement->extra =
            strdup("string");

        advance();
    }
    else if (current()->type == TOKEN_ASSIGN)
    {
        advance();

        if (current()->type != TOKEN_WORD)
        {
            parser_error("LOIS: expected variable name after 'input ='");
            return statement;
        }

        statement->name =
            strdup(current()->text);

        statement->extra =
            strdup("num");

        advance();
    }
    else
    {
        parser_error(
            "LOIS: expected 'is' or '=' after 'input'"
        );
        return statement;
    }

    /*
     * Optional input prompt:
     *
     * input is name for "Enter your name"
     *
     * The expression after 'for' is stored as the
     * input message.
     */
    if (word_is("for"))
    {
        advance();

        if (current()->type != TOKEN_STRING)
        {
            parser_error(
                "LOIS: expected quoted input message after 'for'"
            );
            return statement;
        }

        statement->expression =
            parse_primary();
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
     * "=" means numeric assignment.
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
     *
     * Declares a numeric variable initialized to 0.
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
     * Parse the complete expression here.
     *
     * This is important because:
     *
     *     y is x
     *     z is x + 5
     *     adult is age >= 18
     *     ok is True and False
     *
     * must all become real expression trees.
     *
     * An unresolved bare word is still handled by the
     * interpreter as a string literal, preserving the
     * old behavior for things like:
     *
     *     name is hello
     */
    /*
     * A single bare word after "is" is a literal string.
     *
     *     name is mike
     *
     * stores "mike", rather than trying to resolve mike
     * as a variable.
     *
     * If the word is followed by an operator or function call,
     * parse it as a real expression.
     */
    if (
        current()->type == TOKEN_WORD &&
        peek(1)->type != TOKEN_PLUS &&
        peek(1)->type != TOKEN_MINUS &&
        peek(1)->type != TOKEN_STAR &&
        peek(1)->type != TOKEN_SLASH &&
        peek(1)->type != TOKEN_PERCENT &&
        peek(1)->type != TOKEN_CARET &&
        peek(1)->type != TOKEN_GREATER &&
        peek(1)->type != TOKEN_LESS &&
        peek(1)->type != TOKEN_GREATER_EQUAL &&
        peek(1)->type != TOKEN_LESS_EQUAL &&
        peek(1)->type != TOKEN_EQUAL_EQUAL &&
        peek(1)->type != TOKEN_NOT_EQUAL &&
        peek(1)->type != TOKEN_AND &&
        peek(1)->type != TOKEN_OR &&
        peek(1)->type != TOKEN_LPAREN
    )
    {
        statement->expression =
            new_expr(EXPR_LITERAL);

        statement->expression->text =
            strdup(current()->text);

        advance();

        return statement;
    }

    statement->expression =
        parse_expression(0);

    return statement;
}



/*
 * Collect implicit function parameters from a function expression.
 *
 * LOIS function syntax:
 *
 *     add is function of x + y
 *     square is function of x * x
 *
 * Variables appearing in the expression become parameters.
 *
 * Function names used in calls are NOT parameters:
 *
 *     add is function of square(x) + y
 *
 * produces parameters:
 *
 *     x, y
 */
static int function_has_parameter(
    Statement *statement,
    const char *name
)
{
    for (int i = 0;
         i < statement->parameter_count;
         i++)
    {
        if (strcmp(statement->parameters[i], name) == 0)
            return 1;
    }

    return 0;
}

static void collect_function_parameters(
    Statement *statement,
    Expr *expr
)
{
    if (!expr)
        return;

    switch (expr->type)
    {
        case EXPR_VARIABLE:
            if (
                expr->text &&
                !function_has_parameter(
                    statement,
                    expr->text
                ) &&
                statement->parameter_count < 16
            )
            {
                statement->parameters[
                    statement->parameter_count++
                ] = strdup(expr->text);
            }
            break;

        case EXPR_INDEX:
            /*
             * numbers1 means the variable "numbers"
             * is required by the function.
             */
            collect_function_parameters(
                statement,
                expr->index_target
            );
            break;

        case EXPR_CALL:
            /*
             * The call name itself is a function name,
             * not a parameter.
             *
             * Its arguments may contain parameters.
             */
            for (int i = 0;
                 i < expr->call_argument_count;
                 i++)
            {
                collect_function_parameters(
                    statement,
                    expr->call_arguments[i]
                );
            }
            break;

        case EXPR_UNARY:
            collect_function_parameters(
                statement,
                expr->left
            );
            break;

        case EXPR_BINARY:
            collect_function_parameters(
                statement,
                expr->left
            );

            collect_function_parameters(
                statement,
                expr->right
            );
            break;

        case EXPR_CONDITIONAL:
            collect_function_parameters(
                statement,
                expr->condition
            );

            collect_function_parameters(
                statement,
                expr->left
            );

            collect_function_parameters(
                statement,
                expr->right
            );
            break;

        case EXPR_SET:
            /*
             * Set elements are chained through ->right.
             */
            collect_function_parameters(
                statement,
                expr->left
            );
            break;

        default:
            /*
             * Literals, numbers, booleans, etc.
             * contain no parameters.
             */
            break;
    }
}

static Statement *parse_function_statement(void)
{
    /*
     * LOIS function definition:
     *
     *     add is function of x + y
     *     square is function of x * x
     *     greet is function of "hello"
     *
     * The word "of" separates the function declaration
     * from its body expression.
     *
     * Function calls are always parenthesized:
     *
     *     add(3, 6)
     *     square(5)
     *     greet()
     */

    if (current()->type != TOKEN_WORD)
        return NULL;

    /*
     * Expected beginning:
     *
     *     name is function
     */
    if (
        !peek_word_is(1, "is") ||
        !peek_word_is(2, "function")
    )
    {
        return NULL;
    }

    char *name =
        strdup(current()->text);

    advance(); /* name */
    advance(); /* is */
    advance(); /* function */

    /*
     * "of" separates the declaration from the
     * function expression.
     */
    if (word_is("of"))
        advance();

    Statement *statement =
        new_statement(STMT_FUNCTION);

    statement->name = name;

    /*
     * Parameters are represented by words appearing
     * before the function expression.
     *
     * Example:
     *
     *     add is function x y of x + y
     *
     * However, the preferred LOIS form is:
     *
     *     add is function of x + y
     *
     * In that form variables used by the expression
     * are resolved as function arguments at runtime.
     *
     * For now, do not greedily consume words here because
     * doing so would destroy the expression.
     */

    if (
        current()->type != TOKEN_NEWLINE &&
        current()->type != TOKEN_EOF
    )
    {
        statement->expression =
            parse_expression(0);

        /*
         * Parameters are implicit in LOIS.
         *
         *     add is function of x + y
         *
         * automatically means:
         *
         *     add(x, y)
         */
        collect_function_parameters(
            statement,
            statement->expression
        );
    }

    return statement;
}

static Statement *parse_return_statement(void)
{
    /*
     * return
     * return 1
     *
     * "return" by itself always returns 0.
     */

    if (!word_is("return"))
        return NULL;

    advance();

    Statement *statement =
        new_statement(STMT_RETURN);

    if (
        current()->type != TOKEN_NEWLINE &&
        current()->type != TOKEN_EOF
    )
    {
        statement->expression =
            parse_expression(0);
    }

    return statement;
}

static Statement *parse_function_call_statement(void)
{
    /*
     * Function call statement:
     *
     *     function is greet()
     *     function is square(5)
     *     function is add(3, 6)
     *
     * The parenthesized form is mandatory.
     */

    if (
        !word_is("function") ||
        !(
            peek_word_is(1, "is") ||
            peek(1)->type == TOKEN_ASSIGN
        )
    )
        return NULL;

    advance(); /* function */

    if (word_is("is"))
        advance();
    else
        advance(); /* = */

    if (current()->type != TOKEN_WORD)
    {
        parser_error(
            "LOIS: expected function name after 'function is'"
        );

        return NULL;
    }

    Statement *statement =
        new_statement(STMT_FUNCTION_CALL);

    statement->name =
        strdup(current()->text);

    advance();

    if (current()->type != TOKEN_LPAREN)
    {
        parser_error(
            "LOIS: function calls require parentheses, e.g. square(5)"
        );

        parser_free(statement);
        return NULL;
    }

    advance(); /* ( */

    if (current()->type == TOKEN_RPAREN)
    {
        advance();
        return statement;
    }

    while (current()->type != TOKEN_EOF)
    {
        if (statement->argument_count >= 16)
        {
            parser_error(
                "LOIS: too many function arguments"
            );
            parser_free(statement);
            return NULL;
        }

        Expr *argument =
            parse_expression(0);

        if (!argument)
        {
            parser_error(
                "LOIS: expected function argument"
            );

            parser_free(statement);
            return NULL;
        }

        statement->arguments[
            statement->argument_count++
        ] = argument;

        if (current()->type == TOKEN_RPAREN)
        {
            advance();
            return statement;
        }

        /*
         * Arguments must be separated by commas.
         */
        if (current()->type != TOKEN_COMMA)
        {
            parser_error(
                "LOIS: expected ',' or ')' in function call"
            );

            parser_free(statement);
            return NULL;
        }

        advance(); /* comma */
    }

    parser_error(
        "LOIS: expected ')' after function arguments"
    );

    parser_free(statement);
    return NULL;
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
     * Parse condition.
     */
    statement->condition =
        parse_expression(0);

    skip_newlines();

    if (!word_is("then"))
    {
        parser_error(
            "LOIS: expected 'then' after if condition"
        );

        parser_free(statement);
        return NULL;
    }

    advance();

    /*
     * Parse the single statement after then.
     */
    statement->body =
        parse_single_body();

    /*
     * Consume the remainder of the body line.
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
     * Final else.
     */
    if (word_is("else"))
    {
        advance();

        statement->else_body =
            parse_single_body();

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
     * Else-if:
     *
     *     but if condition
     *     then statement
     *
     * Build the next IF node directly and attach it
     * as this statement's else branch.
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

        skip_newlines();

        if (!word_is("then"))
        {
            parser_error(
                "LOIS: expected 'then' after but if condition"
            );

            parser_free(else_if);
            parser_free(statement);
            return NULL;
        }

        advance();

        /*
         * Parse else-if body.
         */
        else_if->body =
            parse_single_body();

        while (
            current()->type != TOKEN_NEWLINE &&
            current()->type != TOKEN_EOF
        )
        {
            advance();
        }

        skip_newlines();

        /*
         * Attach another branch or the final else.
         */
        if (word_is("but") && peek_word_is(1, "if"))
        {
            /*
             * Recursively parse the remaining chain.
             *
             * We are currently positioned at "but", so
             * consume it and the following "if", then
             * construct the nested IF directly.
             */
            advance(); /* but */
            advance(); /* if */

            Statement *next_if =
                new_statement(STMT_IF);

            next_if->condition =
                parse_expression(0);

            skip_newlines();

            if (!word_is("then"))
            {
                parser_error(
                    "LOIS: expected 'then' after but if condition"
                );

                parser_free(next_if);
                parser_free(statement);
                return NULL;
            }

            advance();

            next_if->body =
                parse_single_body();

            while (
                current()->type != TOKEN_NEWLINE &&
                current()->type != TOKEN_EOF
            )
            {
                advance();
            }

            skip_newlines();

            if (word_is("else"))
            {
                advance();

                next_if->else_body =
                    parse_single_body();

                while (
                    current()->type != TOKEN_NEWLINE &&
                    current()->type != TOKEN_EOF
                )
                {
                    advance();
                }
            }

            else_if->else_body =
                next_if;
        }
        else if (word_is("else"))
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


/*
 * Parse:
 *
 *     while condition
 *     then statement
 *
 * The condition uses the normal expression parser,
 * so all existing Boolean/comparison operators work:
 *
 *     while x < 10
 *     then ...
 *
 *     while x >= 1 and x <= 10
 *     then ...
 *
 *     while not False
 *     then ...
 */
static Statement *parse_while_statement(void)
{
    if (!word_is("while"))
        return NULL;

    advance();

    Statement *statement =
        new_statement(STMT_WHILE);

    /*
     * Parse the loop condition.
     */
    statement->condition =
        parse_expression(0);

    if (!statement->condition)
    {
        fprintf(
            stderr,
            "LOIS: expected condition after 'while'\n"
        );

        parser_free(statement);

        return NULL;
    }

    /*
     * Move to the "then" line.
     */
    skip_newlines();

    if (!word_is("then"))
    {
        fprintf(
            stderr,
            "LOIS: expected 'then' after while condition\n"
        );

        parser_free(statement);

        return NULL;
    }

    advance();

    /*
     * Parse the first statement of the loop body.
     */
    Statement *body =
        parse_single_body();

    if (!body)
    {
        fprintf(
            stderr,
            "LOIS: expected statement after 'then' in while loop\n"
        );

        parser_free(statement);

        return NULL;
    }

    statement->body = body;

    /*
     * The first body statement ends at the newline.
     */
    while (
        current()->type != TOKEN_NEWLINE &&
        current()->type != TOKEN_EOF
    )
    {
        advance();
    }

    /*
     * Additional statements immediately following the
     * while body belong to the loop.
     *
     * Example:
     *
     *     while count <= 3
     *     then output is count
     *     count = count + 1
     *
     * becomes:
     *
     *     WHILE
     *       body -> OUTPUT -> ASSIGN
     *
     * This is important because execute_statement()
     * already walks statement->next.
     */
    Statement *body_tail = body;

    while (body_tail->next)
        body_tail = body_tail->next;

    while (1)
    {
        /*
         * Move to the next logical line.
         */
        skip_newlines();

        /*
         * EOF means the loop body is finished.
         */
        if (current()->type == TOKEN_EOF)
            break;

        /*
         * Stop before another top-level construct.
         *
         * Function definitions, conditionals, loops,
         * repeat statements, etc. should remain top-level.
         */
        if (
            word_is("if") ||
            word_is("while") ||
            word_is("repeat") ||
            word_is("function") ||
            word_is("return")
        )
        {
            break;
        }

        /*
         * Parse another normal statement.
         *
         * This allows:
         *
         *     count = count + 1
         *
         * after the "then" statement.
         */
        Statement *next_body =
            parse_statement();

        if (!next_body)
            break;

        /*
         * Attach it to the loop body chain.
         */
        body_tail->next = next_body;

        body_tail = next_body;

        /*
         * Consume the rest of that logical line.
         */
        while (
            current()->type != TOKEN_NEWLINE &&
            current()->type != TOKEN_EOF
        )
        {
            advance();
        }
    }

    return statement;
}


/*
 * Parse:
 *
 *     repeat 5
 *     then output is hello
 *
 * The repeat count is a normal expression, so this also
 * allows:
 *
 *     repeat x
 *     then output is hello
 *
 * and:
 *
 *     repeat 2 + 3
 *     then output is hello
 */

/*
 * Parse:
 *
 *     for x<=10
 *     then output is x
 *
 *     for x<=10
 *     then for y<=10
 *     then output is x*y
 *
 * A for loop always starts its variable at 1 and
 * increments it by 1 after each iteration.
 *
 * "then" attaches exactly one statement as the body.
 * This means nesting does not require indentation,
 * braces, or an "end" keyword.
 */
static Statement *parse_for_statement(void)
{
    if (!word_is("for"))
        return NULL;

    advance();

    /*
     * The loop variable must be a word.
     *
     *     for x<=10
     *         ^
     */
    if (current()->type != TOKEN_WORD)
    {
        fprintf(
            stderr,
            "LOIS: expected loop variable after 'for'\n"
        );

        return NULL;
    }

    char *loop_variable =
        strdup(current()->text);

    advance();

    /*
     * Currently the simple for syntax is:
     *
     *     for x<=10
     *
     * We deliberately require <=.
     */
    if (current()->type != TOKEN_LESS_EQUAL)
    {
        fprintf(
            stderr,
            "LOIS: expected '<=' after for loop variable\n"
        );

        free(loop_variable);
        return NULL;
    }

    advance();

    /*
     * Parse only the limit expression.
     */
    Expr *limit =
        parse_expression(0);

    if (!limit)
    {
        fprintf(
            stderr,
            "LOIS: expected limit after 'for %s<='\n",
            loop_variable
        );

        free(loop_variable);
        return NULL;
    }

    /*
     * Build:
     *
     *     x <= limit
     *
     * as the normal condition expression.
     */
    Expr *variable =
        new_expr(EXPR_VARIABLE);

    variable->text =
        strdup(loop_variable);

    Expr *condition =
        make_binary(
            variable,
            limit,
            TOKEN_LESS_EQUAL
        );

    Statement *statement =
        new_statement(STMT_FOR);

    statement->loop_variable =
        loop_variable;

    statement->condition =
        condition;

    /*
     * "then" may be on the next line.
     */
    skip_newlines();

    if (!word_is("then"))
    {
        fprintf(
            stderr,
            "LOIS: expected 'then' after for condition\n"
        );

        parser_free(statement);
        return NULL;
    }

    advance();

    /*
     * Exactly one statement is the body.
     *
     * This is what gives us clean nesting:
     *
     *     for x<=10
     *     then for y<=10
     *     then output is x*y
     *
     * Outer FOR
     *   body -> Inner FOR
     *              body -> OUTPUT
     */
    statement->body =
        parse_statement();

    if (!statement->body)
    {
        fprintf(
            stderr,
            "LOIS: expected statement after 'then' in for loop\n"
        );

        parser_free(statement);
        return NULL;
    }

    return statement;
}


static Statement *parse_repeat_statement(void)
{
    if (!word_is("repeat"))
        return NULL;

    advance();

    Statement *statement =
        new_statement(STMT_REPEAT);

    /*
     * Parse the repetition count.
     */
    statement->count =
        parse_expression(0);

    if (!statement->count)
    {
        fprintf(
            stderr,
            "LOIS: expected count after 'repeat'\n"
        );

        parser_free(statement);
        return NULL;
    }

    /*
     * Move to the "then" line.
     */
    skip_newlines();

    if (!word_is("then"))
    {
        fprintf(
            stderr,
            "LOIS: expected 'then' after repeat count\n"
        );

        parser_free(statement);
        return NULL;
    }

    advance();

    /*
     * Parse the single body statement.
     */
    statement->body =
        parse_single_body();

    /*
     * Consume the rest of the body line.
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

    if (word_is("repeat"))
        return parse_repeat_statement();

    if (word_is("return"))
        return parse_return_statement();

    if (
        word_is("function") &&
        (
            peek_word_is(1, "is") ||
            peek(1)->type == TOKEN_ASSIGN
        )
    )
        return parse_function_call_statement();

    if (
        current()->type == TOKEN_WORD &&
        (
            peek_word_is(1, "function") ||
            (
                peek_word_is(1, "is") &&
                peek_word_is(2, "function")
            ) ||
            (
                peek(1)->type == TOKEN_ASSIGN &&
                peek_word_is(2, "function")
            )
        )
    )
    {
        Statement *function =
            parse_function_statement();

        if (function)
            return function;
    }

    if (current()->type == TOKEN_WORD)
        return parse_assignment_statement();

    advance();

    return NULL;
}

static void recover_to_next_line(void)
{
    while (
        current()->type != TOKEN_NEWLINE &&
        current()->type != TOKEN_EOF
    )
    {
        advance();
    }

    if (current()->type == TOKEN_NEWLINE)
        advance();
}

Statement *parser_parse(TokenList *token_list)
{
    parser_error_count = 0;

    tokens = token_list;
    position = 0;

    Statement *head = NULL;
    Statement *tail = NULL;

    while (current()->type != TOKEN_EOF)
    {
        int start_position = position;

        Statement *statement =
            parse_statement();

        if (!statement)
        {
            if (current()->type != TOKEN_EOF)
            {
                if (position == start_position)
                    recover_to_next_line();
            }

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
    }

    return head;
}

static void free_expr(Expr *expr)
{
    if (!expr)
        return;

    free_expr(expr->left);
    free_expr(expr->right);

    if (expr->type == EXPR_CALL)
    {
        free(expr->call_name);
        for (int i = 0; i < expr->call_argument_count; i++)
        {
            free_expr(expr->call_arguments[i]);
        }
    }

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
        free(statement->loop_variable);

        for (int i = 0; i < statement->parameter_count; i++)
            free(statement->parameters[i]);

        for (int i = 0; i < statement->argument_count; i++)
            free_expr(statement->arguments[i]);

        free_expr(statement->expression);
        free_expr(statement->condition);
        free_expr(statement->count);

        parser_free(statement->body);
        parser_free(statement->else_body);

        free(statement);

        statement = next;
    }
}
