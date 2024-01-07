#ifndef __SIGNALS_H
#define __SIGNALS_H

/**
 * @brief Adds pid, name to background processes list
 *
 * @param pid The pid of item to add to background processes list
 * @param name The name of item to add to background processes list
 */
void sendToBackground(int pid, char* name);

/**
 * @brief The master function for the ping command. Sends a signal to
 *  a given PID if the args in the call are valid
 *
 * @param args The arguments in the call
 * @param argsCount The size of args
 */
void sendSignal(char* args[], int argsCount);

#endif