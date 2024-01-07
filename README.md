# Using the Shell
1. Create the executable by entering the following command in your terminal
```bash
make
```
2. Execute the shell
```bash
./a.out
```
- You may clean the executable and temporary files, using:
```bash
make clean
```
___
___
# Contents
* ### [The Unique Factor](https://github.com/Varun0157/SeaShell#the-unique-factor-1)
* ### [Features](https://github.com/Varun0157/SeaShell#features-1)
#### [exit](https://github.com/Varun0157/SeaShell#exit) | [neonate](https://github.com/Varun0157/SeaShell#signals) | [signals](https://github.com/Varun0157/SeaShell#signals) |  [fg and bg](https://github.com/Varun0157/SeaShell#fg-and-bg) | [iMan](https://github.com/Varun0157/SeaShell#iman) | [Multi-Command Structures](https://github.com/Varun0157/SeaShell#multi-command-structures) | [I/O Redirection](https://github.com/Varun0157/SeaShell#io-redirection) | [Piping](https://github.com/Varun0157/SeaShell#piping) | [activities](https://github.com/Varun0157/SeaShell#activities) |[warp](https://github.com/Varun0157/SeaShell#warp) | [peek](https://github.com/Varun0157/SeaShell#peek) | [pastevents](https://github.com/Varun0157/SeaShell#pastevents) | [System Commands](https://github.com/Varun0157/SeaShell#system-commands) | [proclore](https://github.com/Varun0157/SeaShell#proclore) | [seek](https://github.com/Varun0157/SeaShell#seek)
* ### [Improvements on Requirements](https://github.com/Varun0157/SeaShell#improvements-on-requirements-1)
* ### [Assumptions](https://github.com/Varun0157/SeaShell#assumptions-1)
* ### [Limitations](https://github.com/Varun0157/SeaShell#limitations-1)
___
__
# The Unique Factor
If nothing else is read, read this. 
- All commands are stored in a cleanly formatted, equivalent manner, performing similar pre-processing as bash for commands. Thus, any value withing strings, has strings omitted and is re-embedded into the string to be treated on it's own. 
- All commands are stored in the format:

    `command args "< inputRedir" "> outputRedir" | ...`
    
    Thus, even if different commands have the same action with respect to redirection, they are stored equivalently. 
    So: `echo   "hello" >   a.txt` and `echo>a.txt hello` are stored as one and the same in pastevents. 
Both of the above points were an attempt to solve Q90 in the doubts document, which we were told not to solve due to the implementational burden. But, I thought it would improve my shell so I went ahead and did it. 
___
___
# Features
The Shell prompt displays the username, the hostname, the current directory relative to the executable, as well as the time taken by the last command to execute, it it took more than 2 seconds.
If there is a background process that has not explcitly exited yet, a * is shown at the beginning of this prompt, similar to the starship shell. 
___
### Neonate
Has one optional `-n` flag that must be followed by an integer representing time in seconds. If no time is specified, it is taken as 1 second by default. 

The command prints the process-id of the most recently created process on the system every `time-arg` seconds until the key `x` or `X` is pressed. 

The terminal enters raw mode on execution of the command to allow this. 
- `neonate`
prints the most recently created process on the system every 1 second. 
- `neonate -n <time_arg>`
prints the most recently created process on the system every <time_arg> seconds. This must be a non-negative integer. 
___
### Signals
- ping
Using the `ping` command allows us to send particular signals to processes. 
```bash
ping <pid> <signal_number>
```
Sends the signal `signal_number` (%32) to the process `pid`. 

- `Ctrl+C` - interrupts any currently running foreground process by sending it the SIGINT signal. It has no such effect if no foreground process is currently running. In this case, it simply re-prompts with the same session as before (i.e., without handling background processes). 
- `Ctrl+D` - Logs out of the shell (after killing all processes) while having no effect on the actual terminal that the custom shell is running on. Stored as "exit" in pastevents. It only has effect while taking input.
- `Ctrl+Z` - Pushes the running foreground process to the background and changes its state to "Stopped". It has no such effect on the shell if no foreground process is running. In this case, it simply re-prompts with the same session as before (i.e., without handling background processes). 

