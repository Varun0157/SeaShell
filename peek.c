/*
Defining the file as GNU_SOURCE seems to to be essential to use alphasort()

https://stackoverflow.com/questions/19439855/implicit-declaration-of-scandir-alphasort-is-undeclared
*/
#define _GNU_SOURCE

#include "headers.h"

void peek(char* args[], int argsCount) {
  char* pathArg = NULL;

  // getting flags
  bool aFlag = false, lFlag = false;
  int argIndex = 1;
  while (argIndex < argsCount) {
    char* currArg = args[argIndex];
    int argLen = strlen(currArg);

    if (currArg[0] == '-' && argLen > 1) {
      // flag identified
      for (int i = 1; i < argLen; i++) {
        char flagVal = currArg[i];
        if (flagVal == 'a') {
          aFlag = true;
        } else if (flagVal == 'l') {
          lFlag = true;
        } else {
          fprintf(stderr, TCBRED "Invalid flags! " RESET);
          fprintf(stderr, TCNRED "Flags must be any permutation of a and l\n" RESET);

          return;
        }
      }
    } else {
      break;
    }
    argIndex++;
  }

  if (argIndex < argsCount - 1) {
    fprintf(stderr, TCBRED "Too many arguments specified! " RESET);
    fprintf(stderr, TCNRED "Input must be of the form peek <flags> <path/name>\n" RESET);

    return;
  }

  if (argIndex == argsCount - 1) {
    pathArg = args[argsCount - 1];
  } else {
    pathArg = NULL;
  }

  char* finalPath;
  if (pathArg == NULL) {
    const char* CWD = getCWD();
    if (CWD == NULL) {
      fprintf(stderr,
              TCBRED "Unexpected Error: cannot find current working directory\n" RESET);
      fprintf(stderr, TCNRED "A restart is suggested\n" RESET);
      return;
    }
    finalPath = strdup(CWD);
  } else if (pathArg[0] == '~') {
    finalPath = (char*)malloc(sizeof(char) * (MAX_PATH_LENGTH + 1));
    getExecutableDir(finalPath);

    strcat(finalPath, pathArg + 1);
  } else {
    finalPath = strdup(pathArg);
  }

  peekDirectory(finalPath, aFlag, lFlag);

  free(finalPath);
}

bool fileIsHidden(char* fileName) { return fileName[0] == '.'; }

char* getFileDetails(char* name, struct stat st) {
  /*
  Setting default values, '-' to all permissions
  */
  char permissions[11];
  for (int i = 0; i < 10; i++) permissions[i] = '-';
  permissions[10] = '\0';

  if (S_ISDIR(st.st_mode))
    permissions[0] = 'd';
  else if (S_ISLNK(st.st_mode))
    permissions[0] = 'l';
  else if (S_ISCHR(st.st_mode))
    permissions[0] = 'c';
  else if (S_ISBLK(st.st_mode))
    permissions[0] = 'b';

  if (st.st_mode & S_IRUSR) permissions[1] = 'r';
  if (st.st_mode & S_IWUSR) permissions[2] = 'w';
  if (st.st_mode & S_IXUSR) permissions[3] = 'x';
  if (st.st_mode & S_IRGRP) permissions[4] = 'r';
  if (st.st_mode & S_IWGRP) permissions[5] = 'w';
  if (st.st_mode & S_IXGRP) permissions[6] = 'x';
  if (st.st_mode & S_IROTH) permissions[7] = 'r';
  if (st.st_mode & S_IWOTH) permissions[8] = 'w';
  if (st.st_mode & S_IXOTH) permissions[9] = 'x';

  const unsigned long numHardLinks = st.st_nlink;

  char userName[MAX_NAME_LENGTH + 1];
  getUserName(userName);

  struct group* gr = getgrgid(st.st_gid);
  const char* groupName = gr->gr_name;

  const long fileSize = st.st_size;

  char date[100];
  strftime(date, sizeof(date), "%b %d %H: %M", localtime(&st.st_mtime));

  char* ALL_DETAILS = (char*)malloc(sizeof(char) * (2 * MAX_NAME_LENGTH + 1));
  snprintf(ALL_DETAILS, 2 * MAX_NAME_LENGTH + 1, "%s %lu %s %s %ld\t%s %s", permissions,
           numHardLinks, userName, groupName, fileSize, date, name);

  return ALL_DETAILS;
}

void peekDirectory(char* path, bool aFlag, bool lFlag) {
  bool atleastOne = false;
  if (lFlag) {
    printf("total %d\n", getTotalBlocks(path, aFlag));
  }

  struct dirent** nameList;

  /*
  For scandir the following was helpful:
    https://stackoverflow.com/a/25675738/22348094

  At first I was using readdir, but the doubts document mentioned alphasort so
    I googled it and found this.
  */
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

      lstat(fullPath, &st);
      char* itemDetails;
      if (lFlag) {
        itemDetails = getFileDetails(itemName, st);
      } else {
        itemDetails = strdup(itemName);
      }
      if (!fileIsHidden(itemName) || (aFlag && fileIsHidden(itemName))) {
        if (S_ISDIR(st.st_mode)) {
          printf(TCNBLU "%s\n" RESET, itemDetails);
        } else if (st.st_mode & S_IXUSR) {
          printf(TCNGRN "%s\n" RESET, itemDetails);
        } else {
          printf(TCNWHT "%s\n" RESET, itemDetails);
        }
        atleastOne = true;
      }

      free(itemDetails);
      free(fullPath);
    }
  }

  for (int i = 0; i < n; i++) {
    free(nameList[i]);
  }
  free(nameList);

  if (!lFlag && !atleastOne) {
    fprintf(stderr, TCBYEL "No matches found\n" RESET);
  }
}

int getTotalBlocks(char* path, bool aFlag) {
  struct dirent** nameList;
  long totalBlocks = 0;

  int n = scandir(path, &nameList, NULL, alphasort);

  if (n < 0) {
    return 0;
  } else {
    for (int i = 0; i < n; i++) {
      struct stat st;
      char* itemName = nameList[i]->d_name;

      char* fullPath = strdup(path);
      fullPath = realloc(fullPath, sizeof(char) * (MAX_NAME_LENGTH + 1));
      strcat(fullPath, "/");
      strcat(fullPath, itemName);

      lstat(fullPath, &st);
      if (!fileIsHidden(itemName) || (aFlag && fileIsHidden(itemName))) {
        totalBlocks += st.st_blocks;
      }

      free(fullPath);
    }
  }

  for (int i = 0; i < n; i++) {
    free(nameList[i]);
  }
  free(nameList);

  return totalBlocks;
}