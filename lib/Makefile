# Any copyright is dedicated to the Public Domain, NO WARRANTY, see:
#     <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written in 2021 by Alfonso Sabato Siciliano

VERSION = 0.0.2
LIBRARY = bsddialog
LIBRARY_SO = lib${LIBRARY:=.so}
LIBRARY_A = lib${LIBRARY:=.a}
HEADERS = bsddialog.h bsddialog_theme.h bsddialog_progressview.h
SOURCES = barbox.c formbox.c infobox.c libbsddialog.c lib_util.c menubox.c \
	messagebox.c textbox.c theme.c timebox.c
OBJECTS= ${SOURCES:.c=.o}
CFLAGS += -fPIC -Wall -Wextra
LDFLAGS += -fstack-protector-strong -shared -Wl,-x -Wl,--fatal-warnings \
	-Wl,--warn-shared-textrel -Wl,-soname,${LIBRARY_SO}.${VERSION} \
	-L/usr/lib -lformw -lncursesw -ltinfow

.if defined(DEBUG)
# `make -DDEBUG`
CFLAGS += -g
.else
CFLAGS += -O2 -pipe -std=gnu99 -Wno-format-zero-length \
	-fstack-protector-strong -Qunused-arguments
.endif

INSTALL_PREFIX=/usr/local
LN = ln -s -f
RM = rm -f
CP = cp
GZIP = gzip -cn
LDCONFIG = /sbin/ldconfig -m
MAN= ${OUTPUT}.3
GZIP= gzip -cn
MANDIR= /usr/local/share/man/man3
INSTALL= install
RM= rm -f

#all : man ${LIBRARY}
all : ${LIBRARY}

${LIBRARY}: ${LIBRARY_SO} ${LIBRARY_A}

${LIBRARY_SO}.${VERSION}: ${OBJECTS}
	${CC} ${LDFLAGS} ${.ALLSRC} -o ${LIBRARY_SO}.${VERSION}

${LIBRARY_SO}: ${LIBRARY_SO}.${VERSION}
	${LN} ${LIBRARY_SO}.${VERSION} ${LIBRARY_SO}

${LIBRARY_A}: ${OBJECTS}
	${AR} cr ${.TARGET} ${OBJECTS}
	${RANLIB} ${.TARGET}

.c.o:
	${CC} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}

man:
	${GZIP} ${LIBRARY}.3 > ${LIBRARY}.3.gz

clean:
	${RM} ${LIBRARY_SO}* *.o *~ *.gz ${LIBRARY_A}


install:
	${INSTALL} -m 644 ${HEADERS} ${INSTALL_PREFIX}/include
	${INSTALL} -m 644 -s ${LIBRARY_SO}.${VERSION} ${INSTALL_PREFIX}/lib/
	${INSTALL} -l rs ${INSTALL_PREFIX}/lib/${LIBRARY_SO}.${VERSION} ${INSTALL_PREFIX}/lib/${LIBRARY_SO}
	${INSTALL} -m 644 ${LIBRARY_A} ${INSTALL_PREFIX}/lib
	${LDCONFIG} ${INSTALL_PREFIX}/lib
	#${CP} ${LIBRARY}.3.gz ${INSTALL_PREFIX}/man/man3/

unistall:
	${RM} ${INSTALL_PREFIX}/include/${LIBRARY}.h
	${RM} ${INSTALL_PREFIX}/lib/${LIBRARY_SO}
	${RM} ${INSTALL_PREFIX}/lib/${LIBRARY_SO}.${VERSION}
	${LDCONFIG} ${INSTALL_PREFIX}/lib
	#${RM} ${INSTALL_PREFIX}/man/man3/${LIBRARY}.3.gz
