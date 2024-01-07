#include "../utils/headers.h"

#define DEFAULT_TIME 1

/*
Idea: https://stackoverflow.com/a/43298520/22348094
*/

void neonate(char* args[], int argsCount) {
  int delay = DEFAULT_TIME;
  ignoreSignals();

  if (!(argsCount == 1 || argsCount == 3)) {
    fprintf(stderr, TCBRED "Invalid usage: " RESET);
    fprintf(stderr, TCNRED "Correct use can be 'neonate' or 'neonate -n <time>'\n" RESET);
    return;
  } else if (argsCount == 3) {
    if (strcmp(args[1], "-n")) {
      fprintf(stderr, TCBRED "Invalid usage: " RESET);
      fprintf(stderr, TCNRED "Correct use can be 'neonate' or 'neonate -n <time>'\n" RESET);
      return;
    }

    char* timeString = args[2];
    char* endPtr;
    delay = strtol(timeString, &endPtr, 10);
    if (*endPtr != '\0') {
      fprintf(stderr, TCBRED "time delay %s is not a non-negative integer\n" RESET,
              timeString);
      return;
    }

    if (delay < 0) {
      fprintf(stderr, TCBRED "Invalid time %d: delay must be a non-negative integer\n" RESET, delay);
      return;
    }
  }

  const char FILE_NAME[] = "/proc/sys/kernel/ns_last_pid";

  printf(TCBMAG "Enter x or X to exit\n" RESET);

  int pid = fork();
  if (pid < 0) {
    perror("fork");
    return;
  } else if (pid == 0) {
    goto printPID;

    time_t startTime = time(NULL);
    while (true) {
      if (time(NULL) - startTime >= delay) {
      printPID:
        int nsLastPid = -1;
        FILE* fd = fopen(FILE_NAME, "r");
        if (fd == NULL) {
          nsLastPid = -1;
        } else {
          if (fscanf(fd, "%d", &nsLastPid) != 1) {
            nsLastPid = -1;
          }
        }
        if (nsLastPid != -1) {
          printf("%d\n", nsLastPid);
        } else {
          printf(TCBRED "Error reading from %s, please check permissions\n" RESET, FILE_NAME);
        }
        startTime = time(NULL);
        fclose(fd);
      }
    }
  } else {
    while (1) {
      setbuf(stdout, NULL);
      if (!enableRawMode()) {
        break;
      }

      char c;
      bool xReceived = false;
      if (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 'x' || c == 'X') {
          xReceived = true;
          break;
        }
      }

      disableRawMode();
      if (xReceived) break;
    }
    kill(pid, SIGTERM);
    printf(TCBYEL "Terminating\n" RESET);
  }

  disableRawMode();
  receiveSignals();
}

void die(const char* s) {
  perror(s);
  exit(1);
}

struct termios orig_termios;

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

/*
https://gist.github.com/schlechter-afk/e4f6df2868ed0ba1d780747535c54d4e
 */
bool enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    perror("tcgetattr");
    return false;
  }
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");

  return true;
}