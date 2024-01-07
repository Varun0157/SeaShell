#ifndef __SYS_COMMANDS_H
#define __SYS_COMMANDS_H

typedef struct {
  char* name;
  int pid;
} processDetails;

/**
 * @brief Sets the PID of the current foreground process.
 *  Stores -1 if there is none
 *
 * @param newPid The PID to set
 */
void setFgProcessPID(int newPid);
/**
 * @brief Sets the name of the current foreground process.
 *  Stores NULL if there is none
 *
 * @param name The name to set
 */
void setFgProcessName(char* name);
/**
 * @brief returns the pid of the current foreground process
 *
 * @return int the pid of the current fg process
 */
int getFgProcessPID();
/**
 * @brief returns the name of the current fg process
 *
 * @return char* the name of the current fg process
 */
char* getFgProcessName();
/**
 * @brief frees the fgProcessName, if it was dynamically allocated,
 *  to prevent a memory leak.
 *
 */
void freeFgProcessName();

/**
 * @brief Adds a background process item to the list with given pid and name
 *
 * @param pid The pid of the item to add to the bg process list
 * @param name The name of the item to add to the bg process list
 */
void addBackgroundProcess(int pid, char* name);
/**
 * @brief Removes an item with the given pid from the bg process list,
 *  if one is present
 *
 * @param pid the pid of the process to remove
 */
void removeBackGroundProcess(int pid);
/**
 * @brief Returns the entire background process list
 *
 * @return processDetails* the background process list
 */
processDetails* getBackgroundProcesses();
/**
 * @brief returns the size of background process list
 *
 * @return int size of bg process list
 */
int getBackgroundProcessCount();
/**
 * @brief Frees the dynamically allocated parts of the globa bg
 *  process list, to prevent a potential memory leak.
 */
void freeBackGroundProcesses();
/**
 * @brief Kills any processes in the background processes list,
 *  using SIGTERM to allow the process to perform any necessary cleanups
 * first
 */
void killBackGroundProcess();
/**
 * @brief Checks if any processes in the bg process list are done executing
 * and displays and removes them if they are
 */
void handleBackGroundProcesses();

/**
 * @brief The function that performs the sys command execution, with details
 *  provided from makeSystemCall after it parses it
 *
 * @param args The arguments to be passed to exec
 * @param runInBackground True if process is to be executed in the background,
 *  else false
 */
void executeSysCommand(char* args[], bool runInBackground);
/**
 * @brief The master function to implement the system calls in the shell. Takes the args
 * parses them as required and sends to executeSysCommand
 *
 * @param args The arguments given in the call by the user
 * @param argsCount The size of args
 * @return true if the command was executed in the background
 * @return false if the command was executed in the foreground
 */
bool makeSystemCall(char* args[], int* argsCount);

#endif