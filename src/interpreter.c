#define _GNU_SOURCE

#include "interpreter.h"
#include "value.h"
#include "output.h"

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
            strcmp(
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
            strcmp(
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
    char buffer[64];

    if (number == (long long)number)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "%lld",
            (long long)number
        );
    }
    else
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "%.12g",
            number
        );
    }

    lois_print(buffer);
}

static void print_value(Value *value)
{

    if (value->type == VALUE_STRING)
    {
        lois_print(value->string);
    }
    else if (value->type == VALUE_NUMBER)
    {
        print_number(value->number);
    }
    else if (value->type == VALUE_BOOL)
    {
        lois_print(
            value->number != 0
                ? "True"
                : "False"
        );
    }
    else if (value->type == VALUE_SET)
    {
        lois_print("{");

        for (int i = 0;
             i < value->set.count;
             i++)
        {
            if (i > 0)
                lois_print(",");

            print_value(
                &value->set.items[i]
            );
        }

        lois_print("}");
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

    /*
     * Set literal.
     *
     * EXPR_SET stores its elements as a linked Expr chain
     * through left/right.
     */
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

    /*
     * Set indexing is 1-based:
     *
     *     numbers1
     *     numbers2
     */
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

    /*
     * Conditional expression:
     *
     *     if condition then value else value
     *
     * Only the selected branch is evaluated.
     */
    if (expr->type == EXPR_CONDITIONAL)
    {
        Value condition =
            evaluate(expr->condition);

        if (condition.type == VALUE_NONE)
            return value_none();

        int result =
            truthy(&condition);

        value_free(&condition);

        if (result)
            return evaluate(expr->left);

        return evaluate(expr->right);
    }

    if (expr->type == EXPR_CALL)
    {
        /*
         * First check whether this is a user-defined LOIS
         * function.
         */
        Function *function =
            find_function(expr->call_name);

        if (function)
        {
            if (
                expr->call_argument_count !=
                function->parameter_count
            )
            {
                char message[256];

                snprintf(
                    message,
                    sizeof(message),
                    "function %s expects %d argument(s), got %d",
                    function->name,
                    function->parameter_count,
                    expr->call_argument_count
                );

                set_runtime_error(expr, message);

                return value_none();
            }

            /*
             * Evaluate arguments BEFORE installing
             * the function parameters.
             */
            Value arguments[16];

            for (int i = 0;
                 i < expr->call_argument_count;
                 i++)
            {
                arguments[i] =
                    evaluate(
                        expr->call_arguments[i]
                    );

                if (arguments[i].type == VALUE_NONE)
                {
                    for (int j = 0; j < i; j++)
                        value_free(&arguments[j]);

                    return value_none();
                }
            }

            /*
             * Save variables which have the same names
             * as the function parameters.
             */
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

            /*
             * Remove temporary parameters.
             */
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

            /*
             * Restore variables that existed before
             * the function call.
             */
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
                 i < expr->call_argument_count;
                 i++)
            {
                value_free(&arguments[i]);
            }

            return result;
        }

        /*
         * Otherwise this must be a built-in math function.
         *
         * Built-in math functions currently take exactly
         * one argument.
         */
        if (expr->call_argument_count != 1)
        {
            char message[256];

            snprintf(
                message,
                sizeof(message),
                "math function '%s' expects exactly 1 argument",
                expr->call_name
            );

            set_runtime_error(expr, message);

            return value_none();
        }

        Value argument =
            evaluate(
                expr->call_arguments[0]
            );

        if (argument.type != VALUE_NUMBER)
        {
            char message[256];

            snprintf(
                message,
                sizeof(message),
                "math function '%s' requires a number",
                expr->call_name
            );

            set_runtime_error(expr, message);

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
        else if (strcasecmp(expr->call_name, "root5") == 0)
        {
            result = pow(x, 1.0 / 5.0);
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
            char message[256];

            snprintf(
                message,
                sizeof(message),
                "unknown function '%s'",
                expr->call_name
            );

            set_runtime_error(expr, message);

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

        /*
         * Factorial.
         *
         *     5!  -> 120
         *     0!  -> 1
         *
         * Factorial only accepts non-negative integers.
         */
        if (expr->operator == TOKEN_FACTORIAL)
        {
            if (right.type != VALUE_NUMBER)
            {
                set_runtime_error(
                    expr,
                    "factorial requires a number"
                );

                value_free(&right);
                return value_none();
            }

            double n = right.number;

            if (n < 0 || floor(n) != n)
            {
                set_runtime_error(
                    expr,
                    "factorial requires a non-negative integer"
                );

                value_free(&right);
                return value_none();
            }

            double result = 1;

            for (double i = 2; i <= n; i++)
                result *= i;

            value_free(&right);

            return value_number(result);
        }

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

        /*
         * Unary Boolean NOT.
         */
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
        /*
         * Logical operators.
         *
         * Always return real boolean values:
         * True / False
         */
        if (
            expr->operator == TOKEN_AND ||
            expr->operator == TOKEN_OR
        )
        {
            Value left =
                evaluate(expr->left);

            /*
             * Short-circuit AND/OR.
             */
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

        /*
         * String concatenation.
         *
         * Adjacent output pieces use + internally.
         */
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

        /*
         * Relational comparisons require numbers.
         *
         * Equality is handled separately below and may compare
         * values of the same supported type.
         */
        if (
            (
                expr->operator == TOKEN_GREATER ||
                expr->operator == TOKEN_LESS ||
                expr->operator == TOKEN_GREATER_EQUAL ||
                expr->operator == TOKEN_LESS_EQUAL
            ) &&
            (
                left.type != VALUE_NUMBER ||
                right.type != VALUE_NUMBER
            )
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

        /*
         * Numeric / boolean operations.
         *
         * Booleans behave numerically internally:
         *
         * True  = 1
         * False = 0
         *
         * But comparison results are returned as
         * actual VALUE_BOOL values.
         */
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
                /*
                 * Arithmetic is only valid for numbers.
                 */
                case TOKEN_PLUS:
                    if (
                        left.type != VALUE_NUMBER ||
                        right.type != VALUE_NUMBER
                    )
                    {
                        set_runtime_error(
                            expr,
                            "a number is expected for arithmetic"
                        );
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
                        set_runtime_error(
                            expr,
                            "a number is expected for arithmetic"
                        );
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
                        set_runtime_error(
                            expr,
                            "a number is expected for arithmetic"
                        );
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
                        set_runtime_error(
                            expr,
                            "addition requires numbers"
                        );

                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    if (r == 0)
                    {
                        set_runtime_error(
                            expr,
                            "LOIS: division by zero"
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
                        set_runtime_error(
                            expr,
                            "subtraction requires numbers"
                        );

                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    if (r == 0)
                    {
                        set_runtime_error(
                            expr,
                            "LOIS: modulo by zero"
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
                        set_runtime_error(
                            expr,
                            "a number is expected for arithmetic"
                        );
                        value_free(&left);
                        value_free(&right);
                        return value_none();
                    }

                    result = pow(l, r);

                    value_free(&left);
                    value_free(&right);

                    return value_number(result);

                /*
                 * Comparisons.
                 */
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

        /*
         * Boolean equality.
         *
         * True and False are real boolean values.
         * They are compared as booleans, not strings.
         */
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

        /*
         * String equality.
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

            return value_bool(result != 0);
        }

        value_free(&left);
        value_free(&right);
    }

    return value_none();
}

static Value evaluate_output_expr(Expr *expr)
{
    if (!expr)
        return value_none();

    /*
     * Ordinary output text.
     *
     * A literal word is normally printed exactly as written,
     * unless a variable with that exact name exists.
     *
     *     name is mike
     *     output is hello name
     *
     * gives:
     *     hello mike
     */
    if (expr->type == EXPR_LITERAL)
    {
        Variable *variable =
            find_variable(expr->text);

        if (variable)
            return value_copy(&variable->value);

        return value_string(expr->text);
    }

    /*
     * A real variable node still resolves normally.
     */
    if (expr->type == EXPR_VARIABLE)
    {
        Variable *variable =
            find_variable(expr->text);

        if (variable)
            return value_copy(&variable->value);

        return value_string(expr->text);
    }

    /*
     * Output concatenation is represented by +.
     * Evaluate both sides using output semantics so that
     * variables are substituted but ordinary words remain text.
     */
    if (
        expr->type == EXPR_BINARY &&
        expr->operator == TOKEN_PLUS
    )
    {
        Value left =
            evaluate_output_expr(expr->left);

        Value right =
            evaluate_output_expr(expr->right);

        if (
            left.type == VALUE_STRING ||
            right.type == VALUE_STRING
        )
        {
            char left_text[1024];
            char right_text[1024];

            if (left.type == VALUE_STRING)
                snprintf(
                    left_text,
                    sizeof(left_text),
                    "%s",
                    left.string
                );
            else if (left.type == VALUE_NUMBER)
                snprintf(
                    left_text,
                    sizeof(left_text),
                    "%.15g",
                    left.number
                );
            else
                left_text[0] = '\0';

            if (right.type == VALUE_STRING)
                snprintf(
                    right_text,
                    sizeof(right_text),
                    "%s",
                    right.string
                );
            else if (right.type == VALUE_NUMBER)
                snprintf(
                    right_text,
                    sizeof(right_text),
                    "%.15g",
                    right.number
                );
            else
                right_text[0] = '\0';

            char buffer[2048];

            snprintf(
                buffer,
                sizeof(buffer),
                "%s %s",
                left_text,
                right_text
            );

            value_free(&left);
            value_free(&right);

            return value_string(buffer);
        }

        value_free(&left);
        value_free(&right);
    }

    /*
     * Numbers, booleans, function calls, arithmetic, conditionals,
     * etc. retain normal expression semantics.
     */
    return evaluate(expr);
}

static void execute_output(Expr *expr)
{
    if (!expr)
        return;

    Value value =
        evaluate_output_expr(expr);

    /*
     * If evaluation failed, the error has already been
     * reported through set_runtime_error().
     *
     * Do not print VALUE_NONE or add a fake newline.
     */
    if (value.type == VALUE_NONE)
    {
        value_free(&value);
        return;
    }

    print_value(&value);

    value_free(&value);

    lois_print("\n");
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

    /*
     * A LOIS for loop starts at 1.
     *
     *     for x<=10
     *
     * means:
     *
     *     x = 1
     *     while x <= 10
     *         ...
     *         x = x + 1
     */
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

        /*
         * Execute the complete nested body.
         *
         * This is what allows:
         *
         *     for x<=10
         *     then for y<=10
         *     then output is x*y
         */
        execute_statement(
            statement->body
        );

        /*
         * Increment the loop variable.
         */
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

/*
 * WHILE LOOP
 *
 * Re-evaluate the condition before every iteration.
 *
 * Example:
 *
 *     x is 1
 *
 *     while x <= 5
 *     then output is x
 *     x = x + 1
 *
 * The loop keeps executing its body while the
 * condition remains truthy.
 */
static void execute_while(Statement *statement)
{
    /*
     * Safety guard against accidental infinite loops.
     *
     * This is deliberately generous. It prevents a
     * broken program from locking the interpreter
     * forever while still allowing normal loops.
     */
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

                /*
                 * The expression belongs to the parser tree.
                 * Do not free it here.
                 */
                statement->expression = NULL;

                break;
            }

            case STMT_FUNCTION_CALL:
            {
                Function *function =
                    find_function(statement->name);

                if (!function)
                {
                    char message[256];

                    snprintf(
                        message,
                        sizeof(message),
                        "LOIS: no function named %s",
                        statement->name
                    );

                    set_runtime_error(NULL, message);

                    break;
                }

                if (
                    statement->argument_count !=
                    function->parameter_count
                )
                {
                    char message[256];

                    snprintf(
                        message,
                        sizeof(message),
                        "LOIS: function %s expects %d argument(s), got %d",
                        function->name,
                        function->parameter_count,
                        statement->argument_count
                    );

                    set_runtime_error(NULL, message);

                    break;
                }

                /*
                 * Evaluate all arguments BEFORE replacing
                 * the parameter variables.
                 */
                Value arguments[16];

                for (int i = 0;
                     i < statement->argument_count;
                     i++)
                {
                    arguments[i] =
                        evaluate(
                            statement->arguments[i]
                        );

                    if (arguments[i].type == VALUE_NONE)
                    {
                        for (int j = 0; j < i; j++)
                            value_free(&arguments[j]);

                        break;
                    }
                }

                if (interpreter_had_error())
                {
                    break;
                }

                /*
                 * Save existing variables with the same
                 * names as parameters.
                 */
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

                /*
                 * Remove temporary parameter variables.
                 */
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

                /*
                 * Restore variables that existed before
                 * the call.
                 */
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
                lois_print("\n");

                value_free(&result);

                break;
            }

            case STMT_RETURN:
            {
                /*
                 * Return is currently an expression result.
                 *
                 * Bare "return" means 0.
                 */
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
                lois_print("\n");

                value_free(&result);

                break;
            }

            case STMT_ASSIGN:
            {
                /*
                 * x = expression
                 *
                 * Always numeric.
                 */
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

                /*
                 * name is num
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

                /*
                 * Only show a prompt when one was explicitly
                 * supplied with:
                 *
                 * input is name for "Enter your name"
                 */
                if (statement->expression)
                {
                    Value prompt =
                        evaluate(statement->expression);

                    if (prompt.type == VALUE_STRING)
                        printf("%s", prompt.string);

                    value_free(&prompt);
                }

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

                    if (
                        statement->extra &&
                        strcmp(
                            statement->extra,
                            "num"
                        ) == 0
                    )
                    {
                        char *end;

                        double number =
                            strtod(
                                buffer,
                                &end
                            );

                        while (*end == ' ' || *end == '\t')
                            end++;

                        if (
                            end == buffer ||
                            *end != '\0'
                        )
                        {
                            char message[256];

                            snprintf(
                                message,
                                sizeof(message),
                                "input for %s requires a number",
                                statement->name
                            );

                            set_runtime_error(
                                statement->expression,
                                message
                            );

                            break;
                        }

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
                else
                {
                    char message[256];

                    snprintf(
                        message,
                        sizeof(message),
                        "could not read input for %s",
                        statement->name
                    );

                    set_runtime_error(
                        statement->expression,
                        message
                    );
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
