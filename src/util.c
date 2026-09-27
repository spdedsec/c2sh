#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
void die_oom(void){ fputs("c2sh: out of memory\n", stderr); exit(70); }
char *xstrdup(const char *s){ if(!s){return NULL;} char *p=strdup(s); if(!p) die_oom(); return p; }
char *xstrndup(const char *s,size_t n){ char *p=malloc(n+1); if(!p) die_oom(); memcpy(p,s,n); p[n]='\0'; return p; }
int is_blank_string(const char *s){ while(*s){if(!isspace((unsigned char)*s))return 0;s++;} return 1; }
char *trim_copy(const char *s){ while(isspace((unsigned char)*s))s++; size_t n=strlen(s); while(n && isspace((unsigned char)s[n-1]))n--; return xstrndup(s,n); }
