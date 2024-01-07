#ifndef __REDIRECTION_H
#define __REDIRECTION_H

/**
 * @brief If redir is embedded in a token in args (or redirredir if searchDouble is true)
 *  then we separate this token into before+redir+after and separate them into distinct
 *  tokens in args. This way we account for inputs like echo word>filename etc.
 *
 * @param args The list of input tokens
 * @param argsCount Pointer to size of args
 * @param redir The character to search for
 * @param searchDouble True if we should also search for double instances of redir, else
 * false
 * @return true if the function executed succesfully
 * @return false if the function could not execute succesfully
 */
bool isolateRedirect(char* args[], int* argsCount, char redir, bool searchDouble);

/**
 * @brief Calls isolateRedirect on required sets to isolate >, >> and <
 *
 * @param args The list of input tokens
 * @param argsCount Pointer to size of args
 * @return true If the function executed succesfully
 * @return false If the function could not execute succesfully
 */
bool isolateAllRedirects(char* args[], int* argsCount);

/**
 * @brief Executes the command having redirects on getting the arguments,
 *  as well as the input and output file descriptors.
 *
 * @param args The arguments to pass to execvp
 * @param inputFD The input file descriptor determined by the redirect
 * @param outputFD The output file descriptor determined by the redirect]
 * @param runInBackground true if it is to be run in background
 */
void executeRedirect(char* args[], int inputFD, int outputFD, bool runInBackground);

/**
 * @brief The master function responsible for redirect command executions.
 *  Determined the input and output FDs, defaulted at STDIN and STDOUT and
 *  passes to executeRedirect
 *
 * @param args The input tokens
 * @param numArgs Pointer to size of args
 * @return true if function was run in background, else false
 */
bool redirect(char* args[], int* numArgs);

#endif