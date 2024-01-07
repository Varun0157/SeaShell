#include "../utils/headers.h"

char* CWD = NULL;
char* OLD_PWD = NULL;

char* additionalDetails = NULL;

void prompt() {
  char userName[MAX_NAME_LENGTH + 1];
  char hostName[MAX_NAME_LENGTH + 1];
  char relPath[MAX_PATH_LENGTH + 1];

  getUserName(userName);
  getHostName(hostName);

  char currentDirPath[MAX_PATH_LENGTH + 1];
  getCurrentDir(currentDirPath);
  getRelativePath(currentDirPath, relPath);

  printf("\n");
  int numBg;
  if ((numBg = getBackgroundProcessCount()) > 0) {
    printf(TCBBLU " *%d " RESET, numBg);
  }
  printf(TCNCYN "<" RESET);
  printf(TCBGRN "%s" RESET, userName);
  printf(TCNMAG "@" RESET);
  printf(TCBGRN "%s" RESET, hostName);
  printf(TCNMAG ":" RESET);
  printf(TCBBLU "%s" RESET, relPath);
  if (additionalDetails != NULL) {
    printf(TCBYEL " %s" RESET, additionalDetails);
    free(additionalDetails);
    additionalDetails = NULL;
  }
  printf(TCNCYN "> " RESET);
}

char* getCWD() { return CWD; }

char* getOldPWD() { return OLD_PWD; }

void freeDirStores() {
  if (CWD) free(CWD);
  if (OLD_PWD) free(OLD_PWD);
}

void storeDirInPath(char* path) {
  int pathLen = strlen(path);
  for (int i = pathLen - 1; i >= 0; i--) {
    if (path[i] == '/') {
      path[i] = '\0';
      return;
    }
  }
}

void setForegroundProcessDetails(char* details) {
  if (additionalDetails) {
    free(additionalDetails);
    additionalDetails = NULL;
  }
  additionalDetails = strdup(details);
}

void getExecutableDir(char* executablePath) {
  // https://stackoverflow.com/questions/933850/how-do-i-find-the-location-of-the-executable-in-c
  char symPathToExecutable[MAX_PATH_LENGTH + 1];
  if (readlink("/proc/self/exe", symPathToExecutable, MAX_PATH_LENGTH + 1) < 0) {
    perror(TCBRED "Error reading path of executable" TCNRED);
    fprintf(stderr,
            "Ensure the user has appropriate access to read /proc/self/exe for proper "
            "working of the shell\n" RESET);
    return;
  }

  // https://stackoverflow.com/questions/1563168/example-of-realpath-function-in-c
  realpath(symPathToExecutable, executablePath);
  storeDirInPath(executablePath);
}

void getCurrentDir(char* currentDirPath) {
  if (getcwd(currentDirPath, MAX_PATH_LENGTH + 1) == NULL) {
    perror(TCBRED "Error getting current working directory" TCNRED);
    fprintf(stderr, RESET);
    return;
  }

  setCWD(currentDirPath);
}

void setCWD(char* currentDirPath) {
  // updating CWD and OLD_PWD
  if (CWD == NULL) {
    CWD = strdup(currentDirPath);
    return;
  }

  if (strcmp(currentDirPath, CWD) != 0) {
    if (OLD_PWD != NULL) free(OLD_PWD);

    OLD_PWD = strdup(CWD);

    free(CWD);
    CWD = strdup(currentDirPath);
  }
}

void getRelativePath(char* path, char* relativePath) {
  char executablePath[MAX_PATH_LENGTH + 1];
  getExecutableDir(executablePath);

  // now, getting the path itself
  if (strncmp(path, executablePath, strlen(executablePath)) == 0) {
    char* relPath = path + strlen(executablePath);

    strcpy(relativePath, "~");
    strcat(relativePath, relPath);
  } else {
    strcpy(relativePath, path);
  }
}

/*
Source:
https://stackoverflow.com/questions/8953424/how-to-get-the-username-in-c-c-in-linux
*/
void getUserName(char* userName) {
  struct passwd* pw;
  pw = getpwuid(geteuid());

  if (pw) {
    strcpy(userName, pw->pw_name);
  } else {
    strcpy(userName, "username");
  }
}

/*
Source: https://www.systutorials.com/how-to-get-the-hostname-of-the-node-in-c/
*/
void getHostName(char* hostName) { gethostname(hostName, MAX_NAME_LENGTH + 1); }