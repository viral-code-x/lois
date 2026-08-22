#define _GNU_SOURCE

#include "interpreter.h"
#include "value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define MAX_VARIABLES 512
#define MAX_FUNCTIONS 128

typedef struct
{
    char *name;

    Value value;

    int is_num;

    int is_bool;
}
Variable;

typedef struct
{
    char *name;

    char **params;

    int param_count;

    Expr *body;
}
Function;

static Variable variables[
    MAX_VARIABLES
];

static int variable_count;

static Function functions[
    MAX_FUNCTIONS
];

static int function_count;

static Variable *find_variable(
    const char *name
)
{
    for (
        int i = 0;
        i < variable_count;
        i++
    )
    {
        if (
            strcasecmp(
                variables[i].name,
                name
            ) == 0
        )
        {
            return
                &variables[i];
        }
    }

    return NULL;
}

static void set_variable(
    const char *name,
    Value value,
    int is_num,
    int is_bool
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

        existing->is_bool = is_bool;

        return;
    }

    if (
        variable_count >=
        MAX_VARIABLES
    )
    {
        fprintf(
            stderr,
            "LOIS: too many variables\n"
        );

        exit(1);
    }

    variables[
        variable_count
    ].name =
        strdup(name);

    variables[
        variable_count
    ].value = value;

    variables[
        variable_count
    ].is_num = is_num;

    variables[
        variable_count
    ].is_bool = is_bool;

    variable_count++;
}

static int truth(
    Value *value
)
{
    if (
        value->type ==
        VALUE_BOOLEAN
    )
    {
        return value->boolean;
    }

    if (
        value->type ==
        VALUE_NUMBER
    )
    {
        return value->number != 0;
    }

    if (
        value->type ==
        VALUE_STRING
    )
    {
        return
            value->string &&
            value->string[0] != '\0';
    }

    return 0;
}

static void value_to_text(
    Value *value,
    char *buffer,
    size_t size
)
{
    if (
        value->type ==
        VALUE_STRING
    )
    {
        snprintf(
            buffer,
            size,
            "%s",
            value->string
        );

        return;
    }

    if (
        value->type ==
        VALUE_BOOLEAN
    )
    {
        snprintf(
            buffer,
            size,
            "%s",
            value->boolean
                ? "true"
                : "false"
        );

        return;
    }

    if (
        value->type ==
        VALUE_NUMBER
    )
    {
        if (
            value->number ==
            (long long)value->number
        )
        {
            snprintf(
                buffer,
                size,
                "%lld",
                (long long)
                    value->number
            );
        }
        else
        {
            snprintf(
                buffer,
                size,
                "%.12g",
                value->number
            );
        }

        return;
    }

    buffer[0] = '\0';
}

static Value evaluate(
    Expr *expr
);

