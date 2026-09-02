#ifndef LOIS_WEB_H
#define LOIS_WEB_H

const char *lois_run_source(const char *source);

#ifdef __EMSCRIPTEN__
int lois_web_readline(char *buffer, int max_len);
#endif

#endif
