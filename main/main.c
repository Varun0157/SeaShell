#include "../utils/headers.h"

/**
 * @brief If the current input contains "exit", then true, else false.
 *  Used to determine if the shell should exit.
 */
bool EXIT_ON_NEXT_COMMAND = false;

bool ignoreSignalFlag = false;
void ignoreSignals() { ignoreSignalFlag = true; }
void receiveSignals() { ignoreSignalFlag = false; }

void sigintHandler() {
  if (!ignoreSignalFlag) {
    printf("\n");
    prompt();
    fflush(stdout);
    signal(SIGINT, sigintHandler);
  }
}

void sigtstpHandler() {
  if (!ignoreSignalFlag) {
    printf("\n");
    prompt();
    fflush(stdout);
    signal(SIGINT, sigintHandler);
  }
}

int main() {
  signal(SIGINT, sigintHandler);
  signal(SIGTSTP, sigtstpHandler);

  /*
  Stores the past events in a global variable in pastevents.c
  */
  loadPastEvents();

  while (1) {
    usleep(45000);
    prompt();

    char input[MAX_COMMAND_LEN + 1];

    /*
    ctrl-d input leads to eof => NULL input => exit
    */
    if (!fgets(input, MAX_COMMAND_LEN, stdin)) {
      updatePastEvents("exit");
      handleBackGroundProcesses();
      goto exit;
    }

    /*
    Display any background processes that finished executing
    */
    handleBackGroundProcesses();
    fflush(stdout);

    if (strlen(input) < 1 || input[0] == '\n' || input[0] == '\0') {
      continue;
    }

    /*
    Run the commands given on input
    */
    runCommands(input, true);

    if (EXIT_ON_NEXT_COMMAND) {
    exit:

      fprintf(stderr,
              TCBRED "Logging out, killing background processes and exiting\n" RESET);
      killBackgroundAndExit();
      return 0;
    }
  }

  return 0;
}

void killBackgroundAndExit() {
  killBackGroundProcess();
  freeGlobalData();
}

void freeGlobalData() {
  freePastEvents();
  freeBackGroundProcesses();
  freeDirStores();
  freeFgProcessName();
}

void stripBeginningDelimiters(char* input) {
  int startIndex = 0;
  int len = strlen(input);

  while (startIndex < len && strchr(DELIMITERS, input[startIndex])) {
    startIndex += 1;
  }

  char* newInput = strdup(input + startIndex);
  strcpy(input, newInput);

  free(newInput);
}

void stripEndingDelimiters(char* input) {
  int len = strlen(input);
  int endIndex = len - 1;

  while (endIndex >= 0 && strchr(DELIMITERS, input[endIndex])) {
    endIndex -= 1;
  }

  if (endIndex != len - 1) {
    input[endIndex + 1] = '\0';
  }
}

char* pastEventToExecute = NULL;
void setPastEvent(char* command) { pastEventToExecute = command; }

