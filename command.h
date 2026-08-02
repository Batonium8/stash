#define MAX_INPUT 1024
#define MAX_ARGS 64
#define DELIMITER " "

typedef struct command {
  char **argv;
  struct command *next;
} Command;

char **tokenize(char *line, int *num_tokens);

Command *parse_tokens(char **tokens, int num_tokens);

int execute(Command *cmd);
