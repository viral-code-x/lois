#define _GNU_SOURCE

#include "interpreter.h"
#include "value.h"

#include <math.h>
#include <stdio.h>

static int runtime_error = 0;

int interpreter_had_error(void)
{
    return runtime_error;
}

static void set_runtime_error(Expr *expr, const char *message)
{
    if (runtime_error)
        return;

    runtime_error = 1;

    fprintf(
        stderr,
        "Error in line %d:\n%s\n",
        expr ? expr->line : 0,
        message
    );
}


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

#define MAX_FUNCTIONS 256

typedef struct
{
    char *name;

    char *parameters[16];
    int parameter_count;

    Expr *expression;
} Function;

static Function functions[MAX_FUNCTIONS];
static int function_count = 0;

static Function *find_function(const char *name)
{
    for (int i = 0; i < function_count; i++)
    {
        if (
            strcasecmp(
                functions[i].name,
                name
            ) == 0
        )
        {
            return &functions[i];
        }
    }

    return NULL;
}

static void set_function(
    const char *name,
    char **parameters,
    int parameter_count,
    Expr *expression
)
{
    Function *existing =
        find_function(name);

    if (existing)
    {
        for (int i = 0; i < existing->parameter_count; i++)
            free(existing->parameters[i]);

        existing->parameter_count =
            parameter_count;

        for (int i = 0; i < parameter_count; i++)
        {
            existing->parameters[i] =
                strdup(parameters[i]);
        }

        existing->expression =
            expression;

        return;
    }

    if (function_count >= MAX_FUNCTIONS)
    {
        fprintf(
            stderr,
            "LOIS: too many functions\n"
        );

        exit(1);
    }

    functions[function_count].name =
        strdup(name);

    functions[function_count].parameter_count =
        parameter_count;

    for (int i = 0; i < parameter_count; i++)
    {
        functions[function_count].parameters[i] =
            strdup(parameters[i]);
    }

    functions[function_count].expression =
        expression;

    function_count++;
}

static void free_functions(void)
{
    for (int i = 0; i < function_count; i++)
    {
        free(functions[i].name);

        for (int j = 0; j < functions[i].parameter_count; j++)
            free(functions[i].parameters[j]);
    }

    function_count = 0;
}

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

static void print_number(double number)
{
    if (number == (long long)number)
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
    if (value->type == VALUE_STRING)
    {
        printf("%s", value->string);
    }
    else if (value->type == VALUE_NUMBER)
    {
        print_number(value->number);
    }
    else if (value->type == VALUE_BOOL)
    {
        printf(
            "%s",
            value->number != 0
                ? "True"
                : "False"
        );
    }
    else if (value->type == VALUE_SET)
    {
        printf("{");

        for (int i = 0;
             i < value->set.count;
             i++)
        {
            if (i > 0)
                printf(",");

            print_value(
                &value->set.items[i]
            );
        }

        printf("}");
    }
}

static int truthy(Value *value)
{
    if (value->type == VALUE_NUMBER)
        return value->number != 0;

    if (value->type == VALUE_BOOL)
        return value->number != 0;

    if (value->type == VALUE_STRING)
        return value->string[0] != '\0';

    return 0;
}

