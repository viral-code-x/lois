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
    STMT_IF
} StatementType;

typedef struct Statement Statement;

struct Statement
{
    StatementType type;

    char *name;
    char *extra;

    Expr *expression;
    Expr *condition;

    Statement *body;
    Statement *else_body;

    Statement *next;
};

Statement *parser_parse(TokenList *tokens);

void parser_free(Statement *statement);

#endif
