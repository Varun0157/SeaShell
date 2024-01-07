#include "headers.h"

/**
 * @brief The past events, upto 15 stored
 */
char* pastEvents[MAX_ARGS];
int pastEventsSize;

void freePastEvents() {
  for (int i = 0; i < pastEventsSize; i++) {
    free(pastEvents[i]);
  }
  pastEventsSize = 0;
}

void loadPastEvents() {
  pastEventsSize = 0;

  char* historyFilePath = (char*)malloc(sizeof(char) * (MAX_PATH_LENGTH + 1));
  getExecutableDir(historyFilePath);
  strcat(historyFilePath, "/");
  strcat(historyFilePath, HISTORY_FILE);

  FILE* fd = fopen(HISTORY_FILE, "ab+");
  if (!fd) {
    fprintf(
        stderr, TCBRED
        "Unable to open past commands file, command history may not be stored.\n" RESET);
    return;
  }

  free(historyFilePath);

  char buffer[MAX_NAME_LENGTH + 1];
  while (fgets(buffer, MAX_NAME_LENGTH, fd)) {
    buffer[strlen(buffer)] = '\0';
    stripEndingDelimiters(buffer);
    pastEvents[pastEventsSize++] = strdup(buffer);
  }

  fclose(fd);
}

void updatePastEvents(char* command) {
  /*
  If the command contains pastevents, or it is the same as the
    previous command, then do not store it.
  */
  if (strstr(command, "pastevents") ||
      (pastEventsSize > 0 && strcmp(command, pastEvents[pastEventsSize - 1]) == 0)) {
    return;
  }

  /*
  If we have reached max size, then delete the first (oldest) command,
    and shift the rest to make space for the new one.
  */
  if (pastEventsSize == HISTORY_SIZE) {
    free(pastEvents[0]);
    for (int i = 0; i < HISTORY_SIZE - 1; i++) {
      pastEvents[i] = pastEvents[i + 1];
    }
    pastEventsSize -= 1;
  }

  pastEvents[pastEventsSize++] = strdup(command);

  char* historyFilePath = (char*)malloc(sizeof(char) * MAX_PATH_LENGTH + 1);
  getExecutableDir(historyFilePath);
  strcat(historyFilePath, "/");
  strcat(historyFilePath, HISTORY_FILE);

  /*
  Again, this can be made much cleaner using fprintf, but I assumed
    we had to use read, write, open syscalls.
  Removing the file because I could not find another way to clear it,
    I can change this by using fopen.
  */
  remove(historyFilePath);

  int fd = open(historyFilePath, O_WRONLY | O_CREAT, 0664);

  for (int i = 0; i < pastEventsSize; i++) {
    int commandLen = strlen(pastEvents[i]);

    if (commandLen < 1) {
      continue;
    }

    char buffer[commandLen + 1];
    strcpy(buffer, pastEvents[i]);
    buffer[commandLen] = '\n';

    write(fd, buffer, commandLen + 1);
  }

  free(historyFilePath);
  close(fd);
}

void pastevents(char* args[], int* argsCount) {
  if (*argsCount == 1) {
    if (pastEventsSize == 0) {
      fprintf(stderr, TCNYEL "No past events stored.\n" RESET);
      return;
    }
    for (int i = 0; i < pastEventsSize; i++) {
      printf(TCNMAG "%d. " RESET, pastEventsSize - i);
      printf("%s\n", pastEvents[i]);
    }
  } else if (strcmp(args[1], "purge") == 0) {
    if (*argsCount > 2) {
      fprintf(stderr, TCBRED "Invalid arguments after purge\n" RESET);
      return;
    }
    freePastEvents();

    char* historyFilePath = (char*)malloc(sizeof(char) * MAX_PATH_LENGTH + 1);
    getExecutableDir(historyFilePath);
    strcat(historyFilePath, "/");
    strcat(historyFilePath, HISTORY_FILE);
    remove(historyFilePath);

    free(historyFilePath);
  } else if (strcmp(args[1], "execute") == 0) {
    if (*argsCount == 2) {
      fprintf(stderr, TCBRED "Index not specified\n" RESET);
      return;
    } else if (*argsCount > 3) {
      fprintf(stderr,
              TCBRED "Too many arguments. Command is pastevents execute <index>\n" RESET);
      return;
    }

    if (pastEventsSize == 0) {
      fprintf(stderr, TCBRED "No pastevents stored, none can be executed\n" RESET);
      return;
    }

    char* indexStr = args[2];
    char* endPtr;
    int index = strtol(indexStr, &endPtr, 10);
    if (*endPtr != '\0') {
      fprintf(stderr,
              TCBRED "<index> must be an integer in pastevents execute <index>\n" RESET);
      return;
    }

    if (pastEventsSize == 0) {
      fprintf(stderr, TCNYEL "No past events stored." RESET);
      return;
    }

    if (index < 1 || index > pastEventsSize) {
      fprintf(stderr, TCBRED "Invalid index. Index must be in [1, %d]\n" RESET,
              pastEventsSize);
      return;
    }

    index = (pastEventsSize - index);

    char* command = pastEvents[index];
    stripBeginningDelimiters(command);
    stripEndingDelimiters(command);
    
    setPastEvent(command);
    for(int i = 0; i < 3; i++){
      free(args[i]);
    }
    *argsCount = 1;
    stripBeginningDelimiters(command);
    stripEndingDelimiters(command);
    args[0] = strdup(command);
    runCommands(command, false);
  } else {
    fprintf(stderr, TCBRED "Invalid command: " RESET);
    fprintf(
        stderr, TCNRED
        "Expected pastevents, pastevents execute <index> or pastevents purge\n" RESET);
  }
}