#### Note
- On receiving input, `Ctrl+C` and `Ctrl+D` prompt a new input, partially mimicing the behaviour of bash. 
- `ping 0 9` will kill the shell and this is not unexpected
___
### fg and bg
- `fg <pid>`
brings the background process with corresponding pid to foreground, handing it control of terminal. Consequently, it will have no real effect if called on a foreground process. 
- `bg <pid>`
changes the state of a background process to running (in the background). Consequently, has no real effect if called on a foreground process. 
___
### iMan
We use sockets to run GET requests to `man.he.net` and retrieve information about any command, using `iMan <command_name>`. 

Atleast the Name, Synopsis and Description will be printed, if present. 
- `iMan <command_name>`
does the following:
1. DNS resolution for man.he.net
2. opens a TCP socket to the IP address
3. Sends a GET request to the website's server
4. Reads the body of the website
5. Closes the socket
___
### Multi-Command Structures
We can send multiple commands to the shell using `&` and `;` delimiters. Any command that is succeeded by a `&` is to be run as a background process.
```bash
command1 ; command2 & command3
``` 
In the above, `command1` runs in the foreground, followed by `command2` in the background and finally `command3` in the foreground. 

#### Note the following:
- An error is printed in case of erroneous processes. 
- When the command is stored, it is not necessarily stored as it was typed. An equivalent version of the command with cleaner and uniform formatting is stored, to allow for easier comparison with other commands. The command stored is in the form that execvp executes the command, i.e., it attempts to perform the same pre-processing as bash. 
___
### I/O Redirection
Input-Output redirection is supported, similar to bash. 
- `>` Writes to the filename specified as output
- `>>` Appends the file specified as output
- `<` Reads from the file specified as input

These can also be executed in the background similar to system calls. They work for both custom commands and bash commands executed using `execvp`.
___
### Piping
Piping is supported, similar to bash. I/O redirection can also be performed within a pipeline. 

This works for all commands, both custom commands and system commands executed using `execvp`.

#### Note
- If a pipeline is terminated by an ampersand (&), the last command in the pipeline is executed in the background. 
- Continous pipes are somewhat undefined in bash. If the continous pipes are separated by a space, bash throws an error. 
    - To improve on this functionality, we execute continuous pipes if they are separated by a space, treating this space as a 'void' that executes nothing. This can allow us to add commands that are not dependent on previous stages of the pipeline, to it. 
- Piping mimics bash in error handling. If an error is thrown in the middle of a pipeline, the pipeline continues executing after printing the error message. 
___
### activities
Prints the list of all processes currently running that were spawned by the shell, in lexicographic order of pid. 
On calling `activities` we see the list of background processes spawned by our shell, as follows:
```bash
pid_1:  name_1  state_1
...
pid_n:  name_2  state_2  

```
___
### Warp
Is our shell's equivalent to cd in bash. It allows to change directories to any directory given as a parameter. Any relative paths are assumed to be relative to the directory containing the executable of the shell. 
```bash
warp <path/name> <path/name> ...
```
Thus, 
- `~` or `~/path` is a path relative to the home directory
- `-` takes us to the previous working directory
- Any other argument is passed directly to chdir. Thus, if we type `newDir` as an argument it will attempt to cd to `newDir` in the current working directory, and if we type `/` it will take us to the root. 

#### Advantages over cd:
- Multiple inputs allowed
#### Disadvantages with respect to cd:
- Cannot directly navigate to a directory with spaces in its name
___
### Peek
is our shell's equivalent to the ls command in bash. it supports the same flags as bash, i.e.:
- `-l` displays all extra information
- `-a` displays all files, including hidden files

It is called as follows:
```bash
peek <flags> <path/name>
```
Any permutation of l and a is supported as a flag. 
The flags are optional. If no path/name is provided however, we peek the current working directory. 

