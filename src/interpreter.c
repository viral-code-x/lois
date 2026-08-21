#define _GNU_SOURCE

#include "interpreter.h"
#include "value.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define MAX_VARIABLES 512

typedef struct
{
    char *name;
    Value value;
    int is_num;
} Variable;

static Variable variables[MAX_VARIABLES];
static int variable_count = 0;

static int loop_break = 0;

static Variable *find_variable(const char *name)
{
    for (int i = 0;
         i < variable_count;
         i++)
    {
        if (
            strcasecmp(
                variables[i].name,
                name
            ) == 0
        )
        {
            return &variables[i];
        }
    }

    return NULL;
}

static void set_variable(
    const char *name,
    Value value,
    int is_num
)
{
    Variable *existing =
        find_variable(name);

    if (existing)
    {
        value_free(
            &existing->value
        );

        existing->value = value;
        existing->is_num = is_num;

        return;
    }

    if (variable_count >= MAX_VARIABLES)
    {
        fprintf(
            stderr,
            "LOIS: too many variables\n"
        );

        exit(1);
    }

    variables[variable_count].name =
        strdup(name);

    variables[variable_count].value =
        value;

    variables[variable_count].is_num =
        is_num;

    variable_count++;
}

static double value_number_or_zero(
    Value *value
)
{
    if (
        value &&
        value->type == VALUE_NUMBER
    )
    {
        return value->number;
    }

    return 0;
}

static int truthy(Value *value)
{
    if (!value)
        return 0;

    if (value->type == VALUE_NUMBER)
        return value->number != 0;

    if (value->type == VALUE_STRING)
        return value->string &&
               value->string[0] != '\0';

    return 0;
}

static void print_number(double number)
{
    /*
     * Avoid ugly 13.000000 output.
     */
    if (
        number ==
        (long long)number
    )
    {
        printf(
            "%lld",
            (long long)number
        );
    }
    else
    {
        printf(
            "%.12g",
            number
        );
    }
}

static void print_value(Value *value)
{
    if (!value)
        return;

    if (value->type == VALUE_STRING)
    {
        printf(
            "%s",
            value->string
                ? value->string
                : ""
        );
    }
    else if (value->type == VALUE_NUMBER)
    {
        print_number(
            value->number
        );
    }
}

static Value evaluate(Expr *expr);

static Value evaluate_call(Expr *expr)
{
    if (!expr || !expr->text)
        return value_none();

    const char *name =
        expr->text;

    /*
     * pi
     */
    if (
        strcasecmp(name, "pi") == 0 &&
        expr->argument_count == 0
    )
    {
        return value_number(
            M_PI
        );
    }

    /*
     * Evaluate first argument.
     */
    Value argument =
        value_none();

    if (expr->argument_count > 0)
    {
        argument =
            evaluate(
                expr->arguments[0]
            );
    }

    double x =
        value_number_or_zero(
            &argument
        );

    /*
     * sin
     */
    if (
        strcasecmp(name, "sin") == 0
    )
    {
        value_free(&argument);

        return value_number(
            sin(x)
        );
    }

    /*
     * cos
     */
    if (
        strcasecmp(name, "cos") == 0
    )
    {
        value_free(&argument);

        return value_number(
            cos(x)
        );
    }

    /*
     * log = base-10 logarithm.
     */
    if (
        strcasecmp(name, "log") == 0
    )
    {
        value_free(&argument);

        return value_number(
            log10(x)
        );
    }

    /*
     * ln = natural logarithm.
     */
    if (
        strcasecmp(name, "ln") == 0
    )
    {
        value_free(&argument);

        return value_number(
            log(x)
        );
    }

    /*
     * int(x) = integral/truncate.
     */
    if (
        strcasecmp(name, "int") == 0
    )
    {
        value_free(&argument);

        return value_number(
            trunc(x)
        );
    }

    value_free(&argument);

    /*
     * Unknown function for now.
     */
    fprintf(
        stderr,
        "LOIS: unknown function '%s'\n",
        name
    );

    return value_none();
}

