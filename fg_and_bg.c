#include "headers.h"

void bringToFG(char* args[], int argsCount) {
  if (argsCount != 2) {
    fprintf(stderr, TCBRED "Incorrect use of fd: " RESET);
    fprintf(stderr, TCNRED "Correct use: fg <pid>\n" RESET);
    return;
  }

  char* pidString = args[1];
  char* endPtr;
  int PID = (int)strtol(pidString, &endPtr, 10);
  if (*endPtr != '\0') {
    fprintf(stderr, TCBRED "PID %s given in fg is not a number\n" RESET, pidString);
    return;
  }

  if (PID < 1) {
    fprintf(stderr, TCBRED "PID %d given in fg is not a positive number\n" RESET, PID);
    return;
  }

  /*
  https://stackoverflow.com/a/31931126/22348094
  */
  if (getpgid(PID) >= 0) {
    setFgProcessPID(PID);
    setFgProcessName(args[0]);

    /*
    This section is a repeat from sysCommands
    Allow child process to take input and output without shutting down everything
    */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    /*
    Associates terminal foreground group represented by STDIN to that of the new PID.
    Essentially, allows the PID to take control.
    */
    tcsetpgrp(STDIN_FILENO, PID);

    if (kill(PID, SIGCONT) < 0) {
      perror("kill");
      setFgProcessPID(-1);
      goto getControl;
    }

    char* name = getProcessName(PID);
    printf(TCNGRN "Brought %d (%s) to the foreground\n" RESET, PID, name);
    removeBackGroundProcess(PID);

    int status;
    waitpid(PID, &status, WUNTRACED);
    if (WEXITSTATUS(status) == SIGTSTP) {
      sendToBackground(PID, name);
    }  // while debugging noticed that ctrl+z returns status 5247
    free(name);

  getControl:
    /*
    Regains control
    */
    tcsetpgrp(STDIN_FILENO, getpgid(STDIN_FILENO));
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);

    setFgProcessPID(-1);
  } else {
    fprintf(stderr, TCBRED "No process with pid %d found\n" RESET, PID);
  }
}

void sendToBG(char* args[], int argsCount) {
  /*
  Check if the given pid is in your bg list first, instead of getpgid >= 0
  or, make sure it's a fg process.
  */
  if (argsCount != 2) {
    fprintf(stderr, TCBRED "Incorrect use of fd: " RESET);
    fprintf(stderr, TCNRED "Correct use: fg <pid>\n" RESET);
    return;
  }

  char* pidString = args[1];
  char* endPtr;
  int PID = (int)strtol(pidString, &endPtr, 10);
  if (*endPtr != '\0') {
    fprintf(stderr, TCBRED "PID %s given in bg is not a number\n" RESET, pidString);
    return;
  }

  if (PID < 1) {
    fprintf(stderr, TCBRED "PID %d given in bg is not a positive number\n" RESET, PID);
    return;
  }

  if (getpgid(PID) >= 0) {
    kill(PID, SIGCONT);
    char* name = getProcessName(PID);
    sendToBackground(PID, name);
    printf(TCNGRN "%d (%s) given signal SIGCONT\n" RESET, PID, name);
    free(name);
  } else {
    fprintf(stderr, TCBRED "No process with pid %d found\n" RESET, PID);
  }
}

char* getProcessName(int pid) {
  char procPath[MAX_NAME_LENGTH];
  snprintf(procPath, MAX_NAME_LENGTH, "/proc/%d/comm", pid);

  FILE* fd = fopen(procPath, "r");
  char* processName = (char*)malloc(sizeof(char) * (MAX_NAME_LENGTH + 1));
  if (fgets(processName, MAX_NAME_LENGTH, fd) == NULL) {
    goto nullCase;
  }

  int len = strlen(processName);
  if (len < 1) goto nullCase;

  if (processName[len - 1] == '\n')
    processName[len - 1] = '\0';
  else
    processName[len] = '\0';

  fclose(fd);

  return processName;

nullCase:
  snprintf(processName, MAX_NAME_LENGTH, "%d", pid);
  processName[strlen(processName) - 1] = '\0';

  return processName;
}