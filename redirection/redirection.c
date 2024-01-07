#include "../utils/headers.h"

bool redirect(char* args[], int* numArgs) {
  bool runInBackground = false;

  if (*numArgs < 1) {
    return false;
  } else {
    char* lastArg = args[*numArgs - 1];
    int len = strlen(lastArg);
    if (strcmp(lastArg, "&") == 0) {
      runInBackground = true;
      free(lastArg);
      args[--(*numArgs)] = NULL;
    } else if (lastArg[len - 1] == '&') {
      runInBackground = true;
      args[*numArgs - 1][len - 1] = '\0';
    }
  }

  if (*numArgs < 1) {
    return false;
  }

  /*
  Defaulting to the standard input and output
  */
  char* argsCopy[*numArgs + 1];

  /*
  Creating a copy because the function executes by interpreting <filename as an fd of
    the particular type, and then deleting it from the arguments, because the redirection
    will be complete. Hence, this could affect the tokens.
  */
  for (int i = 0; i < *numArgs; i++) argsCopy[i] = strdup(args[i]);

  int argsCount = *numArgs;
  int inputFD = STDIN_FILENO, outputFD = STDOUT_FILENO;
  
  bool inputSet = false, outputSet = false;
  char* inputRedir = NULL, *outputRedir = NULL;

  for (int i = 0; i < argsCount; i++) {
    if (strcmp(argsCopy[i], "<") == 0) {
      if (inputSet) {
        fprintf(stderr,
                TCBRED "Too many input redirects, only one allowed per command\n" RESET);
        goto free;
      }
      if (i >= argsCount - 1) {
        fprintf(stderr, TCBRED "No item specified to redirect input from\n" RESET);
        goto free;
      } else if (i <= 0) {
        fprintf(stderr, TCBRED "No item specified to redirect input to\n" RESET);
        goto free;
      }

      inputFD = open(argsCopy[i + 1], O_RDONLY);
      if (inputFD == -1) {
        char err[MAX_NAME_LENGTH];
        snprintf(err, MAX_NAME_LENGTH, TCBRED "Unable to open file %s" TCNRED,
                 argsCopy[i + 1]);
        perror(err);
        fprintf(stderr, RESET);
        goto free;
      }

      inputRedir = (char*)malloc(sizeof(char) * (MAX_NAME_LENGTH+1));
      snprintf(inputRedir, MAX_NAME_LENGTH, "%s %s", argsCopy[i], argsCopy[i+1]);

      free(argsCopy[i]);
      free(argsCopy[i + 1]);
      argsCount -= 2;

      for (int j = i; j < argsCount; j++) {
        argsCopy[j] = argsCopy[j + 2];
      }
      i--;

      inputSet = true;
    }

    if (strcmp(argsCopy[i], ">") == 0 || strcmp(argsCopy[i], ">>") == 0) {
      if (outputSet) {
        fprintf(stderr,
                TCBRED "Too many output redirects, only one allowed per command\n" RESET);
        goto free;
      }
      if (i <= 0) {
        fprintf(stderr, TCBRED "No item specified to redirect output from\n" RESET);
        goto free;
      } else if (i >= argsCount - 1) {
        fprintf(stderr, TCBRED "No item specified to redirect output to\n" RESET);
        goto free;
      }

      if (strcmp(argsCopy[i], ">") == 0) {
        outputFD = open(argsCopy[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      } else {
        outputFD = open(argsCopy[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
      }

      if (outputFD == -1) {
        char err[MAX_NAME_LENGTH];
        snprintf(err, MAX_NAME_LENGTH, TCBRED "Unable to open file %s" TCNRED,
                 argsCopy[i + 1]);
        perror(err);
        fprintf(stderr, RESET);
        goto free;
      }

      outputRedir = (char*)malloc(sizeof(char) * (MAX_NAME_LENGTH+1));
      snprintf(outputRedir, MAX_NAME_LENGTH, "%s %s", argsCopy[i], argsCopy[i+1]);

      free(argsCopy[i]);
      free(argsCopy[i + 1]);
      argsCount -= 2;

      for (int j = i; j < argsCount; j++) {
        argsCopy[j] = argsCopy[j + 2];
      }
      i--;

      outputSet = true;
    }
  }

  argsCopy[argsCount] = NULL;

  char formattedCommand[MAX_COMMAND_LEN];
  executeArgs(argsCopy, &argsCount, formattedCommand, false, true, inputFD, outputFD, false, true);
  for(int i = 0; i < *numArgs; i++) free(args[i]);
  *numArgs = 1;
  stripBeginningDelimiters(formattedCommand);
  stripEndingDelimiters(formattedCommand);
  args[0] = strdup(formattedCommand);

  bool inputRedirAssigned = false, outputRedirAssigned = false;
  if(inputRedir){
    args[(*numArgs)++] = inputRedir;
    inputRedirAssigned = true;
  }
  if(outputRedir){
    args[(*numArgs)++] = outputRedir;
    outputRedirAssigned = true;
  }
  args[*numArgs] = NULL;


free:
  for (int i = 0; i < argsCount; i++) {
    free(argsCopy[i]);
  }

  if (inputFD != STDIN_FILENO) close(inputFD);
  if (outputFD != STDOUT_FILENO) close(outputFD);

  if(inputRedir && !inputRedirAssigned) free(inputRedir);
  if(outputRedir && !outputRedirAssigned) free(outputRedir);

  return runInBackground;
}

bool isolateRedirect(char* args[], int* argsCount, char redir, bool searchDouble) {
  for (int i = 0; i < *argsCount; i++) {
    char* chPointer;
    int argLen = strlen(args[i]);

    if ((chPointer = strchr(args[i], redir))) {
      int prevEndIndex = (chPointer - 1) - args[i];
      int nextStartInd = (chPointer + 1) - args[i];
      if (searchDouble && *(chPointer + 1) == redir) {
        nextStartInd += 1;
      }

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

      char redirCharStr[2];
      redirCharStr[0] = redir;
      redirCharStr[1] = '\0';

      char redirString[3];
      redirString[0] = '\0';
      for (int j = prevEndIndex + 1; j < nextStartInd; j++) {
        strcat(redirString, redirCharStr);
      }

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
    }
  }

  return true;
}

bool isolateAllRedirects(char* args[], int* argsCount) {
  if (!isolateRedirect(args, argsCount, '<', false)) return false;
  if (!isolateRedirect(args, argsCount, '>', true)) return false;

  return true;
}