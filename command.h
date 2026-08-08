#ifndef COMMAND_H
#define COMMAND_H

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define DELIMITER " "

typedef struct command {
  char **argv;
  char *infile;
  char *outfile;
  int append;
  struct command *next;
} Command;

char **tokenize(char *line, int *num_tokens);

Command *parse_tokens(char **tokens, int num_tokens);

Command *parse_pipeline(char *line);

int execute_pipeline(Command *head);

void free_pipeline(Command *head);

#endif
