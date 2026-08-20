#ifndef COMMAND_H
#define COMMAND_H

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define DELIMITER " "
#define BUILTIN_NOT_FOUND -1
#define SHELL_EXIT -2

typedef struct command {
  char **argv;
  char *infile;
  char *outfile;
  int append;
  struct command *next;
} Command;

Command *parse_pipeline(char *line);

int execute_pipeline(Command *head);

void free_pipeline(Command *head);

#endif
