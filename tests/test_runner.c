#include "lexer.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void test_lex(void){TokenList t;char*e=NULL;assert(lex_line("echo hello | tr a b > out",&t,&e)==0);assert(t.len==9);assert(t.items[0].type==TOK_WORD);assert(strcmp(t.items[0].text,"echo")==0);assert(t.items[2].type==TOK_PIPE);assert(t.items[6].type==TOK_REDIR_OUT);token_list_free(&t);}
static void test_parse(void){TokenList t;CommandList c;char*e=NULL;assert(lex_line("printf hi | wc -c &",&t,&e)==0);assert(parse_tokens(&t,&c,&e)==0);assert(c.count==1);assert(c.items[0]->count==2);assert(c.items[0]->background==1);assert(strcmp(c.items[0]->commands[1]->argv[0],"wc")==0);command_list_free(&c);token_list_free(&t);}
int main(void){test_lex();test_parse();puts("unit tests: ok");return 0;}