static Value evaluate(Expr *expr)
{
    if (!expr)
        return value_none();

    if (expr->type == EXPR_LITERAL)
        return value_string(expr->text);

    if (expr->type == EXPR_SET)
    {
        Value set = value_set();

        Expr *element = expr->left;

        while (element)
        {
            Value item =
                evaluate(element);

            if (item.type == VALUE_NONE)
            {
                value_free(&set);
                return value_none();
            }

            value_set_add(&set, item);

            element = element->right;
        }

        return set;
    }

    if (expr->type == EXPR_INDEX)
    {
        Value target =
            evaluate(expr->index_target);

        if (target.type != VALUE_SET)
        {
            fprintf(
                stderr,
                "LOIS: '%s' is not a set\n",
                expr->index_target &&
                expr->index_target->text
                    ? expr->index_target->text
                    : "value"
            );

            value_free(&target);
            return value_none();
        }

        Value *item =
            value_set_get(
                &target,
                expr->index
            );

        if (!item)
        {
            fprintf(
                stderr,
                "LOIS: set index %d out of range\n",
                expr->index
            );

            value_free(&target);
            return value_none();
        }

        Value result =
            value_copy(item);

        value_free(&target);

        return result;
    }

    if (expr->type == EXPR_NUMBER)
        return value_number(expr->number);

    if (expr->type == EXPR_BOOLEAN)
        return value_bool((int)expr->number);

    if (expr->type == EXPR_VARIABLE)
    {
        Variable *variable =
            find_variable(expr->text);

        if (!variable)
        {
            char message[256];

            snprintf(
                message,
                sizeof(message),
                "%s is not defined",
                expr->text
            );

            set_runtime_error(expr, message);

            return value_none();
        }

        return value_copy(
            &variable->value
        );
    }

    if (expr->type == EXPR_CALL)
    {
        Value argument =
            evaluate(expr->call_argument);

        if (argument.type != VALUE_NUMBER)
        {
            fprintf(
                stderr,
                "LOIS: math function '%s' requires a number\n",
                expr->call_name
            );

            value_free(&argument);
            return value_none();
        }

        double x =
            argument.number;

        double result;

        if (strcasecmp(expr->call_name, "root") == 0)
        {
            if (x < 0)
            {
                set_runtime_error(
                    expr,
                    "root requires a non-negative number"
                );

                value_free(&argument);
                return value_none();
            }

            result = sqrt(x);
        }
        else if (strcasecmp(expr->call_name, "root3") == 0)
        {
            result = cbrt(x);
        }
        else if (strcasecmp(expr->call_name, "root4") == 0)
        {
            if (x < 0)
            {
                set_runtime_error(
                    expr,
                    "root4 requires a non-negative number"
                );

                value_free(&argument);
                return value_none();
            }

            result = pow(x, 1.0 / 4.0);
        }
        else if (strcasecmp(expr->call_name, "sin") == 0)
        {
            result = sin(x);
        }
        else if (strcasecmp(expr->call_name, "cos") == 0)
        {
            result = cos(x);
        }
        else if (strcasecmp(expr->call_name, "tan") == 0)
        {
            if (fabs(cos(x)) < 1e-12)
            {
                set_runtime_error(
                    expr,
                    "tan is undefined at this value"
                );

                value_free(&argument);
                return value_none();
            }

            result = tan(x);
        }
        else if (strcasecmp(expr->call_name, "log") == 0)
        {
            if (x <= 0)
            {
                set_runtime_error(
                    expr,
                    "log requires a positive number"
                );

                value_free(&argument);
                return value_none();
            }

            result = log10(x);
        }
        else if (strcasecmp(expr->call_name, "ln") == 0)
        {
            if (x <= 0)
            {
                set_runtime_error(
                    expr,
                    "ln requires a positive number"
                );

                value_free(&argument);
                return value_none();
            }

            result = log(x);
        }
        else if (strcasecmp(expr->call_name, "abs") == 0)
        {
            result = fabs(x);
        }
        else if (strcasecmp(expr->call_name, "round") == 0)
        {
            result = round(x);
        }
        else if (strcasecmp(expr->call_name, "floor") == 0)
        {
            result = floor(x);
        }
        else if (strcasecmp(expr->call_name, "ceil") == 0)
        {
            result = ceil(x);
        }
        else
        {
            fprintf(
                stderr,
                "LOIS: unknown math function '%s'\n",
                expr->call_name
            );

            value_free(&argument);
            return value_none();
        }

        value_free(&argument);

        return value_number(result);
    }

    if (expr->type == EXPR_UNARY)
    {
        Value right =
            evaluate(expr->right);

        if (expr->operator == TOKEN_MINUS)
        {
            if (right.type != VALUE_NUMBER)
            {
                value_free(&right);
                return value_none();
            }

            double result =
                -right.number;

            value_free(&right);

            return value_number(result);
        }

        if (expr->operator == TOKEN_NOT)
        {
            int result =
                truthy(&right) ? 0 : 1;

            value_free(&right);

            return value_bool(result);
        }

        value_free(&right);

        return value_none();
    }

    if (expr->type == EXPR_BINARY)
    {
        if (
            expr->operator == TOKEN_AND ||
            expr->operator == TOKEN_OR
        )
        {
            Value left =
                evaluate(expr->left);

            int left_true =
                truthy(&left);

            if (
                expr->operator == TOKEN_AND &&
                !left_true
            )
            {
                value_free(&left);
                return value_bool(0);
            }

            if (
                expr->operator == TOKEN_OR &&
                left_true
            )
            {
                value_free(&left);
                return value_bool(1);
            }

            value_free(&left);

            Value right =
                evaluate(expr->right);

            int right_true =
                truthy(&right);

            value_free(&right);

            return value_bool(
                right_true ? 1 : 0
            );
        }

        Value left =
            evaluate(expr->left);

        if (left.type == VALUE_NONE)
            return value_none();

        Value right =
            evaluate(expr->right);

        if (right.type == VALUE_NONE)
            return value_none();

        if (
            expr->operator == TOKEN_PLUS &&
            (
                left.type == VALUE_STRING ||
                right.type == VALUE_STRING
            )
        )
        {
            char left_text[2048];
            char right_text[2048];

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
                    "%.12g",
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
                    "%.12g",
                    right.number
                );
            }

            char result[4096];

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

        if (
            (
                left.type == VALUE_NUMBER ||
                left.type == VALUE_BOOL
            ) &&
            (
                right.type == VALUE_NUMBER ||
                right.type == VALUE_BOOL
            )
        )
        {
            double l =
                left.number;

            double r =
                right.number;

            double result = 0;

            switch (expr->operator)
            {
                case TOKEN_PLUS:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    result = l + r;

                    value_free(&left);
                    value_free(&right);

                    return value_number(result);

                case TOKEN_MINUS:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    result = l - r;

                    value_free(&left);
                    value_free(&right);

                    return value_number(result);

                case TOKEN_STAR:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    result = l * r;

                    value_free(&left);
                    value_free(&right);

                    return value_number(result);

                case TOKEN_SLASH:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    if (r == 0)
                    {
                        fprintf(
                            stderr,
                            "LOIS: division by zero\n"
                        );

                        value_free(&left);
                        value_free(&right);

                        return value_none();
                    }

                    result = l / r;

                    value_free(&left);
                    value_free(&right);

                    return value_number(result);

                case TOKEN_PERCENT:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    if (r == 0)
                    {
                        fprintf(
                            stderr,
                            "LOIS: modulo by zero\n"
                        );

                        value_free(&left);
                        value_free(&right);

                        return value_none();
                    }

                    result = fmod(l, r);

                    value_free(&left);
                    value_free(&right);

                    return value_number(result);

                case TOKEN_CARET:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    result = pow(l, r);

                    value_free(&left);
                    value_free(&right);

                    return value_number(result);

                case TOKEN_GREATER:
                case TOKEN_LESS:
                case TOKEN_GREATER_EQUAL:
                case TOKEN_LESS_EQUAL:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        const char *op = "<";

                        if (expr->operator == TOKEN_GREATER)
                            op = ">";
                        else if (expr->operator == TOKEN_GREATER_EQUAL)
                            op = ">=";
                        else if (expr->operator == TOKEN_LESS_EQUAL)
                            op = "<=";

                        char message[128];

                        snprintf(
                            message,
                            sizeof(message),
                            "a number is expected after the \"%s\"",
                            op
                        );

                        set_runtime_error(expr, message);

                        value_free(&left);
                        value_free(&right);

                        return value_none();
                    }

                    if (expr->operator == TOKEN_GREATER)
                        result = l > r;
                    else if (expr->operator == TOKEN_LESS)
                        result = l < r;
                    else if (expr->operator == TOKEN_GREATER_EQUAL)
                        result = l >= r;
                    else
                        result = l <= r;

                    break;

                case TOKEN_EQUAL_EQUAL:
                case TOKEN_NOT_EQUAL:
                    if (
                        left.type != right.type
                    )
                    {
                        value_free(&left);
                        value_free(&right);

                        set_runtime_error(
                            expr,
                            "values must have the same type"
                        );

                        return value_none();
                    }

                    if (
                        left.type != VALUE_NUMBER &&
                        left.type != VALUE_BOOL
                    )
                    {
                        value_free(&left);
                        value_free(&right);

                        set_runtime_error(
                            expr,
                            "a number or boolean is expected"
                        );

                        return value_none();
                    }

                    result =
                        expr->operator == TOKEN_EQUAL_EQUAL
                            ? l == r
                            : l != r;

                    break;

                case TOKEN_AND:
                    result =
                        (l != 0) &&
                        (r != 0);

                    break;

                case TOKEN_OR:
                    result =
                        (l != 0) ||
                        (r != 0);

                    break;

                default:
                    value_free(&left);
                    value_free(&right);

                    return value_none();
            }

            value_free(&left);
            value_free(&right);

            return value_bool(
                result != 0
            );
        }

        if (
            left.type == VALUE_BOOL &&
            right.type == VALUE_BOOL
        )
        {
            int left_bool =
                left.number != 0;

            int right_bool =
                right.number != 0;

            int result;

            if (
                expr->operator ==
                TOKEN_EQUAL_EQUAL
            )
            {
                result =
                    left_bool == right_bool;
            }
            else if (
                expr->operator ==
                TOKEN_NOT_EQUAL
            )
            {
                result =
                    left_bool != right_bool;
            }
            else
            {
                value_free(&left);
                value_free(&right);

                return value_none();
            }

            value_free(&left);
            value_free(&right);

            return value_bool(result);
        }

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

            double result;

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
            else
            {
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



static void execute_statement(Statement *statement);

static void execute_for(Statement *statement)
{
    if (
        !statement ||
        !statement->condition ||
        !statement->loop_variable
    )
        return;

    set_variable(
        statement->loop_variable,
        value_number(1),
        1
    );

    unsigned long long iterations = 0;

    const unsigned long long MAX_LOOP_ITERATIONS =
        1000000ULL;

    while (1)
    {
        Value condition =
            evaluate(statement->condition);

        int true_value =
            truthy(&condition);

        value_free(&condition);

        if (!true_value)
            break;

        execute_statement(
            statement->body
        );

        Variable *variable =
            find_variable(
                statement->loop_variable
            );

        if (
            !variable ||
            variable->value.type != VALUE_NUMBER
        )
        {
            fprintf(
                stderr,
                "LOIS: for loop variable '%s' must remain a number\n",
                statement->loop_variable
            );

            break;
        }

        variable->value.number += 1;

        iterations++;

        if (iterations >= MAX_LOOP_ITERATIONS)
        {
            fprintf(
                stderr,
                "LOIS: for loop exceeded maximum iteration limit\n"
            );

            break;
        }
    }
}



static void execute_repeat(Statement *statement)
{
    if (!statement || !statement->count)
        return;

    Value count =
        evaluate(statement->count);

    if (
        count.type != VALUE_NUMBER &&
        count.type != VALUE_BOOL
    )
    {
        fprintf(
            stderr,
            "LOIS: repeat count must be a number\n"
        );

        value_free(&count);
        return;
    }

    int repetitions =
        (int)count.number;

    value_free(&count);

    if (repetitions <= 0)
        return;

    for (int i = 0; i < repetitions; i++)
    {
        execute_statement(statement->body);
    }
}



static void execute_if(Statement *statement)
{
    Value condition =
        evaluate(statement->condition);

    int true_value =
        truthy(&condition);

    value_free(&condition);

    if (true_value)
    {
        execute_statement(
            statement->body
        );
    }
    else if (statement->else_body)
    {
        execute_statement(
            statement->else_body
        );
    }
}

static void execute_while(Statement *statement)
{
    unsigned long long iterations = 0;

    const unsigned long long MAX_LOOP_ITERATIONS =
        1000000ULL;

    while (1)
    {
        Value condition =
            evaluate(statement->condition);

        int true_value =
            truthy(&condition);

        value_free(&condition);

        if (!true_value)
            break;

        execute_statement(
            statement->body
        );

        iterations++;

        if (iterations >= MAX_LOOP_ITERATIONS)
        {
            fprintf(
                stderr,
                "LOIS: loop exceeded maximum iteration limit\n"
            );

            break;
        }
    }
}

static void execute_statement(Statement *statement)
{
    while (statement)
    {
        switch (statement->type)
        {
            case STMT_FUNCTION:
            {
                set_function(
                    statement->name,
                    statement->parameters,
                    statement->parameter_count,
                    statement->expression
                );

                statement->expression = NULL;

                break;
            }

            case STMT_FUNCTION_CALL:
            {
                Function *function =
                    find_function(statement->name);

                if (!function)
                {
                    fprintf(
                        stderr,
                        "LOIS: no function named %s\n",
                        statement->name
                    );

                    break;
                }

                if (
                    statement->argument_count !=
                    function->parameter_count
                )
                {
                    fprintf(
                        stderr,
                        "LOIS: function %s expects %d argument(s), got %d\n",
                        function->name,
                        function->parameter_count,
                        statement->argument_count
                    );

                    break;
                }

                Value arguments[16];

                for (int i = 0;
                     i < statement->argument_count;
                     i++)
                {
                    arguments[i] =
                        evaluate(
                            statement->arguments[i]
                        );
                }

                Variable saved[16];
                int saved_count = 0;

                for (int i = 0;
                     i < function->parameter_count;
                     i++)
                {
                    Variable *existing =
                        find_variable(
                            function->parameters[i]
                        );

                    if (existing)
                    {
                        saved[saved_count].name =
                            strdup(existing->name);

                        saved[saved_count].value =
                            value_copy(&existing->value);

                        saved[saved_count].is_num =
                            existing->is_num;

                        saved_count++;
                    }

                    set_variable(
                        function->parameters[i],
                        value_copy(&arguments[i]),
                        arguments[i].type == VALUE_NUMBER
                    );
                }

                Value result;

                if (function->expression)
                {
                    result =
                        evaluate(
                            function->expression
                        );
                }
                else
                {
                    result =
                        value_number(0);
                }

                for (int i = 0;
                     i < function->parameter_count;
                     i++)
                {
                    Variable *variable =
                        find_variable(
                            function->parameters[i]
                        );

                    if (variable)
                    {
                        value_free(
                            &variable->value
                        );

                        free(variable->name);

                        int index =
                            (int)(variable - variables);

                        for (int j = index;
                             j < variable_count - 1;
                             j++)
                        {
                            variables[j] =
                                variables[j + 1];
                        }

                        variable_count--;
                    }
                }

                for (int i = 0;
                     i < saved_count;
                     i++)
                {
                    set_variable(
                        saved[i].name,
                        saved[i].value,
                        saved[i].is_num
                    );

                    free(saved[i].name);
                }

                for (int i = 0;
                     i < statement->argument_count;
                     i++)
                {
                    value_free(&arguments[i]);
                }

                print_value(&result);
                printf("\n");

                value_free(&result);

                break;
            }

            case STMT_RETURN:
            {
                Value result;

                if (statement->expression)
                {
                    result =
                        evaluate(
                            statement->expression
                        );
                }
                else
                {
                    result =
                        value_number(0);
                }

                print_value(&result);
                printf("\n");

                value_free(&result);

                break;
            }

            case STMT_ASSIGN:
            {
                if (
                    statement->extra &&
                    strcasecmp(
                        statement->extra,
                        "num"
                    ) == 0 &&
                    statement->expression
                )
                {
                    Value value =
                        evaluate(
                            statement->expression
                        );

                    if (
                        value.type !=
                        VALUE_NUMBER
                    )
                    {
                        fprintf(
                            stderr,
                            "LOIS: expected number in numeric assignment to %s\n",
                            statement->name
                        );

                        value_free(&value);
                        break;
                    }

                    set_variable(
                        statement->name,
                        value,
                        1
                    );

                    break;
                }

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

                set_variable(
                    statement->name,
                    value,
                    value.type == VALUE_NUMBER
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
                execute_if(statement);
                break;

            case STMT_REPEAT:
                execute_repeat(statement);
                break;

            case STMT_WHILE:
                execute_while(statement);
                break;

            case STMT_FOR:
                execute_for(statement);
                break;
        }

        statement =
            statement->next;
    }
}

void interpreter_run(Statement *program)
{
    runtime_error = 0;
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

    free_functions();
}
