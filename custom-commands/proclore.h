#ifndef __PROCLORE_H
#define __PROCLORE_H

/**
 * @brief Master function for proclore that performs the required
 *  command, if any
 *
 * @param args The command itself
 */
void proclore(char* args[], int argsCount);

/**
 * @brief Get all the details for the given pid
 *
 * @param pid The pid of the process whose details are to be retreived
 */
void getDetails(int pid);

#endif