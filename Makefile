# Any copyright is dedicated to the Public Domain, see:
#     <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written in 2021 by Alfonso Sabato Siciliano

OUTPUT=  bsddialog
SOURCES= theme.c msgbox_yesno.c menus.c lib_util.c libbsddialog.c bsddialog.c
OBJECTS= ${SOURCES:.c=.o}

# BASE ncurses
CFLAGS= -Wall
LDFLAGS= -L/usr/lib -lform -lncurses -ltinfo

# PORT ncurses `make -DPORTNCURSES` or `make -D PORTNCURSES`
.if defined(PORTNCURSES)
CFLAGS += -DPORTNCURSES -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lform -lncurses -ltinfo
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

