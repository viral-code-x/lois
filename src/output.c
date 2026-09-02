#include "output.h"

#include <string.h>

#define OUTPUT_SIZE (1024 * 1024)

static char output_buffer[OUTPUT_SIZE];
static size_t output_length = 0;

static char error_buffer[OUTPUT_SIZE];
static size_t error_length = 0;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, lois_web_write, (const char *text), {
    if (
        typeof window.loisTerminalWrite === "function" &&
        text
    ) {
        window.loisTerminalWrite(
            UTF8ToString(text)
        );
    }
});
#endif

void lois_output_reset(void)
{
    output_length = 0;
    output_buffer[0] = '\0';
}

void lois_output_write(const char *text)
{
    if (!text)
        return;

    size_t length = strlen(text);

    if (output_length + length >= OUTPUT_SIZE)
        return;

    memcpy(
        output_buffer + output_length,
        text,
        length
    );

    output_length += length;
    output_buffer[output_length] = '\0';

#ifdef __EMSCRIPTEN__
    lois_web_write(text);
#endif
}

const char *lois_output_get(void)
{
    return output_buffer;
}

void lois_error_reset(void)
{
    error_length = 0;
    error_buffer[0] = '\0';
}

void lois_error_write(const char *text)
{
    if (!text)
        return;

    size_t length = strlen(text);

    if (error_length + length >= OUTPUT_SIZE)
        return;

    memcpy(
        error_buffer + error_length,
        text,
        length
    );

    error_length += length;
    error_buffer[error_length] = '\0';
}

const char *lois_error_get(void)
{
    return error_buffer;
}

void lois_print(const char *text)
{
    lois_output_write(text);
}
