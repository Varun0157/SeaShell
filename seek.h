#ifndef __SEEK_H
#define __SEEK_H

/**
 * @brief The master seek function responsible for performing the command
 *  stated by the param.
 *
 * @param input The seek command to execute.
 */
void seek(char* args[], int argsCount);

/**
 * @brief Stores the path of @param path relative to @param relativeTo,
 *  in @param relativePath
 *
 * @param path
 * @param relativeTo
 * @param relativePath
 */
void getRelativeToGivenDir(char* path, char* relativeTo, char* relativePath);

/**
 * @brief Recursive function that seeks files and/or directories according to
 *  the set flags
 *
 * @param dFlag True if "-d" is passed, else false
 * @param fFlag True if "-f" is passed, else false
 * @param eFlag True if "-e" is passed, else false
 * @param search The item to search for
 * @param path The path to search in
 * @param sourcePath The initial path passed to source, to obtain relative paths
 * @param atleastOne Pointer to boolean true if atleast one match was found, else to false
 */
void seekItems(bool dFlag, bool fFlag, bool eFlag, char* search, char* path,
               char* sourcePath, bool* atleastOne);

/**
 * @brief Checks if the number of matches is less or more than 1
 *
 * @param dFlag True if "-d" is passed, else false
 * @param fFlag True if "-e" is passed, else false
 * @param search The item to search for
 * @param path The path to search in
 * @param numMatches Pointer to the number of matches stored so far
 */
void numMatchesChecker(bool dFlag, bool fFlag, char* search, char* path, int* numMatches);
/**
 * @brief Returns true if eFlag can be applied (number of matches not more than 1),
 *  else false.
 *
 * @param dFlag True if "-d" is passed, else false
 * @param fFlag True if "-e" is passed, else false
 * @param search The item to search for
 * @param path The path to search in
 */
int eFlagValid(bool dFlag, bool fFlag, char* search, char* path);

#endif