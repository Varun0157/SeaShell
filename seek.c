/*
Defining the file as GNU_SOURCE seems to to be essential to use alphasort()

https://stackoverflow.com/questions/19439855/implicit-declaration-of-scandir-alphasort-is-undeclared
*/
#define _GNU_SOURCE

#include "headers.h"

void seek(char* args[], int argsCount) {
  if (argsCount < 1) {
    fprintf(stderr, TCNRED "No arguments provided\n" RESET);
    return;
  }

  // getting flags
  bool eFlag = false, dFlag = false, fFlag = false;
  int argIndex = 1;
  while (argIndex < argsCount) {
    char* currArg = args[argIndex];
    int argLen = strlen(currArg);
    if (currArg[0] == '-' && argLen > 1) {
      // flag identified
      for (int i = 1; i < argLen; i++) {
        char flagVal = currArg[i];
        if (flagVal == 'd') {
          dFlag = true;
        } else if (flagVal == 'e') {
          eFlag = true;
        } else if (flagVal == 'f') {
          fFlag = true;
        } else {
          fprintf(stderr, TCBRED "Invalid flags! " RESET);
          fprintf(stderr, TCNRED "Flags must be any permutation of d, e and f\n" RESET);

          return;
        }
      }
    } else {
      break;
    }
    argIndex++;
  }

  if (dFlag && fFlag) {
    fprintf(stderr, TCBRED "Invalid flags! " RESET);
    fprintf(stderr, TCNRED "d and f flags cannot be utilised together\n" RESET);

    return;
  }

  if (argIndex >= argsCount) {
    fprintf(stderr, TCBRED "No search item provided\n" RESET);
    return;
  }

  int argsLeft = argsCount - argIndex;
  if (argsLeft > 2) {
    fprintf(stderr, TCBRED
            "Too many arguments provided. Please provide seek <flags> <search> "
            "<targetDir>\n" RESET);
    return;
  }

  char* search = args[argIndex++];
  char* searchDir = (argIndex < argsCount) ? args[argIndex] : NULL;
  char* finalPath;

  if (searchDir == NULL) {
    const char* CWD = getCWD();
    if (CWD == NULL) {
      fprintf(stderr,
              TCBRED "Unexpected Error: cannot find current working directory\n" RESET);
      fprintf(stderr, TCNRED "A restart is suggested\n" RESET);
      return;
    }
    finalPath = strdup(CWD);
  } else if (searchDir[0] == '~') {
    finalPath = (char*)malloc(sizeof(char) * (MAX_PATH_LENGTH + 1));
    getExecutableDir(finalPath);
    strcat(finalPath, searchDir + 1);
  } else {
    finalPath = strdup(searchDir);
  }

  bool atleastOne = false;
  if (!eFlag) {
    seekItems(dFlag, fFlag, eFlag, search, finalPath, finalPath, &atleastOne);
  } else {
    int numMatches = eFlagValid(dFlag, fFlag, search, finalPath);
    if (numMatches == 1) {
      seekItems(dFlag, fFlag, eFlag, search, finalPath, finalPath, &atleastOne);
    } else if (numMatches > 1) {
      fprintf(stderr, TCNRED "Too many matches found. e flag is not valid\n" RESET);
      free(finalPath);
      return;
    }
  }
  if (!atleastOne) {
    fprintf(stderr, TCBYEL "No match found!\n" RESET);
  }

  free(finalPath);
}

void getRelativeToGivenDir(char* path, char* relativeTo, char* relativePath) {
  if (strncmp(path, relativeTo, strlen(relativeTo)) == 0) {
    char* relPath = path + strlen(relativeTo);

    strcpy(relativePath, ".");
    strcat(relativePath, relPath);
  } else {
    strcpy(relativePath, path);
  }
}

