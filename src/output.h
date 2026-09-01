#ifndef LOIS_OUTPUT_H
#define LOIS_OUTPUT_H

void lois_output_reset(void);
void lois_output_write(const char *text);
const char *lois_output_get(void);

void lois_error_reset(void);
void lois_error_write(const char *text);
const char *lois_error_get(void);

void lois_print(const char *text);

#endif
