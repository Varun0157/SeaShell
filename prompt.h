#ifndef __PROMPT_H
#define __PROMPT_H

/**
 * @brief Master function for prompt. Determines the
 *  prompt that the shell provides to the user before getting
 *  input.
 *
 */
void prompt();

/**
 * @brief Returns the current working directory.
 *
 * @return char* - Current Working directory
 */
char* getCWD();

/**
 * @brief Returns the OLD_PWD (Old Present Working Directory), i.e.
 *  the previous working directory. Name was selected based on real
 *  env variable names.
 *
 * @return char* - Previous Working Directory, NULL if does not exist
 */
char* getOldPWD();

/**
 * @brief Frees CWD and OLD_PWD if malloced
 */
void freeDirStores();

/**
 * @brief Takes a path as input and stores the path to the directory
 *  containing that item in that path, in the same variable
 *
 * @param path The path to alter
 */
void storeDirInPath(char* path);

/**
 * @brief Sets the additionalDetails variable to be displayed
 *  in prompt, if any
 *
 * @param details The details to set to additionalDetails
 */
void setForegroundProcessDetails(char* details);

/**
 * @brief Stores the path to the directory of the executable
 *  in executablePath. The inputted path must be malloced with
 *  the required size.
 *
 * @param executablePath The variable to store the path in.
 */
void getExecutableDir(char* executablePath);

/**
 * @brief Stores the current working directory
 *  in the param. Updates CWD and OLD_PWD globals.
 *
 * @param currentDirPath The variable to store the current directory
 *  path in
 */
void getCurrentDir(char* currentDirPath);

/**
 * @brief Resets CWD and OLD_PWD assuming param is the new CWD
 *
 * @param currentDirPath
 */
void setCWD(char* currentDirPath);

/**
 * @brief Stores a relative path to path, relative to the executable,
 *  in to the @param relativePath
 *
 * @param path The path whose relative path is to be found
 * @param relativePath The relative path is stored into this
 */
void getRelativePath(char* path, char* relativePath);

/**
 * @brief Stores the username found by the process,
 *  defaults to UserNameNotFound as of Aug 23'
 *
 * @param userName The username is stored into this variable
 */
void getUserName(char* userName);
/**
 * @brief Stores the host name found by the process.
 *
 * @param hostName The variable to store host name in.
 */
void getHostName(char* hostName);

#endif