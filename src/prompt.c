#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
static int color_enabled(void){return getenv("C2SH_NO_COLOR")==NULL&&isatty(STDOUT_FILENO);}
static char*git_branch(void){FILE*f=popen("git symbolic-ref --short HEAD 2>/dev/null", "r");if(!f)return NULL;char b[256];if(!fgets(b,sizeof(b),f)){pclose(f);return NULL;}pclose(f);size_t n=strlen(b);while(n&& (b[n-1]=='\n'||b[n-1]=='\r'))b[--n]='\0';return n?strdup(b):NULL;}
char *make_prompt(Shell*sh){
 char cwd[PATH_MAX];if(!getcwd(cwd,sizeof(cwd)))snprintf(cwd,sizeof(cwd),"?");
 const char*home=getenv("HOME");char display[1024];
 if(home && (!strcmp(cwd,home) || (strncmp(cwd,home,strlen(home))==0 && cwd[strlen(home)]=='/'))){size_t off=strlen(home); size_t tail=strlen(cwd+off); if(tail>sizeof(display)-2) tail=sizeof(display)-2; display[0]='~'; memcpy(display+1,cwd+off,tail); display[tail+1]=0;}else{size_t n=strlen(cwd); if(n>sizeof(display)-1)n=sizeof(display)-1; memcpy(display,cwd,n); display[n]=0;}
 char*br=git_branch();char buf[4096];
 if(color_enabled()){
  if(br)snprintf(buf,sizeof(buf),"\\001\\033[1;31m\\002c2sh\\001\\033[0m\\002:\\001\\033[36m\\002%s\\001\\033[0m\\002 \\001\\033[33m\\002[%s]\\001\\033[0m\\002 %s$ ",display,br,sh->last_status?"!":"");
  else snprintf(buf,sizeof(buf),"\\001\\033[1;31m\\002c2sh\\001\\033[0m\\002:\\001\\033[36m\\002%s\\001\\033[0m\\002 %s$ ",display,sh->last_status?"!":"");
 }else snprintf(buf,sizeof(buf),"c2sh:%s%s$ ",display,br?br:"");
 free(br);return strdup(buf);
}
