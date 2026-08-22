#define _GNU_SOURCE

#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static char *copy_text(
    const char *start,
    int length
)
{
    char *result =
        malloc((size_t)length + 1);

    if (!result)
    {
        fprintf(stderr,
                "LOIS: out of memory\n");
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
            list->capacity == 0
                ? 128
                : list->capacity * 2;

        list->tokens =
            realloc(
                list->tokens,
                sizeof(Token) * list->capacity
            );

        if (!list->tokens)
        {
            fprintf(stderr,
                    "LOIS: out of memory\n");
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
    {
        token->text =
            copy_text(text, length);
    }
    else
    {
        token->text = NULL;
    }
}

TokenList lexer_tokenize(
    const char *source
)
{
    TokenList list = {0};

    int line = 1;
    int column = 1;

    const char *p = source;

    while (*p)
    {
        char c = *p;

        /*
         * Normal whitespace outside strings
         * is insignificant.
         */
        if (
            c == ' ' ||
            c == '\t' ||
            c == '\r'
        )
        {
            p++;
            column++;
            continue;
        }

        /*
         * Newline matters to LOIS.
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
         * # comments are also accepted.
         */
        if (c == '#')
        {
            while (*p && *p != '\n')
            {
                p++;
                column++;
            }

            continue;
        }

        /*
         * note:
         *
         * Everything after note: on this
         * line is a comment.
         */
        if (isalpha((unsigned char)c))
        {
            const char *q = p;

            while (
                isalnum((unsigned char)*q) ||
                *q == '_'
            )
            {
                q++;
            }

            if (
                (q - p) == 4 &&
                strncasecmp(
                    p,
                    "note",
                    4
                ) == 0 &&
                *q == ':'
            )
            {
                while (*p && *p != '\n')
                {
                    p++;
                    column++;
                }

                continue;
            }
        }

        /*
         * Multiline comment:
         *
         * [
         *     ignored
         * ]
         */
        if (c == '[')
        {
            p++;
            column++;

            while (*p)
            {
                if (*p == ']')
                {
                    p++;
                    column++;
                    break;
                }

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

            continue;
        }

        /*
         * Strings.
         *
         * Newlines INSIDE quotes are preserved.
         */
        if (c == '"')
        {
            int start_line = line;
            int start_column = column;

            p++;
            column++;

            size_t capacity = 64;
            size_t length = 0;

            char *buffer =
                malloc(capacity);

            if (!buffer)
            {
                fprintf(stderr,
                        "LOIS: out of memory\n");
                exit(1);
            }

            while (
                *p &&
                *p != '"'
            )
            {
                char output = *p++;

                if (
                    output == '\\' &&
                    *p
                )
                {
                    char escaped = *p++;

                    if (escaped == 'n')
                        output = '\n';
                    else if (escaped == 't')
                        output = '\t';
                    else if (escaped == 'r')
                        output = '\r';
                    else
                        output = escaped;
                }

                if (
                    length + 1 >=
                    capacity
                )
                {
                    capacity *= 2;

                    buffer =
                        realloc(
                            buffer,
                            capacity
                        );
                }

                buffer[length++] =
                    output;

                if (output == '\n')
                {
                    line++;
                    column = 1;
                }
                else
                {
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

                free(buffer);

                break;
            }

            add_token(
                &list,
                TOKEN_STRING,
                buffer,
                (int)length,
                0,
                start_line,
                start_column
            );

            free(buffer);

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
                isdigit(
                    (unsigned char)p[1]
                )
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
         * Operators and punctuation.
         */
        TokenType type = TOKEN_EOF;

        int length = 1;

        if (c == '.')
            type = TOKEN_DOT;

        else if (c == ',')
            type = TOKEN_COMMA;

        else if (c == '(')
            type = TOKEN_LPAREN;

        else if (c == ')')
            type = TOKEN_RPAREN;

        else if (c == '+')
            type = TOKEN_PLUS;

        else if (c == '-')
            type = TOKEN_MINUS;

        else if (c == '*')
            type = TOKEN_STAR;

        else if (c == '/')
            type = TOKEN_SLASH;

        else if (c == '%')
            type = TOKEN_PERCENT;

        else if (c == '^')
            type = TOKEN_CARET;

        else if (c == '>')
        {
            if (p[1] == '=')
            {
                type =
                    TOKEN_GREATER_EQUAL;

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
                type =
                    TOKEN_LESS_EQUAL;

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
                type =
                    TOKEN_EQUAL_EQUAL;

                length = 2;
            }
            else
            {
                type = TOKEN_ASSIGN;
            }
        }

        else if (c == '!')
        {
            if (p[1] == '=')
            {
                type =
                    TOKEN_NOT_EQUAL;

                length = 2;
            }
            else
            {
                type = TOKEN_BANG;
            }
        }

        else if (
            c == '&' &&
            p[1] == '&'
        )
        {
            type = TOKEN_AND_AND;
            length = 2;
        }

        else if (
            c == '|' &&
            p[1] == '|'
        )
        {
            type = TOKEN_OR_OR;
            length = 2;
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

void lexer_free(
    TokenList *list
)
{
    if (!list)
        return;

    for (
        int i = 0;
        i < list->count;
        i++
    )
    {
        free(list->tokens[i].text);
    }

    free(list->tokens);

    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}
