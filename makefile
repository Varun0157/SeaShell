CFLAGS = -Wall -Wextra -g
CFILES = main.c prompt.c warp.c peek.c proclore.c seek.c pastevents.c sysCommands.c redirection.c activities.c pipe.c signals.c fg_and_bg.c neonate.c iMan.c

main:
	gcc ${CFLAGS} ${CFILES}
	
clean:
	rm -f a.out main ._PAST_EVENTS_.txt
