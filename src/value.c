#include "value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *text)
{
    if (!text)
        text = "";

    size_t length =
        strlen(text);

    char *result =
        malloc(length + 1);

    if (!result)
    {
        fprintf(
            stderr,
            "LOIS: out of memory\n"
        );

        exit(1);
    }

    memcpy(
        result,
        text,
        length + 1
    );

    return result;
}

Value value_string(const char *text)
{
    Value value;

    value.type = VALUE_STRING;
    value.string = copy_string(text);
    value.number = 0;

    return value;
}

Value value_number(double number)
{
    Value value;

    value.type = VALUE_NUMBER;
    value.number = number;
    value.string = NULL;

    return value;
}

Value value_none(void)
{
    Value value;

    value.type = VALUE_NONE;
    value.string = NULL;
    value.number = 0;

    return value;
}

Value value_copy(const Value *value)
{
    if (!value)
        return value_none();

    if (value->type == VALUE_STRING)
        return value_string(value->string);

    if (value->type == VALUE_NUMBER)
        return value_number(value->number);

    return value_none();
}

void value_free(Value *value)
{
    if (!value)
        return;

    if (value->string)
        free(value->string);

    value->string = NULL;
    value->number = 0;
    value->type = VALUE_NONE;
}
