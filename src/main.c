#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "output.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (!file)
    {
        perror(filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);

    long size = ftell(file);

    rewind(file);

    char *buffer = malloc(size + 1);

    if (!buffer)
    {
        fclose(file);
        fprintf(stderr, "LOIS: out of memory\n");
        return NULL;
    }

    size_t read =
        fread(buffer, 1, size, file);

    buffer[read] = '\0';

    fclose(file);

    return buffer;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("LOIS 0.1\n");
        printf("Language Operated on IS\n");
        printf("\n");
        printf("Usage:\n");
        printf("  lois <file.is>\n");
        printf("\n");
        printf("Example:\n");
        printf("  lois examples/hello.is\n");

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

    if (parser_had_error())
    {
        parser_free(program);
        lexer_free(&tokens);
        free(source);
        return 1;
    }

    interpreter_run(program);

    printf("%s", lois_output_get());

    if (interpreter_had_error())
        fprintf(stderr, "%s", lois_error_get());

    int had_error =
        interpreter_had_error();

    parser_free(program);

    lexer_free(&tokens);

    free(source);

    return had_error ? 1 : 0;
}
