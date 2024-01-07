#ifndef __WARP_H
#define __WARP_H

/**
 * @brief Master function for the warp command. Takes args from the user
 *  and warps to the new directory, if valid
 *
 * @param args The arguments in the call from the user
 * @param argsCount The size of args
 */
void warp(char* args[], int argsCount);

#endif