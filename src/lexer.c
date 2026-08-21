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
        token->text =
            copy_text(text, length);
    else
        token->text = NULL;
}

static int is_word_start(char c)
{
    return
        isalpha((unsigned char)c) ||
        c == '_';
}

static int is_word_char(char c)
{
    return
        isalnum((unsigned char)c) ||
        c == '_';
}

TokenList lexer_tokenize(const char *source)
{
    TokenList list = {0};

    const char *p = source;

    int line = 1;
    int column = 1;

    int block_comment = 0;

    while (*p)
    {
        char c = *p;

        /*
         * Multiline comment:
         *
         * [
         *     comment
         * ]
         */
        if (!block_comment &&
            c == '[')
        {
            block_comment = 1;

            p++;
            column++;

            continue;
        }

        if (block_comment)
        {
            if (c == ']')
            {
                block_comment = 0;

                p++;
                column++;

                continue;
            }

            if (c == '\n')
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

            continue;
        }

        /*
         * One-line comment:
         *
         * note: hello world
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
         * Spaces outside strings are separators.
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
         * Strings.
         *
         * Everything inside quotes is preserved,
         * including spaces and newlines.
         */
        if (c == '"')
        {
            int start_line = line;
            int start_column = column;

            p++;
            column++;

            const char *start = p;

            while (*p && *p != '"')
            {
                if (*p == '\n')
                {
                    line++;
                    column = 1;
                    p++;
                }
                else
                {
                    column++;
                    p++;
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
         * Number.
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
        if (is_word_start(c))
        {
            const char *start = p;

            while (is_word_char(*p))
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
         * Parentheses.
         */
        if (c == '(')
        {
            add_token(
                &list,
                TOKEN_LPAREN,
                p,
                1,
                0,
                line,
                column
            );

            p++;
            column++;

            continue;
        }

        if (c == ')')
        {
            add_token(
                &list,
                TOKEN_RPAREN,
                p,
                1,
                0,
                line,
                column
            );

            p++;
            column++;

            continue;
        }

        /*
         * Comma.
         */
        if (c == ',')
        {
            add_token(
                &list,
                TOKEN_COMMA,
                p,
                1,
                0,
                line,
                column
            );

            p++;
            column++;

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

        else if (c == '%')
            type = TOKEN_PERCENT;

        else if (c == '^')
            type = TOKEN_POWER;

        else if (c == '=')
        {
            if (p[1] == '=')
            {
                type = TOKEN_EQUAL_EQUAL;
                length = 2;
            }
            else
            {
                type = TOKEN_EQUAL;
            }
        }

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

        else if (c == '!')
        {
            if (p[1] == '=')
            {
                type = TOKEN_NOT_EQUAL;
                length = 2;
            }
            else
            {
                type = TOKEN_BANG;
            }
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

    if (block_comment)
    {
        fprintf(
            stderr,
            "LOIS: unterminated multiline comment\n"
        );
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
