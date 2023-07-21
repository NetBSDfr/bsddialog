# PUBLIC DOMAIN - NO WARRANTY, see:
#     <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written in 2023 by Alfonso Sabato Siciliano

OUTPUT=  bsddialog
LIBPATH= ${PWD}/lib
LIBBSDDIALOG= ${LIBPATH}/libbsddialog.so
UTILITYPATH= ${PWD}/utility

RM= rm -f
LN = ln -s -f

export DEBUG
PARENTDIR= ${PWD}
export PARENTDIR

all : ${OUTPUT}


${OUTPUT}: ${LIBBSDDIALOG} ${OBJECTS}
	${MAKE} -C ${UTILITYPATH}
	${LN} ${UTILITYPATH}/${OUTPUT} ${PWD}/${OUTPUT}


${LIBBSDDIALOG}:
	${MAKE} -C ${LIBPATH}

clean:
	${MAKE} -C ${LIBPATH} clean
	${MAKE} -C ${UTILITYPATH} clean
	${RM} ${OUTPUT} *.core
