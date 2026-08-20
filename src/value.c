#include "value.h"

#include <stdlib.h>
#include <string.h>

Value value_string(const char *text)
{
    Value value;

    value.type = VALUE_STRING;
    value.number = 0;
    value.string = strdup(text);

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
    value.number = 0;
    value.string = NULL;

    return value;
}

void value_free(Value *value)
{
    if (value->string != NULL)
    {
        free(value->string);
        value->string = NULL;
    }

    value->type = VALUE_NONE;
    value->number = 0;
}
