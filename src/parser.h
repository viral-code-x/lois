#ifndef LOIS_PARSER_H
#define LOIS_PARSER_H

#include "lexer.h"

typedef enum
{
    EXPR_LITERAL,
    EXPR_NUMBER,
    EXPR_BOOLEAN,
    EXPR_VARIABLE,
    EXPR_BINARY,
    EXPR_UNARY
} ExprType;

typedef struct Expr Expr;

struct Expr
{
    ExprType type;

    char *text;
    double number;

    TokenType operator;

    Expr *left;
    Expr *right;
};

typedef enum
{
    STMT_ASSIGN,
    STMT_INPUT,
    STMT_OUTPUT,
    STMT_IF,
    STMT_WHILE,
    STMT_REPEAT,
    STMT_FUNCTION,
    STMT_FUNCTION_CALL,
    STMT_RETURN
} StatementType;

typedef struct Statement Statement;

struct Statement
{
    StatementType type;

    char *name;
    char *extra;

    char *parameters[16];
    int parameter_count;

    Expr *arguments[16];
    int argument_count;

    Expr *expression;
    Expr *condition;
    Expr *count;

    Statement *body;
    Statement *else_body;

    Statement *next;
};

Statement *parser_parse(TokenList *tokens);

void parser_free(Statement *statement);

#endif
