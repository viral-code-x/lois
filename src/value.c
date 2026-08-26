#include "value.h"

#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *text)
{
    size_t length = strlen(text);

    char *result = malloc(length + 1);

    if (!result)
        exit(1);

    memcpy(result, text, length + 1);

    return result;
}

Value value_string(const char *text)
{
    Value value;

    value.type = VALUE_STRING;
    value.string = copy_string(text);
    value.number = 0;
    value.set.items = NULL;
    value.set.count = 0;
    value.set.capacity = 0;

    return value;
}

Value value_number(double number)
{
    Value value;

    value.type = VALUE_NUMBER;
    value.number = number;
    value.string = NULL;
    value.set.items = NULL;
    value.set.count = 0;
    value.set.capacity = 0;

    return value;
}

Value value_bool(int boolean)
{
    Value value;

    value.type = VALUE_BOOL;
    value.number = boolean ? 1 : 0;
    value.string = NULL;
    value.set.items = NULL;
    value.set.count = 0;
    value.set.capacity = 0;

    return value;
}

Value value_set(void)
{
    Value value;

    value.type = VALUE_SET;
    value.string = NULL;
    value.number = 0;

    value.set.items = NULL;
    value.set.count = 0;
    value.set.capacity = 0;

    return value;
}

Value value_none(void)
{
    Value value;

    value.type = VALUE_NONE;
    value.string = NULL;
    value.number = 0;

    value.set.items = NULL;
    value.set.count = 0;
    value.set.capacity = 0;

    return value;
}

void value_set_add(Value *set, Value item)
{
    if (!set || set->type != VALUE_SET)
        return;

    if (set->set.count >= set->set.capacity)
    {
        int new_capacity =
            set->set.capacity == 0
                ? 4
                : set->set.capacity * 2;

        Value *new_items =
            realloc(
                set->set.items,
                sizeof(Value) * new_capacity
            );

        if (!new_items)
            exit(1);

        set->set.items = new_items;
        set->set.capacity = new_capacity;
    }

    set->set.items[
        set->set.count++
    ] = item;
}

Value *value_set_get(Value *set, int index)
{
    if (!set || set->type != VALUE_SET)
        return NULL;

    /*
     * LOIS sets are 1-based:
     *
     * numbers1 -> first element
     * numbers2 -> second element
     */
    if (index < 1 || index > set->set.count)
        return NULL;

    return &set->set.items[index - 1];
}

Value value_copy(const Value *value)
{
    if (!value)
        return value_none();

    if (value->type == VALUE_STRING)
        return value_string(value->string);

    if (value->type == VALUE_NUMBER)
        return value_number(value->number);

    if (value->type == VALUE_BOOL)
        return value_bool((int)value->number);

    if (value->type == VALUE_SET)
    {
        Value copy = value_set();

        for (int i = 0; i < value->set.count; i++)
        {
            value_set_add(
                &copy,
                value_copy(
                    &value->set.items[i]
                )
            );
        }

        return copy;
    }

    return value_none();
}

void value_free(Value *value)
{
    if (!value)
        return;

    if (value->string)
        free(value->string);

    value->string = NULL;

    if (value->type == VALUE_SET)
    {
        for (int i = 0; i < value->set.count; i++)
            value_free(
                &value->set.items[i]
            );

        free(value->set.items);

        value->set.items = NULL;
        value->set.count = 0;
        value->set.capacity = 0;
    }
}
