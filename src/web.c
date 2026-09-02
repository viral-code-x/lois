#include "web.h"

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "output.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(int, lois_web_readline, (char *buffer, int max_len), {
    return Asyncify.handleSleep(function(wakeUp) {

        if (typeof window.loisRequestInput !== "function") {
            wakeUp(0);
            return;
        }

        window.loisRequestInput(function(value) {
            stringToUTF8(value + "\n", buffer, max_len);
            wakeUp(1);
        });
    });
});
#endif

const char *lois_run_source(const char *source)
{
    if (!source)
        return "LOIS: no source provided\n";

    lois_output_reset();
    lois_error_reset();

    TokenList tokens = lexer_tokenize(source);
    Statement *program = parser_parse(&tokens);

    if (parser_had_error())
    {
        parser_free(program);
        lexer_free(&tokens);

        lois_output_write(lois_error_get());

        return lois_output_get();
    }

    interpreter_run(program);

    if (interpreter_had_error())
        lois_output_write(lois_error_get());

    parser_free(program);
    lexer_free(&tokens);

    return lois_output_get();
}
