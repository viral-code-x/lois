#ifndef LOIS_VALUE_H
#define LOIS_VALUE_H

typedef enum {
    VALUE_STRING,
    VALUE_NUMBER,
    VALUE_BOOL,
    VALUE_NONE
} ValueType;

typedef struct {
    ValueType type;
    char *string;
    double number;
} Value;

Value value_string(const char *text);
Value value_number(double number);
Value value_bool(int boolean);
Value value_none(void);

Value value_copy(const Value *value);
void value_free(Value *value);

#endif
