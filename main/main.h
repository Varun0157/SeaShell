#ifndef __MAIN_H
#define __MAIN_H

/**
 * @brief Attempts to kill all background processes and perform a
 *  clean exit, ensuring minimal (ideally, no) memory leaks
 */
void killBackgroundAndExit();

/**
 * @brief Frees all dynamically allocated global data, to prevent
 *  memory leaks
 */
void freeGlobalData();

/**
 * Strips all DELIMITERS away from the beginning of the string
 *
 * @param input the string (usually a command) to alter
 */
void stripBeginningDelimiters(char* input);

/**
 * @brief Strips all DELIMITERS away from the end of the string
 *
 * @param input the string (usually a command) to alter
 */
void stripEndingDelimiters(char* input);

/**
 * @brief Used by pastevents execute <index> to set a past event to execute.
 *  Would probably be better design to appoint this in pastevents.c and get()
 *  it to main.c, but meh
 *
 * @param command The command pointed to by pastevents execute <index>
 */
void setPastEvent(char* command);

/**
 * @brief Runs all the commands in input. Separates the zero or more
 *  individual commands into their constituents and passes them to
 *  executeCommand to run each one.
 *
 * @param input The set of commands, separated by ; or &
 * @param saveCommand if true, sends the run command to the prompt to display
 */
void runCommands(char* input, bool saveCommand);

/**
 * @brief Checks if the input requires a redirect at any point to execute
 *  by looking for a '<' or '>' match that is not within double quotes
 *
 * @param input The input to operate on
 * @return true if redirect is required
 * @return false if redirect is not required
 */
bool needsRedirect(char* input);

/**
 * @brief Checks if the input requires piping at any point by looking for a '|'
 *  match that is not within double quotes
 *
 * @param input The input to operate on
 * @return true if redirect is required
 * @return false if redirect is not required
 */
bool needsPiping(char* input);

/**
 * @brief The master function to execute any arguments. Uses dependency injection to
 *  satisfy use-cases of piping, redirection as well as regular calls.
 *
 * @param args The arguments to execute
 * @param argsCount The size of args
 * @param formattedCommand A cleanly formatted version of the executed command will be
 *  stored here for storage by pastevents, if saveCommand is set
 * @param redirectRequired True if a redirect is required at some point of the command
 * @param setFDs True if inputFD and outputFD need to be dup'd. Main utility is redirects
 * @param inputFD gets dup2'd to STDIN, if setFDs is set
 * @param outputFD gets dup2'd to STDOUT, if setFDs is set
 * @param pipingRequired True if piping is required in the command
 * @param saveCommand True if the command is to be saved, else false
 */
void executeArgs(char* args[], int* argsCount, char* formattedCommand,
                 bool redirectRequired, bool setFDs, int inputFD, int outputFD,
                 bool pipingRequired, bool saveCommand);

/**
 * @brief executes a command given by input. After parsing by ; and &, we send commands
 *  here.
 *
 * @param input The command to execute
 * @param formattedCommand The cleanly formatted command for pastevents is stored here
 * after execution, if saveFormatted is set
 * @param saveFormatted True if the command is to be saved, else false
 */
void executeCommand(char* input, char* formattedCommand, bool saveFormatted);

/**
 * @brief A setter function allowing other files to enable an exit
 *  on next command without an unclean exit
 */
void exitOnNextCommand();

/**
 * @brief Finds the index of the first ch AFTER start in input,
 *  or the index of the end of input
 *
 * @param ch the character to match
 * @param input The input string
 * @param start The index to search beyond
 * @return int The first match of ch after start, or the end of the string
 */
int findClosingCharOrEndOfWord(char ch, char* input, int start);

/**
 * @brief Tokenises the input while respecting bash execution styles.
 *  Converts any args within braces into separate tokens, whether embedded in
 *  other tokens or not. i.e., every instance of "text" becomes text, while respecting
 *  that it must not be executed if it is within quotes.
 * Note that the shell only supports double quotes for now.
 * This function is called only if no pipes are present. Else, we must isolate pipes
 * first.
 *
 * @param input The input command
 * @param args The store of the tokenised input after parsing
 * @param argsCount The size of args
 * @return true if the tokenisation was succesful
 * @return false if the tokenisation was not succesful
 */
bool tokenizeNoPipes(char* input, char* args[], int* argsCount);

/**
 * @brief Custom signal handler for SIGINT, to prevent
 *  shell termination on signal
 */
void sigintHandler();

/**
 * @brief Custom signal handler for SIGTSTP, to prevent
 *  shell termination on signal
 */
void sigtstpHandler();

/**
 * @brief Removes all action on reception of SIGINT or SIGTSTP
 */
void ignoreSignals();

/**
 * @brief Removes all action on reception of SIGINT or SIGTSTP
 */
void receiveSignals();

#endif