static Value call_function(
    const char *name,
    Expr **args,
    int arg_count
)
{
    /*
     * Built-in pi.
     */
    if (
        strcasecmp(
            name,
            "pi"
        ) == 0 &&
        arg_count == 0
    )
    {
        return value_number(M_PI);
    }

    /*
     * sin()
     */
    if (
        strcasecmp(name, "sin") == 0 &&
        arg_count == 1
    )
    {
        Value a =
            evaluate(args[0]);

        double x =
            a.number;

        value_free(&a);

        return value_number(
            sin(x)
        );
    }

    /*
     * cos()
     */
    if (
        strcasecmp(name, "cos") == 0 &&
        arg_count == 1
    )
    {
        Value a =
            evaluate(args[0]);

        double x =
            a.number;

        value_free(&a);

        return value_number(
            cos(x)
        );
    }

    /*
     * log = base 10.
     */
    if (
        strcasecmp(name, "log") == 0 &&
        arg_count == 1
    )
    {
        Value a =
            evaluate(args[0]);

        double x =
            a.number;

        value_free(&a);

        return value_number(
            log10(x)
        );
    }

    /*
     * ln = natural logarithm.
     */
    if (
        strcasecmp(name, "ln") == 0 &&
        arg_count == 1
    )
    {
        Value a =
            evaluate(args[0]);

        double x =
            a.number;

        value_free(&a);

        return value_number(
            log(x)
        );
    }

    /*
     * int(x)
     *
     * Current meaning:
     * integer part.
     */
    if (
        strcasecmp(name, "int") == 0 &&
        arg_count == 1
    )
    {
        Value a =
            evaluate(args[0]);

        double x =
            a.number;

        value_free(&a);

        return value_number(
            (double)(long long)x
        );
    }

    /*
     * space(n)
     */
    if (
        strcasecmp(name, "space") == 0 &&
        arg_count == 1
    )
    {
        Value a =
            evaluate(args[0]);

        int count =
            (int)a.number;

        value_free(&a);

        if (count < 0)
            count = 0;

        if (count > 10000)
            count = 10000;

        char *spaces =
            malloc(
                (size_t)count + 1
            );

        memset(
            spaces,
            ' ',
            (size_t)count
        );

        spaces[count] = '\0';

        Value result =
            value_string(spaces);

        free(spaces);

        return result;
    }

    /*
     * User functions.
     */
    for (
        int i = 0;
        i < function_count;
        i++
    )
    {
        if (
            strcasecmp(
                functions[i].name,
                name
            ) != 0
        )
        {
            continue;
        }

        if (
            arg_count !=
            functions[i].param_count
        )
        {
            fprintf(
                stderr,
                "LOIS: function '%s' expects %d argument(s)\n",
                name,
                functions[i].param_count
            );

            return value_none();
        }

        /*
         * Save old parameter values.
         */
        Variable *saved =
            calloc(
                functions[i].param_count,
                sizeof(Variable)
            );

        int *had =
            calloc(
                functions[i].param_count,
                sizeof(int)
            );

        for (
            int j = 0;
            j < functions[i].param_count;
            j++
        )
        {
            Variable *old =
                find_variable(
                    functions[i].params[j]
                );

            if (old)
            {
                had[j] = 1;

                saved[j].name =
                    strdup(old->name);

                saved[j].value =
                    value_copy(
                        &old->value
                    );

                saved[j].is_num =
                    old->is_num;

                saved[j].is_bool =
                    old->is_bool;
            }

            Value argument =
                evaluate(args[j]);

            set_variable(
                functions[i].params[j],
                argument,
                argument.type ==
                    VALUE_NUMBER,
                argument.type ==
                    VALUE_BOOLEAN
            );
        }

        Value result =
            evaluate(
                functions[i].body
            );

        /*
         * Restore old variables.
         */
        for (
            int j = 0;
            j < functions[i].param_count;
            j++
        )
        {
            if (had[j])
            {
                set_variable(
                    saved[j].name,
                    saved[j].value,
                    saved[j].is_num,
                    saved[j].is_bool
                );

                free(saved[j].name);
            }
            else
            {
                Variable *v =
                    find_variable(
                        functions[i].params[j]
                    );

                if (v)
                {
                    value_free(
                        &v->value
                    );

                    free(v->name);

                    *v =
                        variables[
                            --variable_count
                        ];
                }
            }
        }

        free(saved);

        free(had);

        return result;
    }

    fprintf(
        stderr,
        "LOIS: unknown function '%s'\n",
        name
    );

    return value_none();
}

