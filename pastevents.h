#ifndef __PASTEVENTS_H
#define __PASTEVENTS_H

/**
 * @brief Frees all the globally stored past events, to
 *  prevent memory leaks
 */
void freePastEvents();

/**
 * @brief Reads from HISTORY_FILE to store the past events
 *  into a local variable, pastEvents.
 */
void loadPastEvents();

/**
 * @brief Updates pastevents with the new command executed
 *
 * @param command The most recently executed command
 */
void updatePastEvents(char* command);

/**
 * @brief The master function for pastevents, manages calls to execute, 
 *  purge, and the default pastevents function.
 * 
 * @param args The arguments passed along with the pastevents function
 * @param argsCount The size of args
 */
void pastevents(char* args[], int* argsCount);

#endif