void seekItems(bool dFlag, bool fFlag, bool eFlag, char* search, char* path,
               char* sourcePath, bool* atleastOne) {
  bool findAll = !dFlag && !fFlag;

  struct dirent** nameList;
  int n = scandir(path, &nameList, NULL, alphasort);

  if (n < 0) {
    char errorMessage[MAX_NAME_LENGTH + 1];
    snprintf(errorMessage, MAX_NAME_LENGTH, TCBRED "Error scanning %s" TCNRED, path);
    perror(errorMessage);
    fprintf(stderr, RESET);
    return;
  } else {
    for (int i = 0; i < n; i++) {
      struct stat st;
      char* itemName = nameList[i]->d_name;

      char* fullPath = strdup(path);
      fullPath = realloc(fullPath, sizeof(char) * (MAX_NAME_LENGTH + 1));
      strcat(fullPath, "/");
      strcat(fullPath, itemName);

      char relPath[MAX_NAME_LENGTH + 1];
      getRelativeToGivenDir(fullPath, sourcePath, relPath);

      if (lstat(fullPath, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
          if ((dFlag || findAll) && strncmp(search, itemName, strlen(search)) == 0) {
            printf(TCNBLU "%s\n" RESET, relPath);
            *atleastOne = true;
            if (eFlag) {
              if (access(fullPath, X_OK) != 0) {
                fprintf(stderr,
                        TCBRED "Missing access permissions for directory %s\n" RESET,
                        itemName);

                for (int i = 0; i < n; i++) {
                  free(nameList[i]);
                }
                free(nameList);
                free(fullPath);
                return;
              }
              char cwdPath[MAX_PATH_LENGTH + 1];
              realpath(fullPath, cwdPath);

              if (chdir(fullPath)) {
                perror(TCBRED "chdir" TCNRED);
                fprintf(stderr, RESET);
              } else {
                setCWD(cwdPath);
              }

              for (int i = 0; i < n; i++) {
                free(nameList[i]);
              }
              free(nameList);
              free(fullPath);
              return;  // e needs only one match anyway
            }
          }
          if (strcmp(itemName, ".") && strcmp(itemName, "..")) {
            if (access(fullPath, X_OK) != 0) {
              fprintf(stderr, TCBYEL "Warning: ");
              fprintf(
                  stderr,
                  TCNYEL
                  "cannot search sub-directories inside directory \"%s\", missing access "
                  "permissions\n" RESET,
                  itemName);
            }
            seekItems(dFlag, fFlag, eFlag, search, fullPath, sourcePath, atleastOne);
          }
        } else if (S_ISREG(st.st_mode) && (fFlag || findAll) &&
                   strncmp(search, itemName, strlen(search)) == 0) {
          printf(TCNGRN "%s\n" RESET, relPath);
          *atleastOne = true;
          if (eFlag) {
            if (access(path, R_OK) != 0) {
              fprintf(stderr, TCBRED "Missing access permissions for file \"%s\"\n" RESET,
                      itemName);
              free(fullPath);
              return;
            }

            int fd = open(fullPath, O_RDONLY);
            if (fd == -1) {
              perror(TCBRED "Error opening file to read" TCNRED);
              fprintf(stderr, RESET);

              for (int i = 0; i < n; i++) {
                free(nameList[i]);
              }
              free(nameList);
              free(fullPath);
              return;
            }

            int charsRead;
            char buffer[MAX_NAME_LENGTH + 1];

            while ((charsRead = read(fd, buffer, MAX_NAME_LENGTH)) > 0) {
              buffer[charsRead] = '\0';
              printf("%s", buffer);
            }
            printf("\n");
            if (charsRead < 0) {
              fprintf(stderr, TCBRED "Error reading from file" RESET);
            }

            close(fd);

            for (int i = 0; i < n; i++) {
              free(nameList[i]);
            }
            free(nameList);
            free(fullPath);
            return;
          }
        }
      }

      free(fullPath);
    }
  }

  for (int i = 0; i < n; i++) {
    free(nameList[i]);
  }
  free(nameList);
}

void numMatchesChecker(bool dFlag, bool fFlag, char* search, char* path,
                       int* numMatches) {
  if (*numMatches > 1) return;
  bool findAll = !dFlag && !fFlag;

  struct dirent** nameList;
  int n = scandir(path, &nameList, NULL, alphasort);
  if (n < 0) {
    goto free;
  } else {
    for (int i = 0; i < n; i++) {
      struct stat st;
      char* itemName = nameList[i]->d_name;

      char* fullPath = strdup(path);
      fullPath = realloc(fullPath, sizeof(char) * (MAX_NAME_LENGTH + 1));
      strcat(fullPath, "/");
      strcat(fullPath, itemName);

      if (lstat(fullPath, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
          if ((dFlag || findAll) && strncmp(search, itemName, strlen(search)) == 0) {
            *numMatches += 1;
            if (*numMatches > 1) {
              for (int i = 0; i < n; i++) {
                free(nameList[i]);
              }
              free(nameList);
              return;
            }
          }
          if (strcmp(itemName, ".") && strcmp(itemName, "..")) {
            numMatchesChecker(dFlag, fFlag, search, fullPath, numMatches);
          }
        } else if (S_ISREG(st.st_mode) && (fFlag || findAll) &&
                   strncmp(search, itemName, strlen(search)) == 0) {
          *numMatches += 1;
          if (*numMatches > 1) {
            for (int i = 0; i < n; i++) {
              free(nameList[i]);
            }
            free(nameList);
            return;
          }
        }
      }

      free(fullPath);
    }
  }

free:
  for (int i = 0; i < n; i++) {
    free(nameList[i]);
  }
  free(nameList);
}

int eFlagValid(bool dFlag, bool fFlag, char* search, char* path) {
  int numMatches = 0;
  numMatchesChecker(dFlag, fFlag, search, path, &numMatches);

  return numMatches;
}