static Value evaluate(
    Expr *expr
)
{
    if (!expr)
        return value_none();

    if (
        expr->type ==
        EXPR_LITERAL
    )
    {
        return value_string(
            expr->text
        );
    }

    if (
        expr->type ==
        EXPR_NUMBER
    )
    {
        return value_number(
            expr->number
        );
    }

    if (
        expr->type ==
        EXPR_BOOLEAN
    )
    {
        return value_boolean(
            expr->boolean
        );
    }

    if (
        expr->type ==
        EXPR_VARIABLE
    )
    {
        Variable *variable =
            find_variable(
                expr->text
            );

        if (variable)
        {
            return value_copy(
                &variable->value
            );
        }

        /*
         * pi is the one built-in
         * constant.
         */
        if (
            strcasecmp(
                expr->text,
                "pi"
            ) == 0
        )
        {
            return value_number(
                M_PI
            );
        }

        /*
         * Unknown words in output
         * become literal text.
         */
        return value_string(
            expr->text
        );
    }

    if (
        expr->type ==
        EXPR_CALL
    )
    {
        return call_function(
            expr->text,
            expr->args,
            expr->arg_count
        );
    }

    if (
        expr->type ==
        EXPR_UNARY
    )
    {
        Value a =
            evaluate(expr->left);

        Value result =
            value_none();

        if (
            expr->operator ==
            TOKEN_MINUS &&
            a.type ==
                VALUE_NUMBER
        )
        {
            result =
                value_number(
                    -a.number
                );
        }
        else if (
            expr->operator ==
            TOKEN_BANG
        )
        {
            result =
                value_boolean(
                    !truth(&a)
                );
        }

        value_free(&a);

        return result;
    }

    if (
        expr->type ==
        EXPR_BINARY
    )
    {
        Value left =
            evaluate(expr->left);

        Value right =
            evaluate(expr->right);

        Value result =
            value_none();

        /*
         * String output concatenation.
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
            char combined[8192];

            value_to_text(
                &left,
                left_text,
                sizeof(left_text)
            );

            value_to_text(
                &right,
                right_text,
                sizeof(right_text)
            );

            snprintf(
                combined,
                sizeof(combined),
                "%s %s",
                left_text,
                right_text
            );

            result =
                value_string(
                    combined
                );
        }

        /*
         * Numeric operations.
         */
        else if (
            left.type ==
                VALUE_NUMBER &&
            right.type ==
                VALUE_NUMBER
        )
        {
            switch (
                expr->operator
            )
            {
                case TOKEN_PLUS:
                    result =
                        value_number(
                            left.number +
                            right.number
                        );
                    break;

                case TOKEN_MINUS:
                    result =
                        value_number(
                            left.number -
                            right.number
                        );
                    break;

                case TOKEN_STAR:
                    result =
                        value_number(
                            left.number *
                            right.number
                        );
                    break;

                case TOKEN_SLASH:
                    if (
                        right.number == 0
                    )
                    {
                        fprintf(
                            stderr,
                            "LOIS: division by zero\n"
                        );
                    }
                    else
                    {
                        result =
                            value_number(
                                left.number /
                                right.number
                            );
                    }

                    break;

                case TOKEN_PERCENT:
                    if (
                        right.number == 0
                    )
                    {
                        fprintf(
                            stderr,
                            "LOIS: modulo by zero\n"
                        );
                    }
                    else
                    {
                        result =
                            value_number(
                                fmod(
                                    left.number,
                                    right.number
                                )
                            );
                    }

                    break;

                case TOKEN_CARET:
                    result =
                        value_number(
                            pow(
                                left.number,
                                right.number
                            )
                        );
                    break;

                case TOKEN_GREATER:
                    result =
                        value_boolean(
                            left.number >
                            right.number
                        );
                    break;

                case TOKEN_LESS:
                    result =
                        value_boolean(
                            left.number <
                            right.number
                        );
                    break;

                case TOKEN_GREATER_EQUAL:
                    result =
                        value_boolean(
                            left.number >=
                            right.number
                        );
                    break;

                case TOKEN_LESS_EQUAL:
                    result =
                        value_boolean(
                            left.number <=
                            right.number
                        );
                    break;

                case TOKEN_EQUAL_EQUAL:
                    result =
                        value_boolean(
                            left.number ==
                            right.number
                        );
                    break;

                case TOKEN_NOT_EQUAL:
                    result =
                        value_boolean(
                            left.number !=
                            right.number
                        );
                    break;

                case TOKEN_AND_AND:
                    result =
                        value_boolean(
                            truth(&left) &&
                            truth(&right)
                        );
                    break;

                case TOKEN_OR_OR:
                    result =
                        value_boolean(
                            truth(&left) ||
                            truth(&right)
                        );
                    break;

                default:
                    break;
            }
        }

        /*
         * String equality.
         */
        else if (
            left.type ==
                VALUE_STRING &&
            right.type ==
                VALUE_STRING
        )
        {
            int equal =
                strcmp(
                    left.string,
                    right.string
                ) == 0;

            if (
                expr->operator ==
                TOKEN_EQUAL_EQUAL
            )
            {
                result =
                    value_boolean(
                        equal
                    );
            }
            else if (
                expr->operator ==
                TOKEN_NOT_EQUAL
            )
            {
                result =
                    value_boolean(
                        !equal
                    );
            }
        }

        /*
         * Boolean logic.
         */
        else if (
            expr->operator ==
                TOKEN_AND_AND ||
            expr->operator ==
                TOKEN_OR_OR
        )
        {
            result =
                value_boolean(
                    expr->operator ==
                        TOKEN_AND_AND
                        ? (
                            truth(&left) &&
                            truth(&right)
                          )
                        : (
                            truth(&left) ||
                            truth(&right)
                          )
                );
        }

        value_free(&left);

        value_free(&right);

        return result;
    }

    return value_none();
}

static void print_value(
    Value *value,
    int precision
)
{
    if (
        value->type ==
        VALUE_STRING
    )
    {
        printf(
            "%s",
            value->string
        );
    }
    else if (
        value->type ==
        VALUE_BOOLEAN
    )
    {
        printf(
            "%s",
            value->boolean
                ? "true"
                : "false"
        );
    }
    else if (
        value->type ==
        VALUE_NUMBER
    )
    {
        if (precision >= 0)
        {
            printf(
                "%.*f",
                precision,
                value->number
            );
        }
        else if (
            value->number ==
            (long long)value->number
        )
        {
            printf(
                "%lld",
                (long long)
                    value->number
            );
        }
        else
        {
            printf(
                "%.12g",
                value->number
            );
        }
    }
}

static void execute(
    Statement *statement
);

