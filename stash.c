#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 1024
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

int parse_args() { return 0; }

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
  fflush(stdout); // Reset buffer

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

  free(line);
}
