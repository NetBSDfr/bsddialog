# PUBLIC DOMAIN - NO WARRANTY, see:
#     <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written in 2023 by Alfonso Sabato Siciliano

OUTPUT =  bsddialog
export VERSION=0.5-devel
.CURDIR ?= ${CURDIR}
LIBPATH = ${.CURDIR}/lib
LIBBSDDIALOG = ${LIBPATH}/libbsddialog.so
UTILITYPATH = ${.CURDIR}/utility

RM= rm -f
LN = ln -s -f

### cli options ###
# ports/pkg Makefile: MAKE_ARGS NORPATH=1
NORPATH ?= 0
# `make -DDEBUG`
# `gmake DEBUG=1`
ARGDEBUG ?= 0
export DEBUG=${ARGDEBUG}

all : ${OUTPUT}


${OUTPUT}: ${LIBBSDDIALOG} ${OBJECTS}
	${MAKE} -C ${UTILITYPATH} LIBPATH=${LIBPATH} NORPATH=${NORPATH}
	${LN} ${UTILITYPATH}/${OUTPUT} ${.CURDIR}/${OUTPUT}


${LIBBSDDIALOG}:
	${MAKE} -C ${LIBPATH}

clean:
	${MAKE} -C ${LIBPATH} clean
	${MAKE} -C ${UTILITYPATH} clean
	${RM} ${OUTPUT} *.core

