# PUBLIC DOMAIN - NO WARRANTY, see:
#     <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written in 2021 by Alfonso Sabato Siciliano

OUTPUT =  bsddialog
SOURCES = bsddialog.c util_builders.c util_cli.c util_theme.c
OBJECTS = ${SOURCES:.c=.o}

CFLAGS += -I${LIBPATH} -std=gnu99 -Wall -Wextra -Werror
.if defined(DEBUG)
CFLAGS = -g -Wall -I${LIBPATH}
.endif
LDFLAGS += -ltinfow -Wl,-rpath=${LIBPATH} -L${LIBPATH} -lbsddialog

INSTALL = install
RM = rm -f

all : ${OUTPUT}

${OUTPUT}: ${OBJECTS}
	${CC} ${LDFLAGS} ${OBJECTS} -o ${.PREFIX}

.c.o:
	${CC} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}

clean:
	${RM} ${OUTPUT} *.o *~ *.core *.gz
