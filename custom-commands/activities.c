#include "../utils/headers.h"

int compareProcesses(const void* a, const void* b) {
  return ((processDetails*)a)->pid - ((processDetails*)b)->pid;
}

char getState(int pid) {
  char statusPath[MAX_NAME_LENGTH + 1];
  snprintf(statusPath, MAX_NAME_LENGTH + 1, "/proc/%d/status", pid);

  FILE* statusFile = fopen(statusPath, "r");
  if (!statusFile) {
    return '-';
  }

  char buffer[MAX_NAME_LENGTH + 1];
  while (fgets(buffer, MAX_NAME_LENGTH, statusFile)) {
    if (strncmp(buffer, "State:", 6) == 0) {
      char status;
      sscanf(buffer, "State:\t%c", &status);

      return status;
    }
  }
  return '-';
}

void activities(int argsCount) {
  if (argsCount > 1) {
    fprintf(stderr, TCBRED "Too many args after activities\n" RESET);
    fprintf(stderr, TCNRED "Usage: `activities`\n" RESET);
    return;
  }

  int bgCount = getBackgroundProcessCount();
  processDetails* backgroundProcesses = getBackgroundProcesses();

  if (bgCount <= 0) {
    fprintf(stderr, TCNYEL "No background activities currently present\n" RESET);
    return;
  }

  processDetails bgProcesses[bgCount];
  for (int i = 0; i < bgCount; i++) {
    bgProcesses[i] = backgroundProcesses[i];
  }

  qsort(bgProcesses, bgCount, sizeof(processDetails), compareProcesses);

  printf("PID");
  printf("  ");
  printf("\tNAME\t");
  printf("STATE\n");

  for (int i = 0; i < bgCount; i++) {
    char state = getState(bgProcesses[i].pid);
    if (state == '-')
      continue;  // this usually means that the proc file couldn't be accessed, likely
                 // because the process was killed. Ideally, this will never happen
                 // because the process will leave the background process list in this
                 // case.

    printf(TCBMAG "%d", bgProcesses[i].pid);
    printf(TCNBLU ": ");
    printf(TCNCYN "\t%s\t", bgProcesses[i].name);

    if (state == 'T') {
      printf(TCBRED "Stopped\n" RESET);
    } else if (state == 'R') {
      printf(TCBGRN "Running\n" RESET);
    } else if (state == 'S') {
      printf(TCBGRN "Sleeping\n" RESET);
    } else if (state == 'D') {
      printf(TCBBLU "Disk Sleep\n" RESET);
    } else if (state == 'Z') {
      printf(TCBBLU "Zombie\n" RESET);
    } else if (state == 'X') {
      printf(TCBBLU "Dead\n" RESET);
    } else if (state == 't') {
      printf(TCBBLU "Tracing Stop\n" RESET);
    } else {
      printf(TCBBLU "-\n" RESET);
    }
  }
}
