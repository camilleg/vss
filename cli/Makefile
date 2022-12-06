.SUFFIXES: .c .c++ .o .a .l .y
VERSION= 4.2.1
SRVDIR := ..
TOPDIR := $(shell cd ..; pwd)

# Sets cFLAGS, CFLAGS and LDFLAGS, possibly for debugging via ../Rules.debug.
include $(SRVDIR)/Rules.common

cFLAGS += -Werror -pedantic
CFLAGS += -Werror -pedantic
# -DVERBOSE prints tektronix-green diagnostics.

ifeq "$(PLATFORM)" "VSS_IRIX_53"
  LEX := lex
else
  LEX := flex -l
  # apt install flex
endif

ifeq "$(PLATFORM)" "VSS_CYGWIN32_NT40"
  YACC := byacc
else
  YACC := bison
  # apt install bison
endif

LIBSND := libsnd.a
all: $(LIBSND)

clean:
	-rm -rf parseMsg.tab.c lex.yy.c* lexMsg.c++ parseMsg.c++ parseMsg.tab.h libsnd.a *.o .depend

now=\"$(shell date +"%Y-%m-%d\ %H:%M")\"

OBJCLI = lexMsg.o parseMsg.o vssClient.o actorMsg.o symtab.o cliMsg.o

$(LIBSND): $(SRVDIR)/vssBuild.c++ Makefile $(OBJCLI)
	-rm -f $@
	$(CC) -o vssBuild.o -c $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) $(SRVDIR)/vssBuild.c++
	ar r $@ vssBuild.o $(OBJCLI)
	-rm -f vssBuild.o

-include $(patsubst %.o,.depend/%.d,$(OBJCLI))

parseMsg.c++ parseMsg.tab.h: parseMsg.y
	$(YACC) -d parseMsg.y
	mv -f parseMsg.tab.c parseMsg.c++

lexMsg.o: parseMsg.tab.h

lexMsg.c++: lexMsg.l
	$(LEX) --stdout lexMsg.l | sed -e "s/^yylex/int yylex/" -e "s/#include <unistd.h>//" -e "s/define YY_NEED_STRLEN/undef YY_NEED_STRLEN/" -e "s/register / /g" > $@