void runCommands(char* input, bool saveCommand) {
  int inputLen = strlen(input);

  char commandToPrint[MAX_COMMAND_LEN + 1];
  commandToPrint[0] = '\0';

  /*
  Manually closing the input buffer with a null character to ensure
    no unintended errors while printing. Thus, if I am unable to do this,
    I throw an error saying the input is too long.
  */
  if (inputLen >= MAX_COMMAND_LEN) {
    printf(TCBRED "Input is too long. Aborting ...\n" RESET);
    return;
  }
  input[inputLen] = '\0';

  int startIndex = 0;
  int endIndex = 0;

  time_t start_time = time(NULL);

  /*
  Reason for manual parsing instead of strtok in this case:
    - If a command in the input is closed by an &, then I wish to
        store this &, to recognise whether or not a background execution
        is required.
    - If a command in the input is closed by a ;, I do not wish to
        store this ;.
  Thus, a simple strtok with delimiters as "&;" would not work.

  This parsing stores commands by finding bounds such that the command is
    stored in the substring [startIndex, endIndex) of the input command
  */
  // endIndex <= inputLen to allow for the execution of the last command even if
  //   it does not have a closing ; or &. We have ensured it is defined above.
  while (startIndex < inputLen && endIndex <= inputLen) {
    if (input[endIndex] == '&') {
      int commandLen = endIndex - startIndex + 1;
      char command[commandLen + 1];
      strncpy(command, input + startIndex, commandLen);
      command[commandLen] = '\0';

      /*
      Beginning delimiters are stripped to ensure we can make
        a direct comparison with the initial characters with
        "warp", "peek" etc. to find the required command.
      */
      stripBeginningDelimiters(command);

      // This could happen in cases such as input = (something);&
      // <= 1 because I am counting & in my commands
      if (strlen(command) <= 1) {
        startIndex = endIndex + 1;
        endIndex = startIndex;
        continue;
      }

      char commandToShow[MAX_COMMAND_LEN + 1];
      executeCommand(command, commandToShow, true);
      strcat(commandToPrint, commandToShow);

      if (EXIT_ON_NEXT_COMMAND) break;

      startIndex = endIndex + 1;
      endIndex = startIndex;
    } else if (input[endIndex] == ';' || endIndex == inputLen) {
      int commandLen = (endIndex - 1) - startIndex + 1;

      if (commandLen < 1) {
        startIndex = endIndex + 1;
        endIndex = startIndex;
      }

      char command[commandLen + 1];
      strncpy(command, input + startIndex, commandLen);
      command[commandLen] = '\0';

      stripBeginningDelimiters(command);

      // This could happen in cases such as input = (something)&;
      if (strlen(command) < 1) {
        startIndex = endIndex + 1;
        endIndex = startIndex;
        continue;
      }

      char commandToShow[MAX_COMMAND_LEN + 1];
      executeCommand(command, commandToShow, true);
      strcat(commandToPrint, commandToShow);
      strcat(commandToPrint, "; ");

      if (EXIT_ON_NEXT_COMMAND) break;

      startIndex = endIndex + 1;
      endIndex = startIndex;
    } else {
      endIndex++;
    }
  }
  if (saveCommand) {
    long timeInSeconds = time(NULL) - start_time;

    /*
    This is essential for a few reasons:
     - If we do not strip closing \n from input then it could be printed
        in the shell prompt, which would not look clean.
     - Without stripping the opening and closing delimiters, more direct
        strcmp comparisons would fail while updating pastevents.
          Eg: sleep 5(space) would not be the same as sleep 5
    */
    stripBeginningDelimiters(commandToPrint);
    stripEndingDelimiters(commandToPrint);
    int comLen = strlen(commandToPrint);
    if (comLen > 0) {
      if (commandToPrint[comLen - 1] == ';') {
        commandToPrint[comLen - 1] = '\0';
      }
    }
    stripEndingDelimiters(commandToPrint);

    if (timeInSeconds > 2) {
      char detailsToShow[MAX_COMMAND_LEN + 100];

      snprintf(detailsToShow, MAX_COMMAND_LEN + 99, "%s\033[0;31m %lds", commandToPrint,
               timeInSeconds);
      setForegroundProcessDetails(detailsToShow);
    }

    /*
    Updating the pastevents with the new input
    */
    updatePastEvents(commandToPrint);
  }
}

bool needsRedirect(char* input) {
  int index = 0;
  int maxIndex = strlen(input);

  while (index < maxIndex) {
    if (input[index] == '"') {
      int close = findClosingCharOrEndOfWord('"', input, index);
      index = close + 1;
      continue;
    } else if (input[index] == '>' || input[index] == '<') {
      if (input[index] == '<' && index + 1 < maxIndex && input[index + 1] == '<') {
        fprintf(stderr, TCNYEL
                "WARNING: << is not valid redirection, hence, `<` may be treated as a "
                "file\n" RESET);
      }
      return true;
    }
    index += 1;
  }

  return false;
}

