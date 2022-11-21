.SUFFIXES: .c .c++ .o .a .l .y
VERSION= 4.2.1
SRVDIR := ..
TOPDIR := $(shell cd ..; pwd)
include $(SRVDIR)/Rules.common

OBJCLI = lexMsg.o parseMsg.o vssClient.o actorMsg.o symtab.o cliMsg.o
CFLAGS := -O3 -Wall -Werror -pedantic -DDEBUG -D$(PLATFORMBASE) -D$(PLATFORM)
LEX := flex -l # apt install flex

ifeq "$(PLATFORMBASE)" "VSS_IRIX"
  # Use the mipspro compilers, not gnu.
  CC := CC
  CCC := cc -Xcpluscomm
  CFLAGS += -DVSS_OLD_H_FILES
ifeq "$(PLATFORM)" "VSS_IRIX_53"
  CFLAGS += -mips2
  LEX := lex
else
  CFLAGS += -mips4
  # Or, 64bit -mips4 -64; mips4 32bit -mips4 -n32; 32bit -mips3 -n32; o32bit -o32.
  # All these variants used to be built, in separate subdirectories.
endif
endif

ifeq "$(PLATFORM)" "VSS_CYGWIN32_NT40"
  YACC := byacc
else
  YACC := bison # apt install bison
endif

# -DVERBOSE prints tektronix-green diagnostics.

LIBSND := libsnd.a
all: $(LIBSND)

clean:
	-rm -rf parseMsg.tab.c* lex.yy.c* lexMsg.c++ parseMsg.c++ parseMsg.h parseMsg.tab.h libsnd.a *.o core

now=\"$(shell date +"%Y-%m-%d\ %H:%M")\"

$(LIBSND): $(SRVDIR)/vssBuild.c++ Makefile $(OBJCLI)
	-rm -f $@
	$(CC) -o vssBuild.o -c $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) $(SRVDIR)/vssBuild.c++
	ar r $@ vssBuild.o $(OBJCLI)
	-rm -f vssBuild.o
	chmod g+w $@

depend:
	$(CC) -M $(CFLAGS) *.c* | $(DEPENDFILTER) > dependfile

include dependfile

parseMsg.c++: parseMsg.y
	$(YACC) -d parseMsg.y
	mv parseMsg.tab.c $@
	-@chmod g+w parseMsg.tab.h $@

lexMsg.c++: lexMsg.l parseMsg.h
	$(LEX) lexMsg.l
	sed -e "s/^yylex/int yylex/" -e "s/#include <unistd.h>//" < lex.yy.c > $@
	-@rm lex.yy.c

# bug: seem to need to "make parseMsg.c++" before parseMsg.tab.h gets rebuilt.

%.h : %.y
	$(YACC) -d $<
	cp -p parseMsg.tab.h $*.h
	mv -f parseMsg.tab.c $*.c++

%.o: %.c++
	$(CC) -c $(CFLAGS) $<
	-@chmod g+w $@

%.o: %.c
	$(CCC) -c $(CFLAGS) $<
	-@chmod g+w $@
