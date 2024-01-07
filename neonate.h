#ifndef __NEONATE_H
#define __NEONATE_H

/*
This header is only needed by neonate, so left it here
*/
#include <termios.h>

/**
 * @brief Master function for the neonate command. Displays most recently spawned PID
 *  every [time_duration] set by the passed args, if valid.
 *
 * @param args The arguments determining time delay of neonate
 * @param argsCount The size of args
 */
void neonate(char* args[], int argsCount);

/**
 * @brief displays the error and exits the terminal.
 * An exit is essential because we have entered raw mode and may not be able to exit.
 *
 * @param s The error to display
 */
void die(const char* s);

/**
 * @brief Attempts to disable raw mode
 */
void disableRawMode();

/**
 * @brief Attempts to enable raw mode
 * The ECHO feature causes each key you type to be printed to the terminal, so you can see
 * what you’re typing. Terminal attributes can be read into a termios struct by
 * tcgetattr(). After modifying them, you can then apply them to the terminal using
 * tcsetattr(). The TCSAFLUSH argument specifies when to apply the change: in this case,
 * it waits for all pending output to be written to the terminal, and also discards any
 * input that hasn’t been read. The c_lflag field is for “local flags”
 *
 * @return true If we enabled raw mode
 * @return false If we could not enable raw mode
 */
bool enableRawMode();

#endif