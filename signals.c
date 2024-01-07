#include "headers.h"

void sendToBackground(int pid, char* name) {
  if (pid != -1) {
    addBackgroundProcess(pid, name);
    freeFgProcessName();
  }
}

void sendSignal(char* args[], int argsCount) {
  if (argsCount != 3) {
    fprintf(stderr, TCBRED
            "Invalid arguments. ping must be called as ping <pid> <signal>\n" RESET);
    return;
  }

  char* pidString = args[1];
  char* signalString = args[2];

  char* endPtr;
  int PID = (int)strtol(pidString, &endPtr, 10);
  if (*endPtr != '\0') {
    fprintf(stderr, TCBRED "PID %s given in ping is not a number\n" RESET, pidString);
    return;
  }
  long signalLong = strtol(signalString, &endPtr, 10);
  if (*endPtr != '\0') {
    fprintf(stderr, TCBRED "Signal %s specified in ping is not a number\n" RESET,
            signalString);
  }

  int signal = ((signalLong % 32) + 32) % 32;  // stated to do this in document
  if (getpgid(PID) >= 0) {
    kill(PID, signal);
    char* name = getProcessName(PID);
    printf(TCNGRN "Sent signal %d to process %s with PID %d\n" RESET, signal, name, PID);
    free(name);
  } else {
    fprintf(stderr, TCBRED "No process with pid %d found\n" RESET, PID);
  }
}