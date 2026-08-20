#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *copy_text(const char *start, int length)
{
    char *result = malloc(length + 1);

    if (!result)
    {
        fprintf(stderr, "LOIS: out of memory\n");
        exit(1);
    }

    memcpy(result, start, length);
    result[length] = '\0';

    return result;
}

static void add_token(
    TokenList *list,
    TokenType type,
    const char *text,
    double number
)
{
    if (list->count >= list->capacity)
    {
        list->capacity =
            list->capacity == 0
            ? 32
            : list->capacity * 2;

        list->tokens = realloc(
            list->tokens,
            sizeof(Token) * list->capacity
        );

        if (!list->tokens)
        {
            fprintf(stderr, "LOIS: out of memory\n");
            exit(1);
        }
    }

    list->tokens[list->count].type = type;

    if (text)
        list->tokens[list->count].text = strdup(text);
    else
        list->tokens[list->count].text = NULL;

    list->tokens[list->count].number = number;

    list->count++;
}

TokenList lexer_tokenize(const char *source)
{
    TokenList list = {0};

    int i = 0;

    while (source[i] != '\0')
    {
        char c = source[i];

        /* Ignore whitespace */
        if (isspace((unsigned char)c))
        {
            i++;
            continue;
        }

        /* Comments start with # */
        if (c == '#')
        {
            while (
                source[i] != '\0' &&
                source[i] != '\n'
            )
            {
                i++;
            }

            continue;
        }

        /* Full stop */
        if (c == '.')
        {
            /*
             * If the dot is followed by a number,
             * it may be part of a decimal number.
             */
            if (isdigit((unsigned char)source[i + 1]))
            {
                int start = i;

                i++;

                while (
                    isdigit((unsigned char)source[i])
                )
                {
                    i++;
                }

                char *text = copy_text(
                    source + start,
                    i - start
                );

                add_token(
                    &list,
                    TOKEN_NUMBER,
                    text,
                    atof(text)
                );

                free(text);

                continue;
            }

            add_token(
                &list,
                TOKEN_DOT,
                ".",
                0
            );

            i++;

            continue;
        }

        /* Arithmetic operators */
        if (c == '+')
        {
            add_token(&list, TOKEN_PLUS, "+", 0);
            i++;
            continue;
        }

        if (c == '-')
        {
            add_token(&list, TOKEN_MINUS, "-", 0);
            i++;
            continue;
        }

        if (c == '*')
        {
            add_token(&list, TOKEN_STAR, "*", 0);
            i++;
            continue;
        }

        if (c == '/')
        {
            add_token(&list, TOKEN_SLASH, "/", 0);
            i++;
            continue;
        }

        /* Greater than */
        if (c == '>')
        {
            if (source[i + 1] == '=')
            {
                add_token(
                    &list,
                    TOKEN_GREATER_EQUAL,
                    ">=",
                    0
                );

                i += 2;
            }
            else
            {
                add_token(
                    &list,
                    TOKEN_GREATER,
                    ">",
                    0
                );

                i++;
            }

            continue;
        }

        /* Less than */
        if (c == '<')
        {
            if (source[i + 1] == '=')
            {
                add_token(
                    &list,
                    TOKEN_LESS_EQUAL,
                    "<=",
                    0
                );

                i += 2;
            }
            else
            {
                add_token(
                    &list,
                    TOKEN_LESS,
                    "<",
                    0
                );

                i++;
            }

            continue;
        }

        /* Equal */
        if (c == '=')
        {
            if (source[i + 1] == '=')
            {
                add_token(
                    &list,
                    TOKEN_EQUAL_EQUAL,
                    "==",
                    0
                );

                i += 2;
            }
            else
            {
                fprintf(
                    stderr,
                    "LOIS: unexpected '='\n"
                );

                i++;
            }

            continue;
        }

        /* Not equal */
        if (c == '!')
        {
            if (source[i + 1] == '=')
            {
                add_token(
                    &list,
                    TOKEN_NOT_EQUAL,
                    "!=",
                    0
                );

                i += 2;
            }
            else
            {
                fprintf(
                    stderr,
                    "LOIS: unexpected '!'\n"
                );

                i++;
            }

            continue;
        }

        /* Quoted literal string */
        if (c == '"')
        {
            i++;

            int start = i;

            while (
                source[i] != '\0' &&
                source[i] != '"'
            )
            {
                i++;
            }

            char *text = copy_text(
                source + start,
                i - start
            );

            add_token(
                &list,
                TOKEN_STRING,
                text,
                0
            );

            free(text);

            if (source[i] == '"')
                i++;

            continue;
        }

        /* Number */
        if (isdigit((unsigned char)c))
        {
            int start = i;
            int has_dot = 0;

            while (
                isdigit((unsigned char)source[i]) ||
                (
                    source[i] == '.' &&
                    !has_dot
                )
            )
            {
                if (source[i] == '.')
                    has_dot = 1;

                i++;
            }

            char *text = copy_text(
                source + start,
                i - start
            );

            add_token(
                &list,
                TOKEN_NUMBER,
                text,
                atof(text)
            );

            free(text);

            continue;
        }

        /* Words / identifiers */
        if (
            isalpha((unsigned char)c) ||
            c == '_'
        )
        {
            int start = i;

            while (
                isalnum((unsigned char)source[i]) ||
                source[i] == '_'
            )
            {
                i++;
            }

            char *text = copy_text(
                source + start,
                i - start
            );

            add_token(
                &list,
                TOKEN_WORD,
                text,
                0
            );

            free(text);

            continue;
        }

        fprintf(
            stderr,
            "LOIS lexer: unknown character '%c'\n",
            c
        );

        i++;
    }

    add_token(
        &list,
        TOKEN_EOF,
        NULL,
        0
    );

    return list;
}

void lexer_free(TokenList *list)
{
    if (!list)
        return;

    for (int i = 0; i < list->count; i++)
    {
        free(list->tokens[i].text);
    }

    free(list->tokens);

    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}
