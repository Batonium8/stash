#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define DELIMITER " "

/*
 * @brief Function writes current directory into buffer
 * */
char *get_cwd(char *buf, size_t bufsize) {
  if (getcwd(buf, bufsize) == NULL) {
    perror("Error occured while getcwd");
    return NULL;
  }
  return buf;
}

/*
 * @brief Parsing arguments from line
 *
 * @note Free the array before freeing the line
 * */
char **parse_args(char *line) {
  char **tokens = malloc(MAX_ARGS * sizeof(*tokens));
  unsigned int position = 0;

  char *token = strtok(line, DELIMITER);

  while (token != NULL) {
    token =
        strtok(NULL, DELIMITER); // NULL because we work with the same string
    tokens[position++] = token;
  }

  tokens[position] = NULL; // terminating execvp

  return tokens;
}

/*
 * @brief Function to take input from user
 *
 * @note Dont forget to free() buffer
 * */
char *shell_input(void) {
  char *buf = NULL;
  size_t bufsize = 0;
  char cwd[MAX_INPUT];

  if (get_cwd(cwd, sizeof(cwd)) == NULL) {
    snprintf(cwd, sizeof(cwd), "?");
  }
  printf("%s $> ", cwd);
  fflush(stdout); // Flushing buffer

  if (getline(&buf, &bufsize, stdin) == -1) {
    free(buf);
    // Newline if CTRL+D or EOF
    if (feof(stdin)) {
      printf("\n");
    } else {
      perror("getline");
    }
    return NULL;
  }
  //
  buf[strcspn(buf, "\n")] = '\0';
  return buf;
}

int main(void) {
  char *line = shell_input();
  if (line == NULL) {
    return 0;
  }

  char **args = parse_args(line);

  // First args, then line because args is array of pointers to line
  free(args);
  free(line);
}
