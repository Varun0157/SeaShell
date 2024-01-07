#include "../utils/headers.h"

void warp(char* args[], int argsCount) {
  char homeDir[MAX_PATH_LENGTH + 1];
  getExecutableDir(homeDir);

  if (argsCount == 1) {
    if (chdir(homeDir)) {
      perror(TCBRED "Error going to home directory" RESET);
      return;
    } else {
      printf("%s\n", homeDir);
      setCWD(homeDir);
    }
    return;
  }

  char* newDir = (char*)malloc(sizeof(char) * (MAX_PATH_LENGTH + 1));
  for (int i = 1; i < argsCount; i++) {
    char* arg = args[i];
    getCurrentDir(newDir);
    if (arg[0] == '~') {
      strcpy(newDir, homeDir);
      strcat(newDir, arg + 1);
    } else if (strcmp(arg, "-") == 0) {
      char* oldPWD = getOldPWD();
      if (oldPWD == NULL) {
        fprintf(stderr, TCNYEL "OLDPWD not set\n" RESET);
        continue;
      } else {
        strcpy(newDir, oldPWD);
      }
    } else {
      strcpy(newDir, arg);
    }

    if (chdir(newDir)) {
      perror(TCBRED "Error changing to dir" TCNRED);
      fprintf(stderr, "Dir path provided: %s\n" RESET, newDir);
      continue;
    } else {
      getCurrentDir(newDir);
      printf("%s\n", newDir);
      setCWD(newDir);
    }
  }

  free(newDir);
}