#include "shell.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
void history_add(Shell*, const char*);
static char*default_history(void){const char*h=getenv("HOME");if(!h)h=".";size_t n=strlen(h)+16;char*p=malloc(n);if(!p)die_oom();snprintf(p,n,"%s/.c2sh_history",h);return p;}
void history_init(Shell*sh){const char*e=getenv("C2SH_HISTORY_FILE");if(e) sh->history_path=xstrdup(e); else sh->history_path=default_history();const char*sz=getenv("C2SH_HISTORY_SIZE");sh->history_limit=sz?atoi(sz):5000;if(sh->history_limit<100)sh->history_limit=100;FILE*f=fopen(sh->history_path,"r");if(!f)return;char*line=NULL;size_t cap=0;while(getline(&line,&cap,f)>0){size_t n=strlen(line);while(n&& (line[n-1]=='\n'||line[n-1]=='\r'))line[--n]='\0';if(n)history_add(sh,line);}free(line);fclose(f);}
void history_add(Shell*sh,const char*s){if(!s||!s[0])return;if(sh->history_len&&strcmp(sh->history[sh->history_len-1].text,s)==0)return;if(sh->history_len==sh->history_cap){sh->history_cap=sh->history_cap?sh->history_cap*2:128;sh->history=realloc(sh->history,sh->history_cap*sizeof(*sh->history));if(!sh->history)die_oom();}if(sh->history_len>=(size_t)sh->history_limit){free(sh->history[0].text);memmove(sh->history,sh->history+1,(sh->history_len-1)*sizeof(*sh->history));sh->history_len--;}sh->history[sh->history_len++]=(HistoryEntry){xstrdup(s),0};}
void history_save(Shell*sh){if(!sh->history_path)return;FILE*f=fopen(sh->history_path,"w");if(!f)return;chmod(sh->history_path,0600);size_t start=sh->history_len>(size_t)sh->history_limit?sh->history_len-(size_t)sh->history_limit:0;for(size_t i=start;i<sh->history_len;i++)fprintf(f,"%s\n",sh->history[i].text);fclose(f);}
void history_free(Shell*sh){history_save(sh);for(size_t i=0;i<sh->history_len;i++)free(sh->history[i].text);free(sh->history);free(sh->history_path);sh->history=NULL;sh->history_path=NULL;}
