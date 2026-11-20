#ifndef LOIS_VALUE_H
#define LOIS_VALUE_H

typedef enum
{
    VALUE_STRING,
    VALUE_NUMBER,
    VALUE_BOOL,
    VALUE_SET,
    VALUE_NONE
} ValueType;

typedef struct Value Value;

typedef struct
{
    Value *items;
    int count;
    int capacity;
} ValueSet;

struct Value
{
    ValueType type;

    char *string;
    double number;
    int is_radians;

    ValueSet set;
};

Value value_string(const char *text);
int value_string_try(const char *text, Value *out);
Value value_number(double number);
Value value_bool(int boolean);
Value value_set(void);
Value value_none(void);

int value_set_add(Value *set, Value *item);
Value *value_set_get(Value *set, int index);

int value_copy_try(const Value *source, Value *out);
Value value_copy(const Value *value);
void value_free(Value *value);

#endif
