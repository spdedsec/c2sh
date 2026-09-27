#include "parser.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>

static char *expand_one(const char *s,int last_status){
 size_t cap=strlen(s)+64,len=0; char *b=malloc(cap); if(!b) die_oom(); int quote=0;
 #define PUT(ch) do{if(len+1>=cap){cap*=2;b=realloc(b,cap);if(!b)die_oom();}b[len++]=(ch);}while(0)
 for(size_t i=0;s[i];){
  char c=s[i];
  if(c=='\'' && quote==0){quote=1;i++;continue;}
  if(c=='\'' && quote==1){quote=0;i++;continue;}
  if(c=='"' && quote==0){quote=2;i++;continue;}
  if(c=='"' && quote==2){quote=0;i++;continue;}
  if(c=='$' && quote!=1){
   if(s[i+1]=='?'){char tmp[32];snprintf(tmp,sizeof(tmp),"%d",last_status);for(char*q=tmp;*q;q++)PUT(*q);i+=2;continue;}
   size_t j=i+1;if(s[j]=='{')j++;size_t st=j;while(s[j]&&(isalnum((unsigned char)s[j])||s[j]=='_'))j++;
   if(j>st){char*name=xstrndup(s+st,j-st);const char*v=getenv(name);if(v)for(const char*p=v;*p;p++)PUT(*p);free(name);i=(s[j]=='}'?j+1:j);continue;}
  }
  PUT(c);i++;
 }
 PUT('\0');
 #undef PUT
 return b;
}
static void replace_arg(Command*c,int idx,char**vals,size_t n){
 char*old=c->argv[idx];if(n==0){free(old);c->argv[idx]=xstrdup("");return;} if(n==1){free(old);c->argv[idx]=xstrdup(vals[0]);return;}
 int extra=(int)n-1;while(c->argc+extra+1>=c->cap){c->cap*=2;c->argv=realloc(c->argv,((size_t)c->cap+1)*sizeof(char*));if(!c->argv)die_oom();}
 for(int k=c->argc;k>idx;k--) c->argv[k+extra]=c->argv[k];
 free(old);
 for(size_t k=0;k<n;k++) c->argv[idx+(int)k]=xstrdup(vals[k]);
 c->argc+=(int)extra;
}
int expand_command(Command*c,int last_status){for(int i=0;i<c->argc;i++){int had_quote=(strchr(c->argv[i],'\'')||strchr(c->argv[i],'"'));char*e=expand_one(c->argv[i],last_status);free(c->argv[i]);c->argv[i]=e;glob_t g;memset(&g,0,sizeof(g));int gr=had_quote?GLOB_NOMATCH:glob(e,GLOB_NOCHECK,NULL,&g);if(gr==0&&g.gl_pathc>0){size_t n=g.gl_pathc;replace_arg(c,i,g.gl_pathv,n);i+=(int)n-1;}globfree(&g);}for(Redir*r=c->redirs;r;r=r->next){char*e=expand_one(r->path,last_status);free(r->path);r->path=e;}return 0;}
int expand_pipeline(Pipeline*p,int last_status){for(size_t i=0;i<p->count;i++)expand_command(p->commands[i],last_status);return 0;}