#### Note:
- Stat displays number of blocks in the units of 512 bytes, and this is what is displayed in peek. This may not be the same as ls in all systems. Often, it is double the block size as other systems. 
- We display executables in green, directories in blue, and all other items are considered files, displayed in white. 
___
### Pastevents
Is similar to the history in actual shells. We store upto 15 of the most recent executed commands, across sessions.  
```bash
pastevents
```
The above command displays the past commands, from least recent to most recent, if any. 
```bash
pastevents execute <index>
```
The above command executes the command at the 1-indexed position `index`, if one is present. This command executes as a standalone, meaning if it is used to redirect to some file, then the output of all the commands will be send to that file. 

```bash
pastevents purge
```
Deletes the stored history

###### Note:
- The pastevents command stores an equivalent version of the entered command, and not the command itself, to ensure easy comparison with past and future commands that perform the same purpose. A closing `;` is intentionally not stored, if present, to make sense of pastevents redirecting to files in commands. Consequently, a closing `;` must be explicitly added to denote it as a single command, if required. 
- `pastevents execute <index>` is considered a separate command entity, i.e., on redirection, if the command in pastevents is composed of multiple commands, then they will all output and input from the redirection as a whole. It gets replaced with the command that is executed by it, in the final display.Any command shown on entering `pastevents` can be executed as part of a multi-command input or an individual input. 
- The history is stored as a hidden file in the same directory as the executable, under the name `._PAST_EVENTS_.txt`. The purpose of the obscure wording is to prevent potential clashes. Modifications to this file will lead to improper working of the pastevents command. 
    - This file can be deleted if the user wishes, but it will be regenerated on re-execution of the shell. 
- Complex string input with double quotes is allowed and accounted for, similar to bash. 
    - `echo "random word > a.txt"` is valid and echoes `random word > a.txt` without any redirection, as is `echo "random word" | wc | echo "random word | wc"`, and it will give the same output as bash. 
    - Even regular system commands work similar to bash. echo random"word" prints randomword, same as bash. 
___
### System Commands
Most valid system commands in bash can be executed succesfully in this terminal. We just enter the command as usual to execute it. 
- Any command that is succeeded by a `&`, whether as a part of the final argument or as an individual argument, will be executed in the background. 
- Else, it will execute in the foreground. 
- Background execution is not currently allowed for custom commands (warp, peek, seek etc).
- All arguments are passed directly to execvp(), so things such as paths work as it would in a normal terminal and not necessarily relative to our shell. 
___
### Proclore
is used to obtain information regarding any process. 
```bash
proclore <pid> <pid> ...
```
If no pid is provided, we print the information of our shell. 
We print the following details:
- `pid` the id of the process
- `Process Status` as read from /proc/<pid>/status, including a `+` if it is a foreground process. 
- `Process Group` The process group number of the process
- `Virtual Memory` The amount of virtual memory used by the process, in kilo-bytes
- `Executable Path` The path to the executable of the process, if accessible, relative to the shell executable. 

___
### Seek
This command looks for a file or directory in the specified target directory. 
```bash
seek <flags> <search> <target_directory>
```
The search item is the only mandatory component. 
Valid flags include:
-  `-e` - If there is more than one, or less than one match, then this flag is completely ineffective, and does nothing. However, if there is exactly one match, if it is a directory, then we change the current working directory to it, and if it is a file, then we print its contents. 
- `-f` - Only look for files
- `-d` - Only look for directories
Note that the `-d` and `-f` flags are ineffective together. 

#### Note
- Any item that passes `S_ISDIR(mode)` where `mode` is the mode of the item, is considered a directory, while anything that passes `S_ISREG(mode)` where `mode` is the mode of the item, is considered a file. 
- Files are coloured in green in the results while directories are blue. 
- Other file types, including devices, pipes, sym-links, sockets, etc. are not considered in seek. 
___
### exit
We can use the command exit with any arguments to provde a clean exit to the shell. Using this command ensures that:
- Any background processes started by the shell are killed
- All memory dynamically allocated is freed. 

```bash
exit
```
___