static Value evaluate(Expr *expr)
{
    if (!expr)
        return value_none();

    /*
     * Literal.
     */
    if (
        expr->type ==
        EXPR_LITERAL
    )
    {
        return value_string(
            expr->text
        );
    }

    /*
     * Number.
     */
    if (
        expr->type ==
        EXPR_NUMBER
    )
    {
        return value_number(
            expr->number
        );
    }

    /*
     * Variable.
     */
    if (
        expr->type ==
        EXPR_VARIABLE
    )
    {
        Variable *variable =
            find_variable(
                expr->text
            );

        if (!variable)
        {
            /*
             * Unknown words are strings.
             */
            return value_string(
                expr->text
            );
        }

        return value_copy(
            &variable->value
        );
    }

    /*
     * Function call.
     */
    if (
        expr->type ==
        EXPR_CALL
    )
    {
        return evaluate_call(
            expr
        );
    }

    /*
     * Unary.
     */
    if (
        expr->type ==
        EXPR_UNARY
    )
    {
        Value right =
            evaluate(
                expr->right
            );

        if (
            expr->operator ==
            TOKEN_BANG
        )
        {
            int result =
                !truthy(&right);

            value_free(&right);

            return value_number(
                result
            );
        }

        if (
            expr->operator ==
            TOKEN_MINUS
        )
        {
            double result =
                -value_number_or_zero(
                    &right
                );

            value_free(&right);

            return value_number(
                result
            );
        }

        value_free(&right);

        return value_none();
    }

    /*
     * Binary expression.
     */
    if (
        expr->type ==
        EXPR_BINARY
    )
    {
        Value left =
            evaluate(expr->left);

        Value right =
            evaluate(expr->right);

        /*
         * String concatenation.
         */
        if (
            expr->operator ==
                TOKEN_PLUS &&
            (
                left.type ==
                    VALUE_STRING ||
                right.type ==
                    VALUE_STRING
            )
        )
        {
            char left_text[4096];
            char right_text[4096];

            if (
                left.type ==
                VALUE_STRING
            )
            {
                snprintf(
                    left_text,
                    sizeof(left_text),
                    "%s",
                    left.string
                );
            }
            else
            {
                snprintf(
                    left_text,
                    sizeof(left_text),
                    "%.12g",
                    left.number
                );
            }

            if (
                right.type ==
                VALUE_STRING
            )
            {
                snprintf(
                    right_text,
                    sizeof(right_text),
                    "%s",
                    right.string
                );
            }
            else
            {
                snprintf(
                    right_text,
                    sizeof(right_text),
                    "%.12g",
                    right.number
                );
            }

            char result[8192];

            snprintf(
                result,
                sizeof(result),
                "%s %s",
                left_text,
                right_text
            );

            value_free(&left);
            value_free(&right);

            return value_string(
                result
            );
        }

        /*
         * Numeric operators.
         */
        if (
            left.type ==
                VALUE_NUMBER &&
            right.type ==
                VALUE_NUMBER
        )
        {
            double a =
                left.number;

            double b =
                right.number;

            double result = 0;

            switch (
                expr->operator
            )
            {
                case TOKEN_PLUS:
                    result = a + b;
                    break;

                case TOKEN_MINUS:
                    result = a - b;
                    break;

                case TOKEN_STAR:
                    result = a * b;
                    break;

                case TOKEN_SLASH:
                    if (b == 0)
                    {
                        fprintf(
                            stderr,
                            "LOIS: division by zero\n"
                        );

                        value_free(&left);
                        value_free(&right);

                        return value_none();
                    }

                    result = a / b;
                    break;

                case TOKEN_PERCENT:
                    if (b == 0)
                    {
                        fprintf(
                            stderr,
                            "LOIS: modulo by zero\n"
                        );

                        value_free(&left);
                        value_free(&right);

                        return value_none();
                    }

                    result =
                        fmod(a, b);

                    break;

                case TOKEN_POWER:
                    result =
                        pow(a, b);

                    break;

                case TOKEN_GREATER:
                    result = a > b;
                    break;

                case TOKEN_LESS:
                    result = a < b;
                    break;

                case TOKEN_GREATER_EQUAL:
                    result = a >= b;
                    break;

                case TOKEN_LESS_EQUAL:
                    result = a <= b;
                    break;

                case TOKEN_EQUAL_EQUAL:
                    result = a == b;
                    break;

                case TOKEN_NOT_EQUAL:
                    result = a != b;
                    break;

                default:
                    value_free(&left);
                    value_free(&right);

                    return value_none();
            }

            value_free(&left);
            value_free(&right);

            return value_number(
                result
            );
        }

        /*
         * String equality.
         */
        if (
            left.type ==
                VALUE_STRING &&
            right.type ==
                VALUE_STRING
        )
        {
            int comparison =
                strcmp(
                    left.string,
                    right.string
                );

            double result = 0;

            if (
                expr->operator ==
                TOKEN_EQUAL_EQUAL
            )
            {
                result =
                    comparison == 0;
            }
            else if (
                expr->operator ==
                TOKEN_NOT_EQUAL
            )
            {
                result =
                    comparison != 0;
            }

            value_free(&left);
            value_free(&right);

            return value_number(
                result
            );
        }

        value_free(&left);
        value_free(&right);
    }

    return value_none();
}

