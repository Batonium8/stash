#include "command.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*
 * Function to split line in tokens using strsep()
 *
 * */

static char **tokenize(char *line, int *num_tokens) {
  char **tokens = malloc(MAX_ARGS * sizeof(*tokens));
  if (tokens == NULL) {
    perror("malloc");
    return NULL;
  }
  size_t position = 0;

  char *token;
  char *cursor = line;
  while ((token = strsep(&cursor, DELIMITER)) != NULL) {
    // skip if first char of token is end of line (whitespace)
    if (*token == '\0') {
      continue;
    }
    // TODO: rewrite to realloc
    if (position >= MAX_ARGS - 1) {
      fprintf(stderr, "error: too many arguments\n");
      free(tokens);
      *num_tokens = 0;
      return NULL;
    }

    tokens[position++] = token;
  }
  tokens[position] = NULL;

  *num_tokens = position;

  return tokens;
}
/*
 * Function to parse tokens array in Command structure
 *
 * @note Because of using strdup Command structure now owns
 *  strings and calling function need to free memory
 */
static Command *parse_tokens(char **tokens, int num_tokens) {
  if (tokens == NULL) {
    return NULL;
  }
  if (num_tokens <= 0) {
    fprintf(stderr, "error: empty command\n");
    return NULL;
  }

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
      // Check if file exists after redirect
      if (i + 1 < num_tokens) {
        // If multiple redirects of the same type: last one wins
        // Free previous to avoid memory leak
        if (command->outfile != NULL) {
          free(command->outfile);
        }
        command->outfile = strdup(tokens[i + 1]);
        if (command->outfile == NULL) {
          perror("strdup");
          goto cleanup;
        }
        i += 2; // skip filename
      } else {
        fprintf(stderr, "error: no file after redirect\n");
        goto cleanup;
      }
    } else if (strcmp(tokens[i], "<") == 0) {
      if (i + 1 < num_tokens) {
        if (command->infile != NULL) {
          free(command->infile);
        }
        command->infile = strdup(tokens[i + 1]);
        if (command->infile == NULL) {
          perror("strdup");
          goto cleanup;
        }
        i += 2; // skip filename
      } else {
        fprintf(stderr, "error: no file after redirect\n");
        goto cleanup;
      }
    } else if (strcmp(tokens[i], ">>") == 0) {
      if (i + 1 < num_tokens) {
        if (command->outfile != NULL) {
          free(command->outfile);
        }
        command->outfile = strdup(tokens[i + 1]);
        if (command->outfile == NULL) {
          perror("strdup");
          goto cleanup;
        }
        command->append = 1;
        i += 2; // skip filename
      } else {
        fprintf(stderr, "error: no file after redirect\n");
        goto cleanup;
      }
    } else {
      if (argc == MAX_ARGS - 1) { // last for null terminator
        fprintf(stderr, "error: too many arguments\n");
        goto cleanup;
      }
      command->argv[argc] = strdup(tokens[i]);
      if (command->argv[argc] == NULL) {
        perror("strdup");
        goto cleanup;
      }
      argc++;
      i++;
    }
  }
  command->argv[argc] = NULL; // terminator

  return command;

cleanup:
  for (int i = 0; i < argc; i++) {
    free(command->argv[i]);
  }
  free(command->infile);
  free(command->outfile);
  free(command->argv);
  free(command);

  return NULL;
}

Command *parse_pipeline(char *line) {
  Command *head = NULL;
  Command *curr = NULL;

  char *curr_str = line;
  char *next_str = NULL;

  int num_tokens = 0;

  while (curr_str != NULL) {
    char *pipe_position = strchr(curr_str, '|');

    if (pipe_position != NULL) {
      *pipe_position = '\0';
      pipe_position++;

      next_str = pipe_position;

    } else {
      next_str = NULL;
    }

    char **tokens = tokenize(curr_str, &num_tokens);

    if (tokens == NULL) {
      free_pipeline(head);
      return NULL;
    }

    Command *command = parse_tokens(tokens, num_tokens);

    free(tokens);

    if (command == NULL) {
      free_pipeline(head);
      return NULL;
    }

    if (head == NULL) {
      head = command;
      curr = head;
    } else {
      curr->next = command;
      curr = command;
    }

    curr_str = next_str;
  }
  return head;
}

