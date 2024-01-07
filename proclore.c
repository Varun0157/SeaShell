#include "headers.h"

void proclore(char* args[], int argsCount) {
  int PID;
  if (argsCount > 1) {
    for (int i = 1; i < argsCount; i++) {
      char* pidString = strdup(args[i]);

      char* endPtr;
      PID = (int)strtol(pidString, &endPtr, 10);
      if (*endPtr != '\0') {
        fprintf(stderr, TCBRED "PID %s is not a number\n" RESET, pidString);
        free(pidString);
        continue;
      }
      free(pidString);

      getDetails(PID);
      if (argsCount > 2 && (i + 1 != argsCount)) printf("\n");
    }
  } else {
    PID = getpid();
    getDetails(PID);
  }
}

void getDetails(int pid) {
  char statusPath[MAX_NAME_LENGTH + 1];
  snprintf(statusPath, MAX_NAME_LENGTH + 1, "/proc/%d/status", pid);

  char status;
  char VmSize[MAX_PATH_LENGTH + 1];
  VmSize[0] = '0';
  VmSize[1] = '\0';

  FILE* statusFile = fopen(statusPath, "r");
  if (!statusFile) {
    fprintf(stderr, TCBRED "Error opening file for PID %d\n" RESET, pid);
    fprintf(
        stderr,
        TCNRED
        "Please ensure the process is running and that the user has appropriate access "
        "to /proc/%d/status\n" RESET,
        pid);
    return;
  }

  char buffer[MAX_NAME_LENGTH + 1];
  while (fgets(buffer, MAX_NAME_LENGTH, statusFile)) {
    if (strncmp(buffer, "State:", 6) == 0) {
      sscanf(buffer, "State:\t%c", &status);
    } else if (strncmp(buffer, "VmSize:", 7) == 0) {
      sscanf(buffer, "VmSize:\t%s", VmSize);
    }
  }
  fclose(statusFile);

  int groupId = getpgid(pid);
  int tGroupId = tcgetpgrp(
      STDIN_FILENO);  // get  process id of the terminal associated with STDIN (0)

  printf("pid:");
  printf(TCBMAG "\t\t\t%d\n" RESET, pid);

  printf("Process status:");
  printf(TCBMAG "\t\t%c", status);
  if (groupId == tGroupId) printf("+");
  printf("\n" RESET);

  printf("Process Group:");
  printf(TCBMAG "\t\t%d\n" RESET, groupId);

  printf("Virtual Memory:");
  printf(TCBMAG "\t\t%s KB\n" RESET, VmSize);

  char symPathToExecutable[MAX_PATH_LENGTH + 1];
  snprintf(statusPath, MAX_NAME_LENGTH + 1, "/proc/%d/exe", pid);
  int numRead;
  if ((numRead = readlink(statusPath, symPathToExecutable, MAX_PATH_LENGTH)) < 0) {
    char errorMessage[MAX_NAME_LENGTH + 1];
    snprintf(errorMessage, MAX_NAME_LENGTH,
             TCBRED "Error finding exe file of PID %d" TCNRED, pid);
    perror(errorMessage);
    fprintf(stderr,
            TCNRED
            "Ensure the process is running and the user has appropriate access to "
            "/proc/%d/exe\n" RESET,
            pid);
    return;
  }

  symPathToExecutable[numRead] = '\0';
  char exePath[MAX_PATH_LENGTH + 1];
  realpath(symPathToExecutable, exePath);

  char relPath[MAX_PATH_LENGTH + 1];
  getRelativePath(exePath, relPath);
  printf("Executable path:");
  printf(TCBMAG "\t%s" RESET, relPath);
  printf("\n");
}
