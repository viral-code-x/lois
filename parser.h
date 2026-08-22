#ifndef LOIS_PARSER_H
#define LOIS_PARSER_H

#include "lexer.h"

typedef enum {
    EXPR_LITERAL,
    EXPR_NUMBER,
    EXPR_BOOLEAN,
    EXPR_VARIABLE,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL
} ExprType;

typedef struct Expr Expr;

struct Expr
{
    ExprType type;

    char *text;

    double number;

    int boolean;

    TokenType operator;

    Expr *left;

    Expr *right;

    Expr **args;

    int arg_count;
};

typedef enum {
    STMT_ASSIGN,
    STMT_INPUT,
    STMT_OUTPUT,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_FUNCTION
} StatementType;

typedef struct Statement Statement;

struct Statement
{
    StatementType type;

    char *name;

    char *extra;

    int numeric_assignment;

    int precision;

    char **params;

    int param_count;

    Expr *expression;

    Expr *condition;

    Expr *step;

    Statement *body;

    Statement *else_body;

    Statement *next;
};

Statement *parser_parse(
    TokenList *tokens
);

void parser_free(
    Statement *statement
);

#endif
