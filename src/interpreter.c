#include "interpreter.h"
#include "value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
        if (strcmp(variables[i].name, name) == 0)
            return &variables[i];
    }

    return NULL;
}

static void set_variable(
    const char *name,
    Value value,
    int is_num
)
{
    Variable *variable = find_variable(name);

    if (variable)
    {
        value_free(&variable->value);
        variable->value = value;
        variable->is_num = is_num;
        return;
    }

    if (variable_count >= MAX_VARIABLES)
    {
        fprintf(stderr, "LOIS: too many variables\n");
        value_free(&value);
        return;
    }

    variables[variable_count].name = strdup(name);
    variables[variable_count].value = value;
    variables[variable_count].is_num = is_num;

    variable_count++;
}

static char *value_to_string(Value value)
{
    char buffer[128];

    if (value.type == VALUE_STRING)
        return strdup(value.string);

    if (value.type == VALUE_NUMBER)
    {
        if (fabs(value.number - round(value.number)) < 0.0000001)
        {
            snprintf(buffer, sizeof(buffer), "%.0f", value.number);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%g", value.number);
        }

        return strdup(buffer);
    }

    return strdup("");
}

static Value evaluate(Expr *expr)
{
    if (!expr)
        return value_none();

    if (expr->type == EXPR_LITERAL)
        return value_string(expr->text);

    if (expr->type == EXPR_NUMBER)
        return value_number(expr->number);

    if (expr->type == EXPR_VARIABLE)
    {
        Variable *variable = find_variable(expr->text);

        if (!variable)
            return value_string(expr->text);

        if (variable->value.type == VALUE_NUMBER)
            return value_number(variable->value.number);

        return value_string(variable->value.string);
    }

    if (expr->type == EXPR_BINARY)
    {
        Value left = evaluate(expr->left);
        Value right = evaluate(expr->right);

        if (
            left.type == VALUE_NUMBER &&
            right.type == VALUE_NUMBER
        )
        {
            double result = 0;

            switch (expr->operator)
            {
                case TOKEN_PLUS:
                    result = left.number + right.number;
                    break;

                case TOKEN_MINUS:
                    result = left.number - right.number;
                    break;

                case TOKEN_STAR:
                    result = left.number * right.number;
                    break;

                case TOKEN_SLASH:
                    if (right.number == 0)
                    {
                        fprintf(stderr, "LOIS: division by zero\n");

                        value_free(&left);
                        value_free(&right);

                        return value_none();
                    }

                    result = left.number / right.number;
                    break;

                case TOKEN_GREATER:
                    result = left.number > right.number;
                    break;

                case TOKEN_LESS:
                    result = left.number < right.number;
                    break;

                case TOKEN_GREATER_EQUAL:
                    result = left.number >= right.number;
                    break;

                case TOKEN_LESS_EQUAL:
                    result = left.number <= right.number;
                    break;

                case TOKEN_EQUAL_EQUAL:
                    result = left.number == right.number;
                    break;

                case TOKEN_NOT_EQUAL:
                    result = left.number != right.number;
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

        if (expr->operator == TOKEN_PLUS)
        {
            char *left_text = value_to_string(left);
            char *right_text = value_to_string(right);

            size_t length =
                strlen(left_text) +
                strlen(right_text) +
                1;

            char *combined = malloc(length);

            if (!combined)
            {
                free(left_text);
                free(right_text);
                value_free(&left);
                value_free(&right);

                return value_none();
            }

            strcpy(combined, left_text);
            strcat(combined, right_text);

            Value result = value_string(combined);

            free(combined);
            free(left_text);
            free(right_text);

            value_free(&left);
            value_free(&right);

            return result;
        }

        value_free(&left);
        value_free(&right);

        return value_none();
    }

    return value_none();
}

static void print_value(Value *value)
{
    if (value->type == VALUE_STRING)
    {
        printf("%s\n", value->string);
        return;
    }

    if (value->type == VALUE_NUMBER)
    {
        if (fabs(value->number - round(value->number)) < 0.0000001)
        {
            printf("%.0f\n", value->number);
        }
        else
        {
            printf("%g\n", value->number);
        }
    }
}

static void run_statement(Statement *statement)
{
    if (!statement)
        return;

    if (statement->type == STMT_ASSIGN)
    {
        if (
            statement->extra &&
            strcmp(statement->extra, "num") == 0
        )
        {
            set_variable(
                statement->name,
                value_number(0),
                1
            );

            return;
        }

        Value value = evaluate(statement->expression);

        set_variable(
            statement->name,
            value,
            0
        );

        return;
    }

    if (statement->type == STMT_INPUT)
    {
        char buffer[1024];

        printf("%s: ", statement->name);

        if (fgets(buffer, sizeof(buffer), stdin))
        {
            buffer[strcspn(buffer, "\n")] = '\0';

            set_variable(
                statement->name,
                value_string(buffer),
                0
            );
        }

        return;
    }

    if (statement->type == STMT_OUTPUT)
    {
        Value value = evaluate(statement->expression);

        print_value(&value);

        value_free(&value);

        return;
    }

    if (statement->type == STMT_IF)
    {
        Value condition =
            evaluate(statement->condition);

        if (
            condition.type == VALUE_NUMBER &&
            condition.number != 0
        )
        {
            run_statement(statement->body);
        }

        value_free(&condition);
    }
}

void interpreter_run(Statement *program)
{
    Statement *current = program;

    while (current)
    {
        run_statement(current);
        current = current->next;
    }

    for (int i = 0; i < variable_count; i++)
    {
        free(variables[i].name);
        value_free(&variables[i].value);
    }

    variable_count = 0;
}
