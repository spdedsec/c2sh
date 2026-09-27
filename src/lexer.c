#include "lexer.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void push(TokenList *l, TokenType t, const char *s, size_t n, size_t pos){
 if(l->len==l->cap){l->cap=l->cap?l->cap*2:32;l->items=realloc(l->items,l->cap*sizeof(*l->items));if(!l->items)die_oom();}
 l->items[l->len++] = (Token){t,xstrndup(s,n),pos};
}
static int op(const char *p, TokenType *t, size_t *n){
 if(!strncmp(p,"||",2)){*t=TOK_OR_IF;*n=2;return 1;} if(!strncmp(p,"&&",2)){*t=TOK_AND_IF;*n=2;return 1;}
 if(!strncmp(p,"2>>",3)){*t=TOK_REDIR_ERR_APPEND;*n=3;return 1;} if(!strncmp(p,"2>",2)){*t=TOK_REDIR_ERR;*n=2;return 1;}
 switch(*p){case '|':*t=TOK_PIPE;*n=1;return 1;case ';':*t=TOK_SEMI;*n=1;return 1;case '&':*t=TOK_AMP;*n=1;return 1;case '<':*t=TOK_REDIR_IN;*n=1;return 1;case '>': if(p[1]=='>'){*t=TOK_REDIR_APPEND;*n=2;}else{*t=TOK_REDIR_OUT;*n=1;}return 1;default:return 0;}
}
int lex_line(const char *line, TokenList *out, char **error){ memset(out,0,sizeof(*out)); size_t i=0,n=strlen(line);
 while(i<n){ while(i<n && isspace((unsigned char)line[i]))i++; if(i==n)break; TokenType t;size_t on;
  if(op(line+i,&t,&on)){push(out,t,line+i,on,i);i+=on;continue;}
  size_t start=i; size_t cap=32,len=0; char *buf=malloc(cap); if(!buf)die_oom(); int quote=0;
  while(i<n){ char c=line[i]; if(!quote && (isspace((unsigned char)c)||op(line+i,&t,&on)))break;
   if(c=='\\'){ if(i+1>=n){free(buf);*error=xstrdup("trailing backslash");token_list_free(out);return -1;} if(len+1>=cap){cap*=2;buf=realloc(buf,cap);if(!buf)die_oom();}buf[len++]=line[i+1];i+=2;continue; }
   if(c=='\'' && quote==0){ if(len+1>=cap){cap*=2;buf=realloc(buf,cap);if(!buf)die_oom();} buf[len++]=c; quote=1;i++;continue;} if(c=='\''&&quote==1){ if(len+1>=cap){cap*=2;buf=realloc(buf,cap);if(!buf)die_oom();} buf[len++]=c; quote=0;i++;continue;}
   if(c=='"' && quote==0){ if(len+1>=cap){cap*=2;buf=realloc(buf,cap);if(!buf)die_oom();} buf[len++]=c; quote=2;i++;continue;} if(c=='"'&&quote==2){ if(len+1>=cap){cap*=2;buf=realloc(buf,cap);if(!buf)die_oom();} buf[len++]=c; quote=0;i++;continue;}
   if(len+1>=cap){cap*=2;buf=realloc(buf,cap);if(!buf)die_oom();}buf[len++]=c;i++;
  }
  if(quote){free(buf);*error=xstrdup("unterminated quote");token_list_free(out);return -1;} buf[len]='\0';
  if(start==i){free(buf);*error=xstrdup("unexpected token");token_list_free(out);return -1;}
  push(out,TOK_WORD,buf,len,start);free(buf);
 }
 push(out,TOK_EOF,"",0,n); return 0;
}
void token_list_free(TokenList *l){for(size_t i=0;i<l->len;i++)free(l->items[i].text);free(l->items);memset(l,0,sizeof(*l));}
const char *token_type_name(TokenType t){switch(t){case TOK_WORD:return "word";case TOK_PIPE:return "|";case TOK_OR_IF:return "||";case TOK_AND_IF:return "&&";case TOK_SEMI:return ";";case TOK_AMP:return "&";case TOK_REDIR_IN:return "<";case TOK_REDIR_OUT:return ">";case TOK_REDIR_APPEND:return ">>";case TOK_REDIR_ERR:return "2>";case TOK_REDIR_ERR_APPEND:return "2>>";case TOK_EOF:return "end of input";}return "token";}
