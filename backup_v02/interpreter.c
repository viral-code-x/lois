#define _GNU_SOURCE

#include "interpreter.h"
#include "value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define MAX_VARIABLES 256

typedef struct
{
    char *name;
    Value value;
    int is_num;
} Variable;

static Variable variables[MAX_VARIABLES];
static int variable_count = 0;

static Variable *find_variable(const char *name)
{
    for (int i = 0; i < variable_count; i++)
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
        value_free(&existing->value);

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

static void print_value(Value *value)
{
    if (value->type == VALUE_STRING)
    {
        printf("%s", value->string);
    }
    else if (value->type == VALUE_NUMBER)
    {
        if (
            value->number ==
            (long long)value->number
        )
        {
            printf(
                "%lld",
                (long long)value->number
            );
        }
        else
        {
            printf(
                "%g",
                value->number
            );
        }
    }
}

static Value evaluate(Expr *expr)
{
    if (!expr)
        return value_none();

    /*
     * Literal string.
     */
    if (expr->type == EXPR_LITERAL)
        return value_string(expr->text);

    /*
     * Number.
     */
    if (expr->type == EXPR_NUMBER)
        return value_number(expr->number);

    /*
     * Variable.
     */
    if (expr->type == EXPR_VARIABLE)
    {
        Variable *variable =
            find_variable(expr->text);

        if (!variable)
        {
            /*
             * Unknown words inside output are
             * treated as literal text.
             */
            return value_string(expr->text);
        }

        return value_copy(
            &variable->value
        );
    }

    /*
     * Binary expression.
     */
    if (expr->type == EXPR_BINARY)
    {
        Value left =
            evaluate(expr->left);

        Value right =
            evaluate(expr->right);

        /*
         * String concatenation.
         *
         * LOIS:
         *
         * hello + name
         *
         * becomes:
         *
         * hello Alex
         */
        if (
            expr->operator == TOKEN_PLUS &&
            (
                left.type == VALUE_STRING ||
                right.type == VALUE_STRING
            )
        )
        {
            char left_text[1024];
            char right_text[1024];

            if (left.type == VALUE_STRING)
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
                    "%g",
                    left.number
                );
            }

            if (right.type == VALUE_STRING)
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
                    "%g",
                    right.number
                );
            }

            char result[2048];

            snprintf(
                result,
                sizeof(result),
                "%s %s",
                left_text,
                right_text
            );

            value_free(&left);
            value_free(&right);

            return value_string(result);
        }

        /*
         * Number arithmetic/comparison.
         */
        if (
            left.type == VALUE_NUMBER &&
            right.type == VALUE_NUMBER
        )
        {
            double result = 0;

            switch (expr->operator)
            {
                case TOKEN_PLUS:
                    result =
                        left.number +
                        right.number;
                    break;

                case TOKEN_MINUS:
                    result =
                        left.number -
                        right.number;
                    break;

                case TOKEN_STAR:
                    result =
                        left.number *
                        right.number;
                    break;

                case TOKEN_SLASH:
                    if (right.number == 0)
                    {
                        fprintf(
                            stderr,
                            "LOIS: division by zero\n"
                        );

                        value_free(&left);
                        value_free(&right);

                        return value_none();
                    }

                    result =
                        left.number /
                        right.number;

                    break;

                case TOKEN_GREATER:
                    result =
                        left.number >
                        right.number;
                    break;

                case TOKEN_LESS:
                    result =
                        left.number <
                        right.number;
                    break;

                case TOKEN_GREATER_EQUAL:
                    result =
                        left.number >=
                        right.number;
                    break;

                case TOKEN_LESS_EQUAL:
                    result =
                        left.number <=
                        right.number;
                    break;

                case TOKEN_EQUAL_EQUAL:
                    result =
                        left.number ==
                        right.number;
                    break;

                case TOKEN_NOT_EQUAL:
                    result =
                        left.number !=
                        right.number;
                    break;

                default:
                    value_free(&left);
                    value_free(&right);

                    return value_none();
            }

            value_free(&left);
            value_free(&right);

            return value_number(result);
        }

        /*
         * String comparison.
         */
        if (
            left.type == VALUE_STRING &&
            right.type == VALUE_STRING
        )
        {
            int comparison =
                strcmp(
                    left.string,
                    right.string
                );

            double result = 0;

            switch (expr->operator)
            {
                case TOKEN_EQUAL_EQUAL:
                    result = comparison == 0;
                    break;

                case TOKEN_NOT_EQUAL:
                    result = comparison != 0;
                    break;

                default:
                    value_free(&left);
                    value_free(&right);

                    return value_none();
            }

            value_free(&left);
            value_free(&right);

            return value_number(result);
        }

        value_free(&left);
        value_free(&right);
    }

    return value_none();
}

static void execute_output(Expr *expr)
{
    if (!expr)
        return;

    Value value =
        evaluate(expr);

    print_value(&value);

    value_free(&value);

    printf("\n");
}

static void execute_statement(Statement *statement)
{
    while (statement)
    {
        switch (statement->type)
        {
            case STMT_ASSIGN:
            {
                /*
                 * age is num.
                 */
                if (
                    statement->extra &&
                    strcmp(
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

                /*
                 * Normal assignment.
                 */
                Value value =
                    evaluate(
                        statement->expression
                    );

                int is_num =
                    value.type == VALUE_NUMBER;

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

                if (fgets(
                    buffer,
                    sizeof(buffer),
                    stdin
                ))
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

                    /*
                     * If variable was previously
                     * declared num, parse as number.
                     */
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
                            value_number(number),
                            1
                        );
                    }
                    else
                    {
                        set_variable(
                            statement->name,
                            value_string(buffer),
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

                int true_value =
                    condition.type ==
                    VALUE_NUMBER &&
                    condition.number != 0;

                value_free(&condition);

                if (true_value)
                {
                    execute_statement(
                        statement->body
                    );
                }

                break;
            }
        }

        statement =
            statement->next;
    }
}

void interpreter_run(Statement *program)
{
    variable_count = 0;

    execute_statement(program);

    for (int i = 0; i < variable_count; i++)
    {
        free(variables[i].name);

        value_free(
            &variables[i].value
        );
    }

    variable_count = 0;
}