bool needsPiping(char* input) {
  int index = 0;
  int maxIndex = strlen(input);

  while (index < maxIndex) {
    if (input[index] == '"') {
      int close = findClosingCharOrEndOfWord('"', input, index);
      index = close + 1;
      continue;
    } else if (input[index] == '|') {
      return true;
    }
    index += 1;
  }

  return false;
}

void executeArgs(char* args[], int* argsCount, char* formattedCommand,
                 bool redirectRequired, bool setFDs, int inputFD, int outputFD,
                 bool pipingRequired, bool saveCommand) {
  int OG_FDS[2];
  OG_FDS[0] = dup(STDIN_FILENO);
  OG_FDS[1] = dup(STDOUT_FILENO);

  if (setFDs) {
    if (inputFD != STDIN_FILENO) {
      dup2(inputFD, STDIN_FILENO);
      close(inputFD);
    }
    if (outputFD != STDOUT_FILENO) {
      dup2(outputFD, STDOUT_FILENO);
      close(outputFD);
    }
  }

  bool isBackgroundProcess = false;
  /*
  If pipes are present, then that becomes the first priority, each pipe command must be
  separately executed first.
  */
  if (pipingRequired) {
    executePipes(args, argsCount);
  } else if (redirectRequired) {
    if (isolateAllRedirects(args, argsCount)) {
      isBackgroundProcess = redirect(args, argsCount);
    }
  } else if (strcmp(args[0], "warp") == 0) {
    warp(args, *argsCount);
  } else if (strcmp(args[0], "peek") == 0) {
    peek(args, *argsCount);
  } else if (strcmp(args[0], "proclore") == 0) {
    proclore(args, *argsCount);
  } else if (strcmp(args[0], "seek") == 0) {
    seek(args, *argsCount);
  } else if (strcmp(args[0], "pastevents") == 0) {
    pastevents(args, argsCount);
  } else if (strcmp(args[0], "exit") == 0) {
    if (*argsCount != 1) {
      fprintf(stderr, TCNYEL "exit is a standalone command. Use as 'exit'\n" RESET);
    } else {
      EXIT_ON_NEXT_COMMAND = true;
    }
  } else if (strcmp(args[0], "activities") == 0) {
    activities(*argsCount);
  } else if (strcmp(args[0], "ping") == 0) {
    sendSignal(args, *argsCount);
  } else if (strcmp(args[0], "fg") == 0) {
    bringToFG(args, *argsCount);
  } else if (strcmp(args[0], "bg") == 0) {
    sendToBG(args, *argsCount);
  } else if (strcmp(args[0], "neonate") == 0) {
    neonate(args, *argsCount);
  } else if (strcmp(args[0], "iMan") == 0) {
    iMan(args, *argsCount);
  } else {
    isBackgroundProcess = makeSystemCall(args, argsCount);
  }

  if (saveCommand) {
    formattedCommand[0] = '\0';
    for (int i = 0; i < *argsCount && args[i] != NULL; i++) {
      strcat(formattedCommand, args[i]);
      strcat(formattedCommand, " ");
    }
    if (isBackgroundProcess) {
      strcat(formattedCommand, "& ");
    }
  }

  dup2(OG_FDS[0], STDIN_FILENO);
  dup2(OG_FDS[1], STDOUT_FILENO);
  close(OG_FDS[0]), close(OG_FDS[1]);
}

