#ifndef __HEADERS_H
#define __HEADERS_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "activities.h"
#include "fg_and_bg.h"
#include "iMan.h"
#include "main.h"
#include "neonate.h"
#include "pastevents.h"
#include "peek.h"
#include "pipe.h"
#include "proclore.h"
#include "prompt.h"
#include "redirection.h"
#include "seek.h"
#include "signals.h"
#include "sysCommands.h"
#include "warp.h"

#define DELIMITERS " \n\t"

// https://gist.github.com/kanglicheng/e6a7794dbf847fe1a210a2e1fa4ddaf7
#define RESET "\033[0m"
#define TCDEF "\033[0;39m"
#define TCBGDEF "\033[0;49m"

#define TCNBLK "\033[0;30m"
#define TCNRED "\033[0;31m"
#define TCNGRN "\033[0;32m"
#define TCNYEL "\033[0;33m"
#define TCNBLU "\033[0;34m"
#define TCNMAG "\033[0;35m"
#define TCNCYN "\033[0;36m"
#define TCNWHT "\033[0;37m"

#define TCBBLK "\033[1;30m"
#define TCBRED "\033[1;31m"
#define TCBGRN "\033[1;32m"
#define TCBYEL "\033[1;33m"
#define TCBBLU "\033[1;34m"
#define TCBMAG "\033[1;35m"
#define TCBCYN "\033[1;36m"
#define TCBWHT "\033[1;37m"

#define TCBGBLK "\033[40m"
#define TCBGRED "\033[41m"
#define TCBGGRN "\033[42m"
#define TCBGYEL "\033[43m"
#define TCBGBLU "\033[44m"
#define TCBGMAG "\033[45m"
#define TCBGCYN "\033[46m"
#define TCBGWHT "\033[47m"

#include "settings.h"

#endif
