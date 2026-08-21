#ifndef LOIS_PARSER_H
#define LOIS_PARSER_H

#include "lexer.h"

typedef enum
{
    EXPR_LITERAL,
    EXPR_NUMBER,
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

    TokenType operator;

    Expr *left;
    Expr *right;

    Expr **arguments;
    int argument_count;
};

typedef enum
{
    STMT_ASSIGN,
    STMT_INPUT,
    STMT_OUTPUT,

    STMT_IF,
    STMT_ELSE_IF,
    STMT_ELSE,

    STMT_WHILE,
    STMT_FOR,

    STMT_FUNCTION,

    STMT_BREAK
} StatementType;

typedef struct Statement Statement;

struct Statement
{
    StatementType type;

    char *name;

    /*
     * Used for:
     *
     * num
     * upto
     * etc.
     */
    char *extra;

    Expr *expression;

    Expr *condition;

    /*
     * Function parameter names.
     */
    char **parameters;
    int parameter_count;

    /*
     * Function body / if body / loop body.
     */
    Statement *body;

    /*
     * Else / else-if chain.
     */
    Statement *next_branch;

    /*
     * Normal program linked list.
     */
    Statement *next;
};

Statement *parser_parse(TokenList *tokens);

void parser_free(Statement *statement);

#endif
