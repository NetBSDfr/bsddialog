# PUBLIC DOMAIN - NO WARRANTY, see:
#     <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written in 2021 by Alfonso Sabato Siciliano

OUTPUT=  bsddialog
SOURCES= bsddialog.c util_builders.c util_cli.c util_theme.c
OBJECTS= $(SOURCES:.c=.o)
LIBPATH= $(CURDIR)/lib
LIBBSDDIALOG= $(LIBPATH)/libbsddialog.so
CFLAGS= -D_GNU_SOURCE -Wall -Wextra -I$(LIBPATH)
LDFLAGS= -ltinfo -Wl,-rpath=$(LIBPATH) -L$(LIBPATH) -lbsddialog
RM = rm -f

all : $(OUTPUT)

$(OUTPUT): $(LIBBSDDIALOG) $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

${LIBBSDDIALOG}:
	make -C ${LIBPATH}

%.o: %.c $(LIBBSDDIALOG)
	$(CC) $(CFLAGS) -c $<

clean:
	make -C ${LIBPATH} clean
	$(RM) $(OUTPUT) *.o *~
