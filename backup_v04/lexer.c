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

    Token *token = &list->tokens[list->count++];

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
         * Spaces and tabs are separators.
         * They are NOT tokens.
         */
        if (c == ' ' || c == '\t' || c == '\r')
        {
            p++;
            column++;
            continue;
        }

        /*
         * Newline is meaningful.
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
         * String literal.
         */
        if (c == '"')
        {
            const char *start = ++p;
            int start_column = column++;

            while (*p && *p != '"')
            {
                if (*p == '\n')
                {
                    line++;
                    column = 1;
                }
                else
                {
                    column++;
                }

                p++;
            }

            if (*p != '"')
            {
                fprintf(
                    stderr,
                    "LOIS: unterminated string at %d:%d\n",
                    line,
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
                line,
                start_column
            );

            p++;
            column++;

            continue;
        }

        /*
         * Number.
         */
        if (isdigit((unsigned char)c) ||
            (c == '.' && isdigit((unsigned char)p[1])))
        {
            char *end;

            double number = strtod(p, &end);

            int length = (int)(end - p);

            add_token(
                &list,
                TOKEN_NUMBER,
                p,
                length,
                number,
                line,
                column
            );

            column += length;
            p = end;

            continue;
        }

        /*
         * Words.
         */
        if (isalpha((unsigned char)c) || c == '_')
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

            add_token(
                &list,
                TOKEN_WORD,
                start,
                (int)(p - start),
                0,
                line,
                column
            );

            continue;
        }

        /*
         * Operators.
         */
        TokenType type = TOKEN_EOF;
        int length = 1;

        if (c == '+')
            type = TOKEN_PLUS;
        else if (c == '-')
            type = TOKEN_MINUS;
        else if (c == '*')
            type = TOKEN_STAR;
        else if (c == '/')
            type = TOKEN_SLASH;
        else if (c == '>')
        {
            if (p[1] == '=')
            {
                type = TOKEN_GREATER_EQUAL;
                length = 2;
            }
            else
            {
                type = TOKEN_GREATER;
            }
        }
        else if (c == '<')
        {
            if (p[1] == '=')
            {
                type = TOKEN_LESS_EQUAL;
                length = 2;
            }
            else
            {
                type = TOKEN_LESS;
            }
        }
        else if (c == '=')
        {
            if (p[1] == '=')
            {
                type = TOKEN_EQUAL_EQUAL;
                length = 2;
            }
        }
        else if (c == '!')
        {
            if (p[1] == '=')
            {
                type = TOKEN_NOT_EQUAL;
                length = 2;
            }
        }
        else if (c == '.')
        {
            type = TOKEN_DOT;
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
