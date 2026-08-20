#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"

/*
 * @brief Function writes current directory into buffer
 * */
static char *get_cwd(char *buf, size_t bufsize) {
  if (getcwd(buf, bufsize) == NULL) {
    perror("getcwd");
    return NULL;
  }
  return buf;
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
  char *line;
  Command *pipeline;
  int result = 0;

  while (1) {
    line = shell_input();
    if (line == NULL) {
      printf("\n");
      break;
    }
    if (strlen(line) == 0) {
      free(line);
      continue;
    }
    pipeline = parse_pipeline(line);

    if (pipeline == NULL) {
      free(line);
      continue;
    }

    result = execute_pipeline(pipeline);

    free_pipeline(pipeline);
    free(line);

    if (result == SHELL_EXIT)
      break;
  }

  return 0;
}
