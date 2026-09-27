#ifndef C2SH_PARSER_H
#define C2SH_PARSER_H
#include "lexer.h"
typedef enum { COND_ALWAYS, COND_AND, COND_OR } Condition;
typedef enum { REDIR_IN, REDIR_OUT, REDIR_APPEND, REDIR_ERR, REDIR_ERR_APPEND } RedirType;
typedef struct Redir { RedirType type; int fd; char *path; struct Redir *next; } Redir;
typedef struct Command { char **argv; int argc; int cap; Redir *redirs; } Command;
typedef struct Pipeline { Command **commands; size_t count; size_t cap; int background; } Pipeline;
typedef struct CommandList { Pipeline **items; Condition *conditions; size_t count; size_t cap; } CommandList;
int parse_tokens(const TokenList *tokens, CommandList *out, char **error);
void command_list_free(CommandList *list);
int expand_command(Command *cmd, int last_status);
int expand_pipeline(Pipeline *pipeline, int last_status);
#endif
