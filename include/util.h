#ifndef C2SH_UTIL_H
#define C2SH_UTIL_H
#include <stddef.h>
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);
void die_oom(void);
int is_blank_string(const char *s);
char *trim_copy(const char *s);
#endif