static int execute_builtin(Command *head) {
  if (head == NULL || head->argv[0] == NULL) {
    return BUILTIN_NOT_FOUND;
  }

  if (strcmp(head->argv[0], "cd") == 0) {
    if (head->argv[2] != NULL) {
      fprintf(stderr, "cd: too many arguments\n");
      return 1;
    }
    const char *dir = (head->argv[1] == NULL) ? getenv("HOME") : head->argv[1];

    if (dir == NULL) {
      fprintf(stderr, "HOME directory not set\n");
      return 1;
    }

    if (chdir(dir) != 0) {
      perror("cd");
      return 1;
    }
    return 0;
  } else if (strcmp(head->argv[0], "exit") == 0) {
    return SHELL_EXIT;
  }

  return BUILTIN_NOT_FOUND;
}

// @note - called only inside execute_pipeline()
static void execute_command(Command *head) {
  if (head == NULL || head->argv[0] == NULL) {
    exit(EXIT_FAILURE);
  }

  int exit_status = execute_builtin(head);
  if (exit_status != BUILTIN_NOT_FOUND) {
    exit(exit_status == SHELL_EXIT ? 0 : exit_status);
  }

  if (head->infile != NULL) {
    int file = open(head->infile, O_RDONLY);
    if (file == -1) {
      perror(head->infile);
      exit(EXIT_FAILURE);
    }
    dup2(file, STDIN_FILENO);
    close(file);
  }

  if (head->outfile != NULL) {
    int flags = O_WRONLY | O_CREAT;
    if (head->append) {
      flags |= O_APPEND;
    } else {
      flags |= O_TRUNC;
    }
    int file =
        open(head->outfile, flags, 0644); // 0644 - default file permissions
    if (file == -1) {
      perror(head->outfile);
      exit(EXIT_FAILURE);
    }
    dup2(file, STDOUT_FILENO);
    close(file);
  }

  execvp(head->argv[0], head->argv);
  perror("child process");
  exit(EXIT_FAILURE);
}

int execute_pipeline(Command *head) {
  if (head == NULL) {
    return 0;
  }
  if (head->next == NULL) {
    int builtin_exit_code = execute_builtin(head);
    if (builtin_exit_code != BUILTIN_NOT_FOUND) {
      return builtin_exit_code;
    }
  }
  Command *curr = head;
  pid_t pid, last_pid = 0;

  // Previous pipe read end
  int fd_in = 0;
  while (curr != NULL) {
    int fd[2];

    if (curr->next != NULL) {
      if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
      }
    }

    pid = fork();

    if (pid < 0) {
      perror("fork");
      return 1;
    }
    if (pid == 0) {
      if (fd_in != 0) {
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
      }
      if (curr->next != NULL) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
      }
      execute_command(curr);
    } else {
      if (fd_in != 0) {
        close(fd_in);
      }
      if (curr->next != NULL) {
        close(fd[1]);
        fd_in = fd[0];
      }
      last_pid = pid;
    }
    curr = curr->next;
  }

  pid_t wpid;
  int status, last_exit_code = 0;
  while ((wpid = wait(&status)) > 0) {
    if (wpid == last_pid && WIFEXITED(status)) {
      last_exit_code = WEXITSTATUS(status);
    }
  }

  return last_exit_code;
}

void free_pipeline(Command *head) {
  while (head != NULL) {
    Command *next = head->next;
    for (int i = 0; head->argv[i] != NULL; i++) {
      free(head->argv[i]);
    }
    free(head->argv);
    free(head->infile);
    free(head->outfile);
    free(head);
    head = next;
  }
}
