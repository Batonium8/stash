#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 1024

/*
 * @brief Function prints current directory
 * @return writes current directory into buffer
 * */
char *get_cwd(char *buf, size_t bufsize) {
  if (getcwd(buf, bufsize) == NULL) {
    perror("Error occured while getcwd");
    return NULL;
  }
  return buf;
}

/*
 * @brief Function to take input from user
 * */
void shell_input(void) {
  char *buf = NULL;
  size_t bufsize = 0;
  char cwd[MAX_INPUT];

  printf("%s $> ", get_cwd(cwd, sizeof(cwd)));

  fflush(stdout);
  if (getline(&buf, &bufsize, stdin) == -1) {
  }

  printf("Input %s", buf);

  free(buf);
}

int main(void) { shell_input(); }
