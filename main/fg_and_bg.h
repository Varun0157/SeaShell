#ifndef __FG_AND_BG_H
#define __FG_AND_BG_H

/**
 * @brief brings the process passed with the arguments to the foreground,
 *  if the command is valid, and the process can do the same
 *
 * @param args The arguments passed with the command call
 * @param argsCount The size of args
 */
void bringToFG(char* args[], int argsCount);

/**
 * @brief sends the SIGCONT signal to the process passed with the arguments,
 *  if the command is valid and the process can be sent to running state.
 *
 * @param args The argumetns passed with the command call
 * @param argsCount The size of args
 */
void sendToBG(char* args[], int argsCount);

/**
 * @brief Returns the name of the process denoted by pid.
 *  In the fail case, returns the pid itself as a string
 *
 * @param pid The process id whose name is to be retrieved
 * @return char* The name of the process denoted by pid
 */
char* getProcessName(int pid);

#endif