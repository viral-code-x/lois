#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *filename)
{
    FILE *file =
        fopen(filename, "rb");

    if (!file)
    {
        perror(filename);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return NULL;
    }

    long size =
        ftell(file);

    if (size < 0)
    {
        fclose(file);
        return NULL;
    }

    rewind(file);

    char *buffer =
        malloc((size_t)size + 1);

    if (!buffer)
    {
        fclose(file);

        fprintf(
            stderr,
            "LOIS: out of memory\n"
        );

        return NULL;
    }

    size_t read =
        fread(
            buffer,
            1,
            (size_t)size,
            file
        );

    buffer[read] = '\0';

    fclose(file);

    return buffer;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("LOIS 0.2\n");
        printf("Language Operated on IS\n");
        printf("\n");
        printf("Usage:\n");
        printf("  lois <file.is>\n");

        return 0;
    }

    char *source =
        read_file(argv[1]);

    if (!source)
        return 1;

    TokenList tokens =
        lexer_tokenize(source);

    Statement *program =
        parser_parse(&tokens);

    interpreter_run(program);

    parser_free(program);

    lexer_free(&tokens);

    free(source);

    return 0;
}
