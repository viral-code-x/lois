#define _GNU_SOURCE

#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *copy_text(const char *start, int length)
{
    char *result = malloc((size_t)length + 1);

    if (!result)
    {
        fprintf(stderr, "LOIS: out of memory\n");
        exit(1);
    }

    memcpy(result, start, (size_t)length);
    result[length] = '\0';

    return result;
}

static void add_token(
    TokenList *list,
    TokenType type,
    const char *text,
    int length,
    double number,
    int line,
    int column
)
{
    if (list->count >= list->capacity)
    {
        list->capacity =
            list->capacity == 0 ? 64 : list->capacity * 2;

        list->tokens =
            realloc(
                list->tokens,
                sizeof(Token) * list->capacity
            );

        if (!list->tokens)
        {
            fprintf(stderr, "LOIS: out of memory\n");
            exit(1);
        }
    }

    Token *token =
        &list->tokens[list->count++];

    token->type = type;
    token->number = number;
    token->line = line;
    token->column = column;

    if (text && length > 0)
        token->text = copy_text(text, length);
    else
        token->text = NULL;
}

TokenList lexer_tokenize(const char *source)
{
    TokenList list = {0};

    int line = 1;
    int column = 1;

    const char *p = source;

    while (*p)
    {
        char c = *p;

        /*
         * Outside strings, spaces are separators.
         */
        if (c == ' ' || c == '\t' || c == '\r')
        {
            p++;
            column++;
            continue;
        }

        /*
         * Newlines are meaningful.
         */
        if (c == '\n')
        {
            add_token(
                &list,
                TOKEN_NEWLINE,
                NULL,
                0,
                0,
                line,
                column
            );

            p++;
            line++;
            column = 1;
            continue;
        }

        /*
         * One-line comment:
         *
         * note: this is ignored
         */
        if (
            (c == 'n' || c == 'N') &&
            strncasecmp(p, "note:", 5) == 0
        )
        {
            while (*p && *p != '\n')
            {
                p++;
                column++;
            }

            continue;
        }

        /*
         * Multiline comment:
         *
         * [ this is ignored ]
         */
        if (c == '[')
        {
            p++;
            column++;

            while (*p)
            {
                if (*p == ']' )
                {
                    p++;
                    column++;
                    break;
                }

                if (*p == '\n')
                {
                    p++;
                    line++;
                    column = 1;
                }
                else
                {
                    p++;
                    column++;
                }
            }

            continue;
        }

        /*
         * Strings.
         *
         * Newlines inside quotes are preserved.
         */
        if (c == '"')
        {
            const char *start;
            int start_line = line;
            int start_column = column;

            p++;
            column++;

            start = p;

            while (*p && *p != '"')
            {
                if (*p == '\n')
                {
                    p++;
                    line++;
                    column = 1;
                }
                else
                {
                    p++;
                    column++;
                }
            }

            if (*p != '"')
            {
                fprintf(
                    stderr,
                    "LOIS: unterminated string at %d:%d\n",
                    start_line,
                    start_column
                );

                break;
            }

            add_token(
                &list,
                TOKEN_STRING,
                start,
                (int)(p - start),
                0,
                start_line,
                start_column
            );

            p++;
            column++;

            continue;
        }

        /*
         * Numbers.
         */
        if (
            isdigit((unsigned char)c) ||
            (
                c == '.' &&
                isdigit((unsigned char)p[1])
            )
        )
        {
            char *end;

            double number =
                strtod(p, &end);

            int length =
                (int)(end - p);

            add_token(
                &list,
                TOKEN_NUMBER,
                p,
                length,
                number,
                line,
                column
            );

            p = end;
            column += length;

            continue;
        }

        /*
         * Words.
         */
        if (
            isalpha((unsigned char)c) ||
            c == '_'
        )
        {
            const char *start = p;

            while (
                isalnum((unsigned char)*p) ||
                *p == '_'
            )
            {
                p++;
                column++;
            }

            int length = (int)(p - start);

            /*
             * Logical operators are keywords, not arithmetic
             * operators.
             */
            if (
                length == 3 &&
                strncasecmp(start, "not", 3) == 0
            )
            {
                add_token(
                    &list,
                    TOKEN_NOT,
                    start,
                    length,
                    0,
                    line,
                    column
                );
            }
            else if (
                length == 3 &&
                strncasecmp(start, "and", 3) == 0
            )
            {
                add_token(
                    &list,
                    TOKEN_AND,
                    start,
                    length,
                    0,
                    line,
                    column
                );
            }
            else if (
                length == 2 &&
                strncasecmp(start, "or", 2) == 0
            )
            {
                add_token(
                    &list,
                    TOKEN_OR,
                    start,
                    length,
                    0,
                    line,
                    column
                );
            }
            else
            {
                add_token(
                    &list,
                    TOKEN_WORD,
                    start,
                    length,
                    0,
                    line,
                    column
                );
            }

            continue;
        }

        TokenType type = TOKEN_EOF;
        int length = 1;

        switch (c)
        {
            case '+':
                type = TOKEN_PLUS;
                break;

            case '-':
                type = TOKEN_MINUS;
                break;

            case '*':
                type = TOKEN_STAR;
                break;

            case '/':
                type = TOKEN_SLASH;
                break;

            case '%':
                type = TOKEN_PERCENT;
                break;

            case '^':
                type = TOKEN_CARET;
                break;

            case '(':
                type = TOKEN_LPAREN;
                break;

            case ')':
                type = TOKEN_RPAREN;
                break;

            case '{':
                type = TOKEN_LBRACE;
                break;

            case '}':
                type = TOKEN_RBRACE;
                break;

            case ',':
                type = TOKEN_COMMA;
                break;

            case '|':
                type = TOKEN_ABS;
                break;

            case '=':
                if (p[1] == '=')
                {
                    type = TOKEN_EQUAL_EQUAL;
                    length = 2;
                }
                else
                {
                    type = TOKEN_ASSIGN;
                }
                break;

            case '>':
                if (p[1] == '=')
                {
                    type = TOKEN_GREATER_EQUAL;
                    length = 2;
                }
                else
                {
                    type = TOKEN_GREATER;
                }
                break;

            case '<':
                if (p[1] == '=')
                {
                    type = TOKEN_LESS_EQUAL;
                    length = 2;
                }
                else
                {
                    type = TOKEN_LESS;
                }
                break;

            case '!':
                if (p[1] == '=')
                {
                    type = TOKEN_NOT_EQUAL;
                    length = 2;
                }
                else
                {
                    fprintf(
                        stderr,
                        "LOIS: use 'not' for logical negation at %d:%d\n",
                        line,
                        column
                    );
                }
                break;

            default:
                break;
        }

        if (type != TOKEN_EOF)
        {
            add_token(
                &list,
                type,
                p,
                length,
                0,
                line,
                column
            );

            p += length;
            column += length;

            continue;
        }

        fprintf(
            stderr,
            "LOIS: unexpected character '%c' at %d:%d\n",
            c,
            line,
            column
        );

        p++;
        column++;
    }

    add_token(
        &list,
        TOKEN_EOF,
        NULL,
        0,
        0,
        line,
        column
    );

    return list;
}

void lexer_free(TokenList *list)
{
    if (!list)
        return;

    for (int i = 0; i < list->count; i++)
        free(list->tokens[i].text);

    free(list->tokens);

    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}