void executeCommand(char* input, char* formattedCommand, bool saveFormatted) {
  char* args[MAX_ARGS];
  int argsCount = 0;

  bool hasPipe = needsPiping(input);
  if (hasPipe) {
    /*
    If pipes are present, we just separate the input by the pipes, instead of complete
    tokenisation. i.e. we separate into command1 | command2 | .... Then, we execute the
    individual commands.

    Set the one arg as the input itself. isolatePipes will separate it as required.
    */
    args[argsCount++] = strdup(input);
    stripBeginningDelimiters(input);
    stripEndingDelimiters(input);

    if (isolatePipes(args, &argsCount)) {
      executeArgs(args, &argsCount, formattedCommand, false, false, -1, -1, true,
                  saveFormatted);
    } else {
      if(saveFormatted){
        strncpy(formattedCommand, input, MAX_COMMAND_LEN);
        strcat(formattedCommand, " ");
      }
    }
    goto free;
  }

  if (!tokenizeNoPipes(input, args, &argsCount)) {
    if(saveFormatted){
      strncpy(formattedCommand, input, MAX_COMMAND_LEN);
      strcat(formattedCommand, " ");
    }
    goto free;
  }

  if (argsCount < 1) return;

  /*
  No pipes case
  */
  executeArgs(args, &argsCount, formattedCommand, needsRedirect(input), false, -1, -1,
              false, saveFormatted);

free:
  for (int i = 0; i < argsCount; i++) {
    free(args[i]);
  }

  fflush(stdout);
}

void exitOnNextCommand() { EXIT_ON_NEXT_COMMAND = true; }

int findClosingCharOrEndOfWord(char ch, char* input, int start) {
  int index = start + 1;
  int inputLen = strlen(input);

  while (index < inputLen) {
    if (input[index] == ch) {
      return index;
    }
    index += 1;
  }

  return index;
}

bool tokenizeNoPipes(char* input, char* args[], int* argsCount) {
  stripBeginningDelimiters(input);
  stripEndingDelimiters(input);

  int inputLen = strlen(input);

  int start = 0;
  int end = 0;

  if (strchr(input, '\'')) {
    fprintf(stderr, TCNYEL
            "WARNING: Only double quotes support bash-like execution. Single quotes may "
            "lead to unintended output\n" RESET);
  }

  while (start < inputLen && end < inputLen) {
    if (input[start] == '"') {
      int close = findClosingCharOrEndOfWord('"', input, start);
      if (close >= inputLen) {
        fprintf(stderr, TCBRED "Odd number of double quotes found, invalid\n" RESET);
        return false;
      }

      int tokenLenWithQuotes = close - start + 1;
      int tokenLenWithoutQuotes = tokenLenWithQuotes - 2;
      if (tokenLenWithoutQuotes < 1) {
        goto nextPart;
      }

      char* token = (char*)malloc(sizeof(char) * (tokenLenWithQuotes));
      token[0] = '\0';
      strncpy(token, input + start + 1, tokenLenWithoutQuotes);
      token[tokenLenWithoutQuotes] = '\0';

      if (start > 0 && !strchr(DELIMITERS, input[start - 1]) && *argsCount > 0) {
        args[*argsCount - 1] =
            realloc(args[*argsCount - 1],
                    sizeof(char) * (strlen(args[*argsCount - 1]) + tokenLenWithQuotes));
        strcat(args[*argsCount - 1], token);
        free(token);
      } else {
        args[(*argsCount)++] = token;
      }
    nextPart:
      start = close + 1;
      end = start;
    } else if (strchr(DELIMITERS, input[end]) || input[end] == '"' ||
               end == inputLen - 1) {
      int tokenLen;
      if (end == inputLen - 1)
        tokenLen = end - start + 1;
      else
        tokenLen = (end - 1) - start + 1;

      if (tokenLen < 1) {
        goto nextLoop;
      }

      char* token = (char*)malloc(sizeof(char) * (tokenLen + 1));
      strncpy(token, input + start, tokenLen);
      token[tokenLen] = '\0';

      if (start > 0 && input[start - 1] == '"' && *argsCount > 0) {
        args[*argsCount - 1] =
            realloc(args[*argsCount - 1],
                    sizeof(char) * (strlen(args[*argsCount - 1]) + tokenLen + 1));
        strcat(args[*argsCount - 1], token);
        free(token);
      } else {
        args[(*argsCount)++] = token;
      }

    nextLoop:
      if (input[end] == '"') {
        start = end;  // we will go to the if next
        end = start;
      } else {
        start = end + 1;
        end = start;
      }
    } else {
      end += 1;
    }
  }

  return true;
}