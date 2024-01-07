.PHONY: main

CFLAGS = -Wall -Wextra -g
CFILES = main/main.c main/prompt.c custom-commands/warp.c custom-commands/peek.c custom-commands/proclore.c custom-commands/seek.c custom-commands/pastevents.c main/sysCommands.c redirection/redirection.c custom-commands/activities.c redirection/pipe.c main/signals.c main/fg_and_bg.c custom-commands/neonate.c networking/iMan.c

main:
	gcc ${CFLAGS} ${CFILES}
	
clean:
	rm -f a.out ._PAST_EVENTS_.txt
