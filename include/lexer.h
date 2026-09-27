#ifndef C2SH_LEXER_H
#define C2SH_LEXER_H
#include <stddef.h>
typedef enum { TOK_WORD, TOK_PIPE, TOK_OR_IF, TOK_AND_IF, TOK_SEMI, TOK_AMP, TOK_REDIR_IN, TOK_REDIR_OUT, TOK_REDIR_APPEND, TOK_REDIR_ERR, TOK_REDIR_ERR_APPEND, TOK_EOF } TokenType;
typedef struct { TokenType type; char *text; size_t pos; } Token;
typedef struct { Token *items; size_t len; size_t cap; } TokenList;
int lex_line(const char *line, TokenList *out, char **error);
void token_list_free(TokenList *list);
const char *token_type_name(TokenType type);
#endif
