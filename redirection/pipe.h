#ifndef __PIPE_H
#define __PIPE_H

/**
 * @brief Takes an arbitrary string input as the first
 *  argument in a strin array, and separates it into a list of commands
 *  separated by |'s
 *
 * @param args A string array of size 1, containing the input.
 *  The input size need not be 1, but that is the intention.
 * @param argsCount Size of args
 * @return true If we could isolate pipes succesfully
 * @return false If we could not isolate pipes succesfully
 */
bool isolatePipes(char* args[], int* argsCount);

/**
 * @brief executes the pipe separated commands. We have
 *  command1 | command2 | etc. in args.
 *
 * @param args The argument list to execute
 * @param argsCount The size of args
 */
void executePipes(char* args[], int* argsCount);

#endif