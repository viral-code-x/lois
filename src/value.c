#include "value.h"

#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

int value_string_try(const char *text, Value *out)
{
    if (!out)
        return 0;

    *out = value_none();

    if (!text)
        return 0;

    size_t length = strlen(text);
    char *copy = malloc(length + 1);

    if (!copy)
        return 0;

    memcpy(copy, text, length + 1);

    out->type = VALUE_STRING;
    out->string = copy;
    return 1;
}

Value value_string(const char *text)
{
    Value value = value_none();
    value_string_try(text, &value);
    return value;
}

Value value_number(double number)
{
    Value value;

    value.type = VALUE_NUMBER;
    value.number = number;
    value.is_radians = 0;
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
    value.is_radians = 0;
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
    value.is_radians = 0;

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
    value.is_radians = 0;

    value.set.items = NULL;
    value.set.count = 0;
    value.set.capacity = 0;

    return value;
}

int value_set_add(Value *set, Value *item)
{
    if (!set || set->type != VALUE_SET || !item)
        return 0;

    if (set->set.count < 0 ||
        set->set.capacity < 0 ||
        set->set.capacity < set->set.count ||
        (set->set.capacity == 0 && set->set.items != NULL) ||
        (set->set.capacity > 0 && set->set.items == NULL))
        return 0;

    if (set->set.count == INT_MAX)
        return 0;

    if (set->set.count >= set->set.capacity)
    {
        int new_capacity;

        if (set->set.capacity == 0)
            new_capacity = 4;
        else if (set->set.capacity > INT_MAX / 2)
            new_capacity = INT_MAX;
        else
            new_capacity = set->set.capacity * 2;

        if ((size_t)new_capacity > SIZE_MAX / sizeof(Value))
            return 0;

        Value *new_items = realloc(
            set->set.items,
            sizeof(Value) * (size_t)new_capacity
        );

        if (!new_items)
            return 0;

        set->set.items = new_items;
        set->set.capacity = new_capacity;
    }

    set->set.items[set->set.count++] = *item;
    *item = value_none();

    return 1;
}

Value *value_set_get(Value *set, int index)
{
    if (!set || set->type != VALUE_SET)
        return NULL;

    if (set->set.count < 0 ||
        set->set.capacity < 0 ||
        set->set.capacity < set->set.count ||
        (set->set.capacity == 0 && set->set.items != NULL) ||
        (set->set.capacity > 0 && set->set.items == NULL))
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

int value_copy_try(const Value *source, Value *out)
{
    if (!source || !out || source == out)
        return 0;

    *out = value_none();

    switch (source->type)
    {
        case VALUE_NONE:
            *out = value_none();
            return 1;

        case VALUE_NUMBER:
            *out = value_number(source->number);
            out->is_radians = source->is_radians;
            return 1;

        case VALUE_BOOL:
            *out = value_bool((int)source->number);
            return 1;

        case VALUE_STRING:
            return value_string_try(source->string, out);

        case VALUE_SET:
        {
            if (source->set.count < 0 ||
                source->set.capacity < 0 ||
                source->set.capacity < source->set.count ||
                (source->set.capacity == 0 && source->set.items != NULL) ||
                (source->set.capacity > 0 && source->set.items == NULL))
                return 0;

            Value copy = value_set();

            for (int i = 0; i < source->set.count; i++)
            {
                Value item = value_none();

                if (!value_copy_try(&source->set.items[i], &item))
                {
                    value_free(&copy);
                    return 0;
                }

                if (!value_set_add(&copy, &item))
                {
                    value_free(&item);
                    value_free(&copy);
                    return 0;
                }
            }

            *out = copy;
            return 1;
        }

        default:
            return 0;
    }
}

Value value_copy(const Value *value)
{
    Value copy = value_none();

    if (!value_copy_try(value, &copy))
        return value_none();

    return copy;
}

void value_free(Value *value)
{
    if (!value)
        return;

    if (value->type == VALUE_STRING)
    {
        free(value->string);
    }
    else if (value->type == VALUE_SET)
    {
        int count = value->set.count;
        int capacity = value->set.capacity;

        if (count < 0)
            count = 0;

        if (capacity < 0)
            capacity = 0;

        if (count > capacity)
            count = capacity;

        if (value->set.items)
        {
            for (int i = 0; i < count; i++)
                value_free(&value->set.items[i]);
        }

        free(value->set.items);
    }

    *value = value_none();
}
