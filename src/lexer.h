#ifndef LOIS_LEXER_H
#define LOIS_LEXER_H

typedef enum
{
    TOKEN_EOF,

    TOKEN_WORD,
    TOKEN_NUMBER,
    TOKEN_STRING,

    TOKEN_NEWLINE,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CARET,

    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_NOT,

    TOKEN_ASSIGN,

    TOKEN_AND,
    TOKEN_OR,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COMMA,
    TOKEN_ABS
} TokenType;

typedef struct
{
    TokenType type;
    char *text;
    double number;
    int line;
    int column;
} Token;

typedef struct
{
    Token *tokens;
    int count;
    int capacity;
} TokenList;

TokenList lexer_tokenize(const char *source);
void lexer_free(TokenList *list);

#endif
