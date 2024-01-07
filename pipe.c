#include "headers.h"

bool isolatePipes(char* args[], int* argsCount) {
  for (int i = 0; i < *argsCount; i++) {
    int argLen = strlen(args[i]);
    char* arg = args[i];

    for (int j = 0; j < argLen; j++) {
      if (arg[j] == '"') {
        int close = findClosingCharOrEndOfWord('"', arg, j);
        j = close;
        continue;
      } else if (arg[j] == '|') {
        int prevEndIndex = (j - 1);
        int nextStartInd = (j + 1);

        if (prevEndIndex < 0 && nextStartInd >= argLen) {
          continue;
        }

        char* before = NULL;
        char* after = NULL;

        if (prevEndIndex >= 0) {
          int beforeLen = prevEndIndex + 1;
          before = (char*)malloc(sizeof(char) * (beforeLen + 1));
          strncpy(before, args[i], beforeLen);
          before[beforeLen] = '\0';
        }
        if (nextStartInd < argLen) {
          int afterLen = (argLen - 1) - nextStartInd + 1;
          after = (char*)malloc(sizeof(char) * (afterLen + 1));
          strncpy(after, args[i] + nextStartInd, afterLen);
          after[afterLen] = '\0';
        }

        int redirectIndex = i;
        if (before != NULL) {
          if (*argsCount == MAX_ARGS) {
            if (before) free(before);
            if (after) free(after);

            fprintf(stderr, TCBRED
                    "Too many arguments, cannot execute redirects. Increase MAX_ARGS and "
                    "try again\n" RESET);
            return false;
          }
          redirectIndex += 1;
          *argsCount += 1;
          for (int j = *argsCount - 1; j > i; j--) {
            args[j] = args[j - 1];
          }
          args[i] = before;
        }

        char redirString[2];
        redirString[0] = '|';
        redirString[1] = '\0';

        free(args[redirectIndex]);
        args[redirectIndex] = strdup(redirString);

        int afterIndex = redirectIndex + 1;
        if (after != NULL) {
          if (*argsCount == MAX_ARGS) {
            if (before) free(before);
            if (after) free(after);

            fprintf(stderr, TCBRED
                    "Too many arguments, cannot execute redirects. Increase MAX_ARGS and "
                    "try again\n" RESET);
            return false;
          }

          *argsCount += 1;
          for (int j = *argsCount - 1; j > afterIndex; j--) {
            args[j] = args[j - 1];
          }
          args[afterIndex] = after;
        }

        i--;
        break;
      }
    }
  }

  if (strcmp(args[0], "|") == 0 || strcmp(args[*argsCount - 1], "|") == 0) {
    fprintf(stderr,
            TCBRED "Pipes are not allowed at the beginning or end of a command\n" RESET);
    fprintf(stderr, TCNRED "Pipes can only occur between two commands\n" RESET);
    return false;
  }
  for (int i = 0; i < *argsCount - 1; i++) {
    if (strcmp(args[i], "|") == 0 && strcmp(args[i + 1], "|") == 0) {
      fprintf(stderr, TCBRED "Continuous pipes are not allowed\n" RESET);
      return false;
    }
  }

  return true;
}

/*
https://www.youtube.com/watch?v=NkfIUo_Qq4c
*/
void executePipes(char* args[], int* argsCount) {
  /*
  Saving stdin and stdout to restore later
  */
  int OG_FDS[2];
  if ((OG_FDS[0] = dup(STDIN_FILENO)) < 0) {
    perror(TCBRED "dup" RESET);
    return;
  }
  if ((OG_FDS[1] = dup(STDOUT_FILENO)) < 0) {
    perror(TCBRED "dup" RESET);
    dup2(OG_FDS[0], STDIN_FILENO);
    close(OG_FDS[0]);
    return;
  };

  int pipeCount = 0;
  for (int i = 0; i < *argsCount; i++) {
    if (strcmp(args[i], "|") == 0) {
      pipeCount += 1;
    }
  }

  // this should never happen but meh
  if (pipeCount < 1) return;

  char formattedPipes[pipeCount * MAX_COMMAND_LEN + 1];
  formattedPipes[0] = '\0';

  /*
  Arbitrarily adding a closing | so the last command in pipeline can easily
  be executed without adding an individual condition for it
  */
  args[(*argsCount)++] = "|";

  /*
  Creating all the pipedd fd's at the start for ease of execution later.
    Also, this ensures we do not start a pipeline when it cannot be finished.
  */
  int fds[pipeCount][2];
  for (int i = 0; i < pipeCount; i++) {
    if (pipe(fds[i]) < 0) {
      perror(TCBRED "pipe" RESET);
      for (int j = 0; j < i; j++) {
        close(fds[j][0]), close(fds[j][1]);
      }
      goto restore;
    }
  }

  char* command = NULL;
  int runningPipeCount = 0;

  for (int i = 0; i < *argsCount; i++) {
    if (strcmp(args[i], "|") == 0) {
      /*
      Saving the fds to restore later
      */
      int fdSave[2];
      if ((fdSave[0] = dup(STDIN_FILENO)) < 0) {
        perror(TCBRED "dup" RESET);
        continue;
      }
      if ((fdSave[1] = dup(STDOUT_FILENO)) < 0) {
        perror("dup");
        close(fdSave[0]);
        continue;
      }

      /*
      If it is the first pipe, then we take input from stdin anyway,
        so no need to dup
      If it is the last pipe, we send output to stdout anyway, no need to
        dup
      */
      if (runningPipeCount != 0) {
        if (dup2(fds[runningPipeCount - 1][0], STDIN_FILENO) < 0) {
          perror(TCBRED "dup2" RESET);
          continue;
        }
        close(fds[runningPipeCount - 1][0]);
      }
      if (runningPipeCount != pipeCount) {
        if (dup2(fds[runningPipeCount][1], STDOUT_FILENO) < 0) {
          perror(TCBRED "dup2" RESET);
          if (dup2(fdSave[0], STDIN_FILENO) < 0) {
            perror(TCBRED "dup2" RESET);
            goto restore;
          }
          close(fdSave[0]);
          continue;
        }
        close(fds[runningPipeCount][1]);
      }

      /*
      Execute the command in the pipeline without saving
        to pastevents, hence NULL and false as we do not need
        to save a formattedCommand
      */
      char formattedCommand[MAX_COMMAND_LEN + 1];
      executeCommand(command, formattedCommand, true);
      stripEndingDelimiters(formattedCommand);
      strcat(formattedPipes, formattedCommand);
      if(runningPipeCount != pipeCount){
        strcat(formattedPipes, " | ");
      }

      runningPipeCount += 1;

      if (dup2(fdSave[0], STDIN_FILENO) < 0) {
        perror(TCBRED "dup" RESET);
        goto restore;
      }
      close(fdSave[0]);
      if (dup2(fdSave[1], STDOUT_FILENO) < 0) {
        perror(TCBRED "dup" RESET);
        goto restore;
      }
      close(fdSave[1]);

      continue;
    }
    // if the arg is not a |, that means it is an odd arg, that must be saved
    //  to be executed in the pipe
    command = args[i];
  }

  /*
  Get rid of the extra pipe we added
  */
  args[--(*argsCount)] = NULL;
  for(int i = 0; i < *argsCount; i++){
    free(args[i]);
  }

  *argsCount = 1;
  args[0] = strdup(formattedPipes);

restore:
  dup2(OG_FDS[0], STDIN_FILENO);
  close(OG_FDS[0]);
  dup2(OG_FDS[1], STDOUT_FILENO);
  close(OG_FDS[1]);
}
