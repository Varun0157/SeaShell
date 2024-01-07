#include "headers.h"

/*
Important source:
https://www.geeksforgeeks.org/exit-status-child-process-linux/
*/

int fgProcessPID = -1;
void setFgProcessPID(int newPid) { fgProcessPID = newPid; }

char* fgProcessName = NULL;
void setFgProcessName(char* name) {
  freeFgProcessName();
  fgProcessName = strdup(name);
}

int getFgProcessPID() { return fgProcessPID; }
char* getFgProcessName() { return fgProcessName; }

void freeFgProcessName() {
  if (fgProcessName) {
    free(fgProcessName);
    fgProcessName = NULL;
  }
}

processDetails backgroundProcesses[MAX_ARGS];
int backgroundProcessCount = 0;

void addBackgroundProcess(int pid, char* name) {
  for (int i = 0; i < backgroundProcessCount; i++) {
    if (pid == backgroundProcesses[i].pid) {
      return;
    }
  }

  if (backgroundProcessCount >= MAX_ARGS) {
    fprintf(stderr, TCNYEL
            "Limit of allowed background processes reached, this process will execute, "
            "but cannot be tracked." RESET);
  }
  backgroundProcesses[backgroundProcessCount++] =
      (processDetails){.pid = pid, .name = strdup(name)};
  printf(TCNCYN "[" RESET);
  printf(TCBBLU "%d" RESET, backgroundProcessCount);
  printf(TCNCYN "]" RESET);
  printf(TCBMAG " %d\n" RESET, pid);
}

void removeBackGroundProcess(int pid) {
  for (int i = 0; i < backgroundProcessCount; i++) {
    if (backgroundProcesses[i].pid == pid) {
      free(backgroundProcesses[i].name);
      for (int j = i; j < backgroundProcessCount - 1; j++) {
        backgroundProcesses[j] = backgroundProcesses[j + 1];
      }
      backgroundProcessCount -= 1;
      return;
    }
  }
}

processDetails* getBackgroundProcesses() { return backgroundProcesses; }
int getBackgroundProcessCount() { return backgroundProcessCount; }

void freeBackGroundProcesses() {
  for (int i = 0; i < backgroundProcessCount; i++) {
    free(backgroundProcesses[i].name);
  }
  backgroundProcessCount = 0;
}

void killBackGroundProcess() {
  for (int i = 0; i < backgroundProcessCount; i++) {
    int result = kill(backgroundProcesses[i].pid, SIGTERM);
    if (result == 0) {
      printf(TCNGRN "Background process %s (%d) killed succesfully. \n" RESET,
             backgroundProcesses[i].name, backgroundProcesses[i].pid);
    } else {
      printf(TCNYEL "Error killing background process %s (%d). \n" RESET,
             backgroundProcesses[i].name, backgroundProcesses[i].pid);
    }
  }
}

void handleBackGroundProcesses() {
  for (int i = 0; i < backgroundProcessCount; i++) {
    int pid = backgroundProcesses[i].pid;
    char* name = backgroundProcesses[i].name;
    int status;

    if (waitpid(pid, &status, WNOHANG) > 0) {
      if (WIFEXITED(status)) {
        int exitStatus = WEXITSTATUS(status);
        if (exitStatus == 0) {
          printf(TCNGRN "%s exited normally (%d)\n" RESET, name, pid);
        } else {
          printf(TCNYEL "%s exited with status %d (%d)\n" RESET, name, exitStatus, pid);
        }
      } else {
        printf(TCNCYN "%s exited with status %d (%d)\n" RESET, name, status, pid);
      }

      free(name);

      for (int j = i; j < backgroundProcessCount - 1; j++) {
        backgroundProcesses[j] = backgroundProcesses[j + 1];
      }
      backgroundProcessCount--;
      i--;
    }
  }
}

void executeSysCommand(char* args[], bool runInBackground) {
  int pid = fork();
  if (pid < 0) {
    perror("fork");
    return;
  } else if (pid == 0) {
    setpgid(0, 0);

    execvp(args[0], args);

    if (errno == ENOENT) {
      fprintf(stderr, TCBRED "%s: command not found\n" RESET, args[0]);
      exit(0);
    }
    perror(TCBRED "Error executing process" RESET);
    fprintf(stderr, RESET);

    exit(1);
  } else {
    if (!runInBackground) {
      fgProcessPID = pid;
      freeFgProcessName();
      fgProcessName = strdup(args[0]);
      /*
      Allow child process to take input and output without shutting down everything
      */
      signal(SIGTTOU, SIG_IGN);
      signal(SIGTTIN, SIG_IGN);
      /*
    Associates terminal foreground group represented by STDIN to that of the new PID.
    Essentially, allows the PID to take control.
    */
      tcsetpgrp(STDIN_FILENO, pid); /* Give control of the terminal */

      int status;
      waitpid(pid, &status, WUNTRACED);
      if (WEXITSTATUS(status) == SIGTSTP) {
        sendToBackground(pid, fgProcessName);
      }

      tcsetpgrp(STDIN_FILENO, getpgid(STDIN_FILENO)); /* Take control back */
      signal(SIGTTOU, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);

      fgProcessPID = -1;
    } else {
      addBackgroundProcess(pid, args[0]);
    }
  }
}

bool makeSystemCall(char* args[], int* argsCount) {
  bool runInBackground = false;

  if (*argsCount < 1) {
    return false;
  } else {
    char* lastArg = args[*argsCount - 1];
    int len = strlen(lastArg);
    if (strcmp(lastArg, "&") == 0) {
      runInBackground = true;
      free(lastArg);
      args[--(*argsCount)] = NULL;
    } else if (lastArg[len - 1] == '&') {
      runInBackground = true;
      args[*argsCount - 1][len - 1] = '\0';
    }
  }

  if (*argsCount < 1) {
    return false;
  }

  char* argsToPass[*argsCount + 1];
  for (int i = 0; i < *argsCount; i++) {
    argsToPass[i] = args[i];
  }
  argsToPass[*argsCount] = NULL;

  executeSysCommand(argsToPass, runInBackground);

  return runInBackground;
}
