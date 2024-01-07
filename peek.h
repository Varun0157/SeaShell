#ifndef __PEEK_H
#define __PEEK_H

/**
 * @brief Master function for peek to parse the inputs and execute the
 *  required commands, if any.
 *
 * @param input The peek command to execute
 */
void peek(char* args[], int argsCount);

/**
 * @brief Determines if the file is hidden
 *
 * @param fileName The name of he file
 * @return true if the file is hidden,
 *   false if the file is not hidden
 */
bool fileIsHidden(char* fileName);

/**
 * @brief Returns all the details of the file as shown
 *  in ls -l
 *
 * @param name The name of the file whose details are to be fetched
 * @param st The stat struct of the file
 * @return char* - The details of the file
 */
char* getFileDetails(char* name, struct stat st);

/**
 * @brief Responsible for displaying the results of peeking the stated
 *  directory
 *
 * @param path The path to the file to peek
 * @param aFlag True if -a was passed, else false
 * @param lFlag True if -l was passed, else false
 */
void peekDirectory(char* path, bool aFlag, bool lFlag);

int getTotalBlocks(char* path, bool aFlag);

#endif