#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define DELIMITER " "

/*
 * @brief Functon handling builtin commands like cd or exit
 * */
int execute_command(char **args) {
  if (args == NULL || args[0] == NULL) {
    return 0;
  }

  if (strcmp(args[0], "exit") == 0) {
    return 1;
  } else if (strcmp(args[0], "cd") == 0) {
    const char *dir = (args[1] == NULL) ? getenv("HOME") : args[1];

    if (dir == NULL) {
      fprintf(stderr, "HOME directory not set\n");
      return 0;
    }

    if (chdir(dir) != 0) {
      perror("cd");
    }
  } else {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
      perror("fork");
      return 0;
    }

    if (pid == 0) {
      printf("DEBUG: args[0] = '%s'\n", args[0]);
      if (execvp(args[0], args) == -1) {
        perror("child process");
        exit(EXIT_FAILURE);
      }
    } else {
      waitpid(pid, &status, 0);
    }
  }
  return 0;
}

/*
 * @brief Function writes current directory into buffer
 * */
char *get_cwd(char *buf, size_t bufsize) {
  if (getcwd(buf, bufsize) == NULL) {
    perror("getcwd");
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
    tokens[position++] = token;
    token =
        strtok(NULL, DELIMITER); // NULL because we work with the same string
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
  char *line;
  char **args;
  int status = 0;

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

    args = parse_args(line);

    status = execute_command(args);

    free(args);
    free(line);

    if (status == 1) {
      break;
    }
  }
  // First args, then line because args is array of pointers to line

  return 0;
}
