# Any copyright is dedicated to the Public Domain:
#     <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written by Alfonso Sabato Siciliano <https://alfonsosiciliano.gitlab.io>

OUTPUT=  bsddialog
SOURCES= libbsddialog.c bsddialog.c
OBJECTS= ${SOURCES:.c=.o}

# BASE ncurses
CFLAGS= -Wall
LDFLAGS= -L/usr/lib -lmenu -lncurses -ltinfo -L/usr/local/lib

# PORT ncurses `make -DPORTNCURSES` or `make -D PORTNCURSES`
.if defined(PORTNCURSES)
CFLAGS += -DPORTNCURSES -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lmenu -lform -lncurses -ltinfo
.endif

BINDIR= /usr/local/sbin

MAN= ${OUTPUT}.1
GZIP= gzip -cn
MANDIR= /usr/local/share/man/man1

INSTALL= install
RM= rm -f

all : ${OUTPUT}

clean:
	${RM} ${OUTPUT} *.o *~ *.core ${MAN}.gz

${OUTPUT}: ${OBJECTS}
	${CC} ${LDFLAGS} ${OBJECTS} -o ${.PREFIX}

.c.o:
	${CC} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}

install:
	${INSTALL} -s -m 555 ${OUTPUT} ${BINDIR}
	${GZIP} ${MAN} > ${MAN}.gz
	${INSTALL} -m 444 ${MAN}.gz ${MANDIR}

unistall:
	${RM} ${BINDIR}/${OUTPUT}
	${RM} ${MANDIR}/${MAN}.gz