static void execute_output(
    Statement *statement
)
{
    Value value =
        evaluate(
            statement->expression
        );

    print_value(
        &value,
        statement->precision > 0
            ? statement->precision
            : -1
    );

    value_free(&value);

    putchar('\n');
}

static void execute(
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
                        1,
                        0
                    );

                    break;
                }

                /*
                 * name is string
                 */
                if (
                    statement->extra &&
                    strcasecmp(
                        statement->extra,
                        "string"
                    ) == 0
                )
                {
                    set_variable(
                        statement->name,
                        value_string(""),
                        0,
                        0
                    );

                    break;
                }

                /*
                 * ready is boolean
                 */
                if (
                    statement->extra &&
                    strcasecmp(
                        statement->extra,
                        "boolean"
                    ) == 0
                )
                {
                    set_variable(
                        statement->name,
                        value_boolean(0),
                        0,
                        1
                    );

                    break;
                }

                Value value =
                    evaluate(
                        statement->expression
                    );

                Variable *old =
                    find_variable(
                        statement->name
                    );

                int numeric =
                    statement->numeric_assignment ||
                    (
                        old &&
                        old->is_num
                    );

                /*
                 * is 13 normally means
                 * string.
                 *
                 * But if previously declared
                 * num, it remains numeric.
                 */
                if (
                    !numeric &&
                    value.type ==
                        VALUE_NUMBER
                )
                {
                    char text[128];

                    value_to_text(
                        &value,
                        text,
                        sizeof(text)
                    );

                    value_free(&value);

                    value =
                        value_string(text);
                }

                set_variable(
                    statement->name,
                    value,
                    numeric,
                    value.type ==
                        VALUE_BOOLEAN
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
                            1,
                            0
                        );
                    }
                    else
                    {
                        set_variable(
                            statement->name,
                            value_string(
                                buffer
                            ),
                            0,
                            0
                        );
                    }
                }

                break;
            }

            case STMT_OUTPUT:
                execute_output(
                    statement
                );
                break;

            case STMT_IF:
            {
                Value condition =
                    evaluate(
                        statement->condition
                    );

                int true_value =
                    truth(&condition);

                value_free(
                    &condition
                );

                if (true_value)
                {
                    execute(
                        statement->body
                    );
                }
                else if (
                    statement->else_body
                )
                {
                    execute(
                        statement->else_body
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

                    int true_value =
                        truth(&condition);

                    value_free(
                        &condition
                    );

                    if (!true_value)
                        break;

                    execute(
                        statement->body
                    );
                }

                if (guard >= 100000)
                {
                    fprintf(
                        stderr,
                        "LOIS: while loop limit reached\n"
                    );
                }

                break;
            }

            case STMT_FOR:
            {
                Value start =
                    evaluate(
                        statement->expression
                    );

                set_variable(
                    statement->name,
                    start,
                    1,
                    0
                );

                int guard = 0;

                while (guard++ < 100000)
                {
                    Value condition =
                        evaluate(
                            statement->condition
                        );

                    int true_value =
                        truth(&condition);

                    value_free(
                        &condition
                    );

                    if (!true_value)
                        break;

                    execute(
                        statement->body
                    );

                    Variable *variable =
                        find_variable(
                            statement->name
                        );

                    if (!variable)
                        break;

                    Value step =
                        evaluate(
                            statement->step
                        );

                    if (
                        step.type ==
                        VALUE_NUMBER
                    )
                    {
                        double next =
                            variable->value.number +
                            step.number;

                        value_free(
                            &step
                        );

                        set_variable(
                            statement->name,
                            value_number(next),
                            1,
                            0
                        );
                    }
                    else
                    {
                        value_free(
                            &step
                        );

                        break;
                    }
                }

                break;
            }

            case STMT_FUNCTION:
                /*
                 * Functions are registered before
                 * execution, so definitions can
                 * appear anywhere as complete lines.
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

    function_count = 0;

    /*
     * Register functions first.
     */
    for (
        Statement *s = program;
        s;
        s = s->next
    )
    {
        if (
            s->type ==
            STMT_FUNCTION
        )
        {
            if (
                function_count >=
                MAX_FUNCTIONS
            )
                break;

            functions[
                function_count
            ].name =
                strdup(s->name);

            functions[
                function_count
            ].params =
                s->params;

            functions[
                function_count
            ].param_count =
                s->param_count;

            functions[
                function_count
            ].body =
                s->expression;

            function_count++;
        }
    }

    execute(program);

    for (
        int i = 0;
        i < variable_count;
        i++
    )
    {
        free(
            variables[i].name
        );

        value_free(
            &variables[i].value
        );
    }

    for (
        int i = 0;
        i < function_count;
        i++
    )
    {
        free(
            functions[i].name
        );
    }

    variable_count = 0;

    function_count = 0;
}
