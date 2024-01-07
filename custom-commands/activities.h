#ifndef __ACTIVITIES_H
#define __ACTIVITIES_H

/**
 * @brief Comparator function to sort processes by PID
 *
 * @param a The first item
 * @param b The second item
 * @return int The difference in PIDs between the two items
 */
int compareProcesses(const void* a, const void* b);

/**
 * @brief Get the state of the process represented by PID
 * Returns '-' in case the state is not accessible for any reason
 *
 * @param pid The pid of the process
 * @return char A character denoting the state of 'pid'
 */
char getState(int pid);

/**
 * @brief The master function for the activities command
 *
 * @param argsCount The number of arguments passed with the command
 */
void activities(int argsCount);

#endif