# Assumptions
- Based on Q38 in the Doubts Document, I assumed that I can colour the entire line of data of each item in the required format, not necessarily just the name. 
- Time is shown if the previous ENTIRE command takes more than 2 seconds to execute, whether background or foreground, whether system command or custom one.
- Displaying username as "username" by default, i.e., if username is not found 
- seek searches strictly for prefix. This means even newFolderFile will match on seeking newFolder. This is based on the wording of answer 75. 
- Apart from ~ and - directories, we pass the argument directly to chdir, which handles it. Thus, warp / would take us back to the root. Same with peek, seek etc. 
- In seek, anything that passes `S_ISDIR` is a directory, anything that passes `S_ISREG` is a file. 
- In peek, however, anything that passes `S_ISDIR` is a directory, anything that has execute permissions from the owner is an executable, and everything else is a file. 
- peek has no support for `-`, similar to ls, and neither does seek. We can only warp to the previous working directory, and not read it or perform similar operations.  
- Only double quote structures are supported for relevant commands. Eg: echo "random    word" will print as expected from the bash terminal, but 'random    word' may not. 
- Ctrl+D signal is only handled on receiving input.
- sleep, gedit and more go to the `S` (sleeping) state on executing them as a background process, so instead of only displaying running and stopped in `activities`, I am displaying all background processes that are active, in whatever state they are in, whether stopped, running, sleeping, or otherwise. 
- `<<` is treated as an attempt to perform `<` on `<`, and will inevitably lead to unintended output. A warning is provided in the execution for this too. 
- Background processes only store command name and not arguments made. This is what acttivities displays. 
- Redirection of files when used with relative paths is not relative to the shell executable. ~ will act as the home directory of the shell only for seek, peek and warp. 
- Operating on special files (eg: `sort < /dev/zero`) can lead to undefined behaviour. 
- iMan works by searching for name to set the start-index, and to set the end-index, it will find the last "\n\n" in the string, so as to eliminate the footer. 
___
___
# Improvements on Requirements
- Complex string input with double quotes is allowed and accounted for, similar to bash. 
    - `echo "random word > a.txt"` is valid and echoes `random word > a.txt` without any redirection, as is `echo "random word" | wc | echo "random word | wc"`, and it will give the same output as bash. 
    - Even regular system commands work similar to bash. echo random"word" prints randomword, same as bash. 
    - Please read [this](https://github.com/serc-courses/mini-project-1-Varun0157/tree/final#the-unique-factor-1)
- pastevents stores a uniformly formatted equivalent version of the passed command. This allows for various improvements. For example sleep\t5 is treated equivalent to sleep 5. 
- warp and seek change the working directory even in the middle of commands. Consequently, commands like `seek -e -d newfolder ./doe ; peek -al ;` would peek the new directory entered by seek, if any. 
- Large multi-command calls can be executed by `pastevents execute <index>`. This acts as a standalone function, meaning redirects lead to the output of all the commands in this call being redirected, and same for input. 
- Multiple PIDs can be passed as input in proclore. 
- Any types of flags allowed in peek, seek etc. We can enter any permutation of valid flags, and get verbose errors in case of disallowed ones. You can pass -allaalll as a valid flag in peek. 
- System commands are allowed to take input and provide output without breaking both parent and child processes, using signals. 
- exit command
- I/O redirection supports background execution. In piping, the last command can be executed in the background.
- If a pipeline is terminated by an ampersand (&), the last command in the pipeline is executed in the background. 
- Continous pipes are somewhat undefined in bash. If the continous pipes are separated by a space, bash throws an error. 
    - To improve on this functionality, we execute continuous pipes if they are separated by a space, treating this space as a 'void' that executes nothing. This can allow us to add commands that are not dependent on previous stages of the pipeline, to it.  
___
___
# Limitations
- Tokenising the input always leads to limitations. Eg: we cannot directly navigate to a file that has spaces in it's name because our shell will assume that two arguments were passed to navigate to. Also, it leads to improper echo outputs when echo contains a strict string which has delimiters within it. 
- Custom commands do not support background execution, and the character & is intentionally treated purely as & and not as a special character denoting background execution for them. 
- Single quote structures not adequately accounted for, although double quote structures are. 
___
___
