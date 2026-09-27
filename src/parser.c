#include "parser.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static Command *cmd_new(void){Command*c=calloc(1,sizeof(*c));if(!c)die_oom();c->cap=8;c->argv=calloc((size_t)c->cap+1,sizeof(char*));if(!c->argv)die_oom();return c;}
static void cmd_arg(Command*c,const char*s){if(c->argc+1>=c->cap){c->cap*=2;c->argv=realloc(c->argv,((size_t)c->cap+1)*sizeof(char*));if(!c->argv)die_oom();}c->argv[c->argc++]=xstrdup(s);c->argv[c->argc]=NULL;}
static void redir_add(Command*c,RedirType t,int fd,const char*s){Redir*r=calloc(1,sizeof(*r));if(!r)die_oom();r->type=t;r->fd=fd;r->path=xstrdup(s);if(!c->redirs)c->redirs=r;else{Redir*p=c->redirs;while(p->next)p=p->next;p->next=r;}}
static void cmd_free(Command*c){if(!c)return;for(int i=0;i<c->argc;i++)free(c->argv[i]);free(c->argv);Redir*r=c->redirs;while(r){Redir*n=r->next;free(r->path);free(r);r=n;}free(c);}
static Pipeline *pipe_new(void){Pipeline*p=calloc(1,sizeof(*p));if(!p)die_oom();p->cap=4;p->commands=calloc(p->cap,sizeof(*p->commands));if(!p->commands)die_oom();return p;}
static void pipe_add(Pipeline*p,Command*c){if(p->count==p->cap){p->cap*=2;p->commands=realloc(p->commands,p->cap*sizeof(*p->commands));if(!p->commands)die_oom();}p->commands[p->count++]=c;}
static void pipe_free(Pipeline*p){if(!p)return;for(size_t i=0;i<p->count;i++)cmd_free(p->commands[i]);free(p->commands);free(p);}
int parse_tokens(const TokenList*t,CommandList*out,char**error){memset(out,0,sizeof(*out));size_t i=0;out->cap=8;out->items=calloc(out->cap,sizeof(*out->items));out->conditions=calloc(out->cap,sizeof(*out->conditions));if(!out->items||!out->conditions)die_oom();Condition next=COND_ALWAYS;
 while(t->items[i].type!=TOK_EOF){Pipeline*p=pipe_new();Command*c=cmd_new();int have=0;
  while(1){Token *x=&t->items[i]; if(x->type==TOK_WORD){cmd_arg(c,x->text);have=1;i++;continue;}
   if(x->type==TOK_REDIR_IN||x->type==TOK_REDIR_OUT||x->type==TOK_REDIR_APPEND||x->type==TOK_REDIR_ERR||x->type==TOK_REDIR_ERR_APPEND){RedirType rt;int fd=STDOUT_FILENO;if(x->type==TOK_REDIR_IN){rt=REDIR_IN;fd=STDIN_FILENO;}else if(x->type==TOK_REDIR_OUT){rt=REDIR_OUT;}else if(x->type==TOK_REDIR_APPEND){rt=REDIR_APPEND;}else if(x->type==TOK_REDIR_ERR){rt=REDIR_ERR;fd=STDERR_FILENO;}else{rt=REDIR_ERR_APPEND;fd=STDERR_FILENO;}i++;if(t->items[i].type!=TOK_WORD){*error=xstrdup("redirection requires a filename");pipe_free(p);cmd_free(c);command_list_free(out);return -1;}redir_add(c,rt,fd,t->items[i].text);i++;continue;}
   if(x->type==TOK_PIPE){if(!have){*error=xstrdup("pipe requires a command");pipe_free(p);cmd_free(c);command_list_free(out);return -1;}pipe_add(p,c);c=cmd_new();have=0;i++;continue;}break;
  }
  if(!have){cmd_free(c);pipe_free(p);*error=xstrdup("expected command");command_list_free(out);return -1;}pipe_add(p,c);
  if(t->items[i].type==TOK_AMP){p->background=1;i++;}
  if(out->count==out->cap){out->cap*=2;out->items=realloc(out->items,out->cap*sizeof(*out->items));out->conditions=realloc(out->conditions,out->cap*sizeof(*out->conditions));if(!out->items||!out->conditions)die_oom();}
  out->items[out->count]=p;out->conditions[out->count]=next;out->count++;next=COND_ALWAYS;
  if(t->items[i].type==TOK_SEMI){i++;continue;} if(t->items[i].type==TOK_AND_IF){next=COND_AND;i++;continue;} if(t->items[i].type==TOK_OR_IF){next=COND_OR;i++;continue;} if(t->items[i].type==TOK_EOF)break;
  *error=xstrdup("unexpected operator");command_list_free(out);return -1;
 }
 return 0;}
void command_list_free(CommandList*l){if(!l)return;for(size_t i=0;i<l->count;i++)pipe_free(l->items[i]);free(l->items);free(l->conditions);memset(l,0,sizeof(*l));}
