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

    ValueSet set;
};

Value value_string(const char *text);
Value value_number(double number);
Value value_bool(int boolean);
Value value_set(void);
Value value_none(void);

void value_set_add(Value *set, Value item);
Value *value_set_get(Value *set, int index);

Value value_copy(const Value *value);
void value_free(Value *value);

#endif
