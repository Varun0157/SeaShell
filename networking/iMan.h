#ifndef __IMAN_H
#define __IMAN_H

/*
These headers are completely independent of the
  rest of the code so leaving them here.
*/
#include <arpa/inet.h>
#include <netdb.h>

/**
 * @brief Prints the details of the command passed in the argument,
 *  provided by 'man.he.net' if the call is valid. Performs some checks first.
 *
 * @param args The arguments passed with the call to iMan
 * @param argsCount The size of args
 */
void iMan(char* args[], int argsCount);

#endif