static void execute_statement(
    Statement *statement
);

static void execute_output(
    Expr *expr
)
{
    if (!expr)
        return;

    Value value =
        evaluate(expr);

    print_value(&value);

    printf("\n");

    value_free(&value);
}

static void execute_statement(
    Statement *statement
)
{
    while (statement)
    {
        switch (statement->type)
        {
            case STMT_ASSIGN:
            {
                /*
                 * age is num
                 */
                if (
                    statement->extra &&
                    strcasecmp(
                        statement->extra,
                        "num"
                    ) == 0
                )
                {
                    set_variable(
                        statement->name,
                        value_number(0),
                        1
                    );

                    break;
                }

                Value value =
                    evaluate(
                        statement->expression
                    );

                int is_num =
                    value.type ==
                    VALUE_NUMBER;

                set_variable(
                    statement->name,
                    value,
                    is_num
                );

                break;
            }

            case STMT_INPUT:
            {
                char buffer[1024];

                printf(
                    "%s: ",
                    statement->name
                );

                fflush(stdout);

                if (
                    fgets(
                        buffer,
                        sizeof(buffer),
                        stdin
                    )
                )
                {
                    buffer[
                        strcspn(
                            buffer,
                            "\n"
                        )
                    ] = '\0';

                    Variable *variable =
                        find_variable(
                            statement->name
                        );

                    if (
                        variable &&
                        variable->is_num
                    )
                    {
                        char *end;

                        double number =
                            strtod(
                                buffer,
                                &end
                            );

                        set_variable(
                            statement->name,
                            value_number(
                                number
                            ),
                            1
                        );
                    }
                    else
                    {
                        set_variable(
                            statement->name,
                            value_string(
                                buffer
                            ),
                            0
                        );
                    }
                }

                break;
            }

            case STMT_OUTPUT:
                execute_output(
                    statement->expression
                );
                break;

            case STMT_IF:
            {
                Value condition =
                    evaluate(
                        statement->condition
                    );

                int yes =
                    truthy(
                        &condition
                    );

                value_free(
                    &condition
                );

                if (yes)
                {
                    execute_statement(
                        statement->body
                    );
                }

                break;
            }

            case STMT_WHILE:
            {
                int guard = 0;

                while (guard++ < 100000)
                {
                    Value condition =
                        evaluate(
                            statement->condition
                        );

                    int yes =
                        truthy(
                            &condition
                        );

                    value_free(
                        &condition
                    );

                    if (!yes)
                        break;

                    execute_statement(
                        statement->body
                    );

                    if (loop_break)
                    {
                        loop_break = 0;
                        break;
                    }
                }

                break;
            }

            case STMT_FOR:
            {
                /*
                 * Initial value.
                 */
                Value start =
                    evaluate(
                        statement->expression
                    );

                set_variable(
                    statement->name,
                    start,
                    1
                );

                int guard = 0;

                while (guard++ < 100000)
                {
                    Value condition =
                        evaluate(
                            statement->condition
                        );

                    int yes =
                        truthy(
                            &condition
                        );

                    value_free(
                        &condition
                    );

                    if (!yes)
                        break;

                    execute_statement(
                        statement->body
                    );

                    Variable *v =
                        find_variable(
                            statement->name
                        );

                    if (!v ||
                        v->value.type !=
                            VALUE_NUMBER)
                    {
                        break;
                    }

                    v->value.number += 1;

                    if (loop_break)
                    {
                        loop_break = 0;
                        break;
                    }
                }

                break;
            }

            case STMT_ELSE_IF:
            case STMT_ELSE:
            case STMT_FUNCTION:
            case STMT_BREAK:
                /*
                 * Functions and the complete
                 * branch system are added in
                 * the next engine revision.
                 */
                break;
        }

        statement =
            statement->next;
    }
}

void interpreter_run(
    Statement *program
)
{
    variable_count = 0;
    loop_break = 0;

    execute_statement(
        program
    );

    for (int i = 0;
         i < variable_count;
         i++)
    {
        free(
            variables[i].name
        );

        value_free(
            &variables[i].value
        );
    }

    variable_count = 0;
}
