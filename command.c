#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **tokenize(char *line, int *num_tokens) {
  char **tokens = malloc(MAX_ARGS * sizeof(*tokens));
  unsigned int position = 0;

  char *token = strtok(line, DELIMITER);

  while (token != NULL) {
    tokens[position++] = token;
    token = strtok(NULL, DELIMITER);
  }
  tokens[position] = NULL;

  *num_tokens = position;

  return tokens;
}

Command *parse_tokens(char **tokens, int num_tokens) {
  Command *command = malloc(sizeof(Command));
  if (command == NULL) {
    perror("malloc");
    return NULL;
  }
  command->argv = malloc(MAX_ARGS * sizeof(char *));
  if (command->argv == NULL) {
    perror("malloc");
    free(command);
    return NULL;
  }

  command->infile = NULL;
  command->outfile = NULL;
  command->append = 0;
  command->next = NULL;

  int argc = 0;
  int i = 0;
  while (i < num_tokens) {
    if (strcmp(tokens[i], ">") == 0) {
      if (i + 1 < num_tokens) {
        command->outfile = tokens[i + 1];
        i += 2; // skip filename
      } else {
        fprintf(stderr, "error: no file after redirect\n");
        free(command->argv);
        free(command);
        return NULL;
      }
    } else if (strcmp(tokens[i], "<") == 0) {
      if (i + 1 < num_tokens) {
        command->infile = tokens[i + 1];
        i += 2; // skip filename
      } else {
        fprintf(stderr, "error: no file after redirect\n");
        free(command->argv);
        free(command);
        return NULL;
      }
    } else if (strcmp(tokens[i], ">>") == 0) {
      if (i + 1 < num_tokens) {
        command->outfile = tokens[i + 1];
        command->append = 1;
        i += 2; // skip filename
      } else {
        fprintf(stderr, "error: no file after redirect\n");
        free(command->argv);
        free(command);
        return NULL;
      }
    } else {
      if (argc == MAX_ARGS - 1) { // last for null terminator
        fprintf(stderr, "error: too many arguments\n");
        free(command->argv);
        free(command);
        return NULL;
      }
      command->argv[argc++] = tokens[i];
      i++;
    }
  }
  command->argv[argc] = NULL; // terminator

  return command;
}

int execute(Command *cmd) { return 0; }
