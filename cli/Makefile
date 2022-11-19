# cli/Makefile

.SUFFIXES: .c .c++ .o .a .l .y

SRVDIR := ..

VERSION= 4.0
TOPDIR := $(shell cd ..; pwd)
all: do-it-all
include $(SRVDIR)/Rules.common

OBJCLI = lexMsg.o parseMsg.o vssClient.o actorMsg.o symtab.o cliMsg.o
CFLAGS_ALWAYS := -O3 -DDEBUG -D$(PLATFORMBASE) -D$(PLATFORM)
  LEX := flex -l # apt install flex

ifeq "$(PLATFORMBASE)" "VSS_IRIX"
  CC := CC
  CCC := cc -Xcpluscomm
  # Use the mipspro compilers on irix, not gnu.
  CFLAGS_ALWAYS += -DVSS_OLD_H_FILES
ifeq "$(PLATFORM)" "VSS_IRIX_53"
  VER_NORMAL = -mips2
  LEX := lex
else
  VER_NORMAL = -mips4
endif
  VER_64BIT = -mips4 -64
  VER_MIPS4_32BIT = -mips4 -n32
  VER_32BIT = -mips3 -n32
  VER_O32BIT = -o32
endif

ifeq "$(PLATFORMBASE)" "VSS_LINUX"
# deprecate -DREDHAT52 ;;;;
  CFLAGS_ALWAYS += -DREDHAT52
endif
ifeq "$(PLATFORMBASE)" "VSS_FreeBSD"
# deprecate -DREDHAT52 ;;;;
  CFLAGS_ALWAYS += -DREDHAT52
endif

ifeq "$(PLATFORM)" "VSS_CYGWIN32_NT40"
  YACC := byacc
else
  YACC := bison # apt install bison
endif

ifeq "$(PLATFORMBASE)" "VSS_SOLARIS"
  LEX := lex
  # necessary for building a shared object that
  # links libsnd.a:
  CFLAGS_ALWAYS += -fpic	
endif

# -DVERBOSE prints tektronix-green diagnostics.

CFLAGS = $(CFLAGS_ALWAYS)
cFLAGS = $(CFLAGS_ALWAYS)
YFLAGS = -d

LIBSND      := libsnd.a
LIBSND_o32  := libsnd_o32.a
LIBSND_MIPS4_n32 := libsnd_mips4n32.a
LIBSND_n32  := libsnd_n32.a
LIBSND_64   := libsnd_64.a

SRC := *.c++ *.h *.y *.l

TARGETS4 = 0
TARGETS := $(LIBSND)
ifeq "$(PLATFORMBASE)" "VSS_IRIX"
ifneq "$(PLATFORM)" "VSS_IRIX_53"
  TARGETS += $(LIBSND_o32) $(LIBSND_n32) $(LIBSND_MIPS4_n32) $(LIBSND_64)
  TARGETS4 = 1
endif
endif

do-it-all: $(TARGETS)

clean:
	-rm -rf parseMsg.tab.c* lex.yy.c* lexMsg.c++ parseMsg.c++ parseMsg.h parseMsg.tab.h libsnd.a *.o obj obj_64 obj_mips4_n32 obj_n32 obj_o32 core
	@mkdir obj
	@mkdir obj_64
	@mkdir obj_mips4_n32
	@mkdir obj_n32
	@mkdir obj_o32

now=\"$(shell date +"%Y-%m-%d\ %H:%M")\"

$(LIBSND): $(SRVDIR)/vssBuild.c++ Makefile $(OBJCLI)
	-rm -f $@
	$(CC) -o vssBuild.o -c $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) $(VER_NORMAL) $(SRVDIR)/vssBuild.c++
ifneq "$(PLATFORMBASE)" "VSS_IRIX"
	ar r $@ vssBuild.o $(OBJCLI)
else
	ar r $@ vssBuild.o `echo $(OBJCLI) | sed -e 's,^,obj/,' -e 's, , obj/,g'`
endif
	-rm -f vssBuild.o
	chmod g+w $@

$(LIBSND_o32): $(SRVDIR)/vssBuild.c++ Makefile $(OBJCLI)
	-rm -f $@
	$(CC) -o vssBuild.o -c $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) $(VER_O32BIT) $(SRVDIR)/vssBuild.c++
	ar r $@ vssBuild.o `echo $(OBJCLI) | sed -e 's,^,obj_o32/,' -e 's, , obj_o32/,g'`
	-rm -f vssBuild.o
	chmod g+w $@

$(LIBSND_MIPS4_n32): $(SRVDIR)/vssBuild.c++ Makefile $(OBJCLI)
	-rm -f $@
	$(CC) -o vssBuild.o -c $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) $(VER_MIPS4_32BIT) $(SRVDIR)/vssBuild.c++
	ar r $@ vssBuild.o `echo $(OBJCLI) | sed -e 's,^,obj_mips4_n32/,' -e 's, , obj_mips4_n32/,g'`
	-rm -f vssBuild.o
	chmod g+w $@

$(LIBSND_n32): $(SRVDIR)/vssBuild.c++ Makefile $(OBJCLI)
	-rm -f $@
	$(CC) -o vssBuild.o -c $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) $(VER_32BIT) $(SRVDIR)/vssBuild.c++
	ar r $@ vssBuild.o `echo $(OBJCLI) | sed -e 's,^,obj_n32/,' -e 's, , obj_n32/,g'`
	-rm -f vssBuild.o
	chmod g+w $@

$(LIBSND_64): $(SRVDIR)/vssBuild.c++ Makefile $(OBJCLI)
	-rm -f $@
	$(CC) -o vssBuild.o -c $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) $(VER_64BIT) $(SRVDIR)/vssBuild.c++
	ar r $@ vssBuild.o `echo $(OBJCLI) | sed -e 's,^,obj_64/,' -e 's, , obj_64/,g'`
	-rm -f vssBuild.o
	chmod g+w $@

depend:
	$(CC) -M $(CFLAGS_ALWAYS) *.c* | $(DEPENDFILTER) > dependfile

include dependfile

parseMsg.c++: parseMsg.y
	$(YACC) $(YFLAGS) parseMsg.y
	mv parseMsg.tab.c $@
	-@chmod g+w parseMsg.tab.h $@

lexMsg.c++:	lexMsg.l parseMsg.h
	$(LEX) lexMsg.l
	sed -e "s/^yylex/int yylex/" -e "s/#include <unistd.h>//" < lex.yy.c > $@
	-@rm lex.yy.c

# bug: seem to need to "make parseMsg.c++" before parseMsg.tab.h gets rebuilt.

%.h : %.y
	$(YACC) -d $<
	cp -p parseMsg.tab.h $*.h
	mv -f parseMsg.tab.c $*.c++

lexMsg.o : lexMsg.c++
ifneq "$(PLATFORMBASE)" "VSS_IRIX"
ifneq "$(PLATFORMBASE)" "VSS_SOLARIS"
	$(CC) -c $(CFLAGS) lexMsg.c++
else
	$(CC) -c $(CFLAGS) -D__EXTERN_C__ lexMsg.c++
endif
	-@chmod g+w $@
else
	-@if test ! -d obj ; then rm -rf obj; mkdir obj; fi
	$(CC) -c $(CFLAGS) $(VER_NORMAL) lexMsg.c++
	-@chmod g+w $@; mv $@ obj
ifeq "$(TARGETS4)" "1"
	-@if test ! -d obj_o32 ; then rm -rf obj_o32; mkdir obj_o32; fi
	-@if test ! -d obj_n32 ; then rm -rf obj_n32; mkdir obj_n32; fi
	-@if test ! -d obj_mips4_n32 ; then rm -rf obj_mips4_n32; mkdir obj_mips4_n32; fi
	-@if test ! -d obj_64 ; then rm -rf obj_64; mkdir obj_64; fi
	$(CC) -c $(CFLAGS) $(VER_O32BIT) lexMsg.c++
	-@chmod g+w $@; mv $@ obj_o32
	$(CC) -c $(CFLAGS) $(VER_32BIT) lexMsg.c++
	-@chmod g+w $@; mv $@ obj_n32
	$(CC) -c $(CFLAGS) $(VER_MIPS4_32BIT) lexMsg.c++
	-@chmod g+w $@; mv $@ obj_mips4_n32
	$(CC) -c $(CFLAGS) $(VER_64BIT) lexMsg.c++
	-@chmod g+w $@; mv $@ obj_64
	-@touch $@; chmod g+w $@
endif
endif

parseMsg.o : parseMsg.c++
ifneq "$(PLATFORMBASE)" "VSS_IRIX"
	$(CC) -c $(CFLAGS) parseMsg.c++
	-@chmod g+w $@
else
	-@if test ! -d obj ; then rm -rf obj; mkdir obj; fi
	$(CC) -c $(CFLAGS) $(VER_NORMAL) parseMsg.c++
	-@chmod g+w $@; mv $@ obj
ifeq "$(TARGETS4)" "1"
	-@if test ! -d obj_o32 ; then rm -rf obj_o32; mkdir obj_o32; fi
	-@if test ! -d obj_n32 ; then rm -rf obj_n32; mkdir obj_n32; fi
	-@if test ! -d obj_mips4_n32 ; then rm -rf obj_mips4_n32; mkdir obj_mips4_n32; fi
	-@if test ! -d obj_64 ; then rm -rf obj_64; mkdir obj_64; fi
	$(CC) -c $(CFLAGS) $(VER_O32BIT) parseMsg.c++
	-@chmod g+w $@; mv $@ obj_o32
	$(CC) -c $(CFLAGS) $(VER_32BIT) parseMsg.c++
	-@chmod g+w $@; mv $@ obj_n32
	$(CC) -c $(CFLAGS) $(VER_MIPS4_32BIT) parseMsg.c++
	-@chmod g+w $@; mv $@ obj_mips4_n32
	$(CC) -c $(CFLAGS) $(VER_64BIT) parseMsg.c++
	-@chmod g+w $@; mv $@ obj_64
	-@touch $@; chmod g+w $@
endif
endif

%.o: %.c++
ifneq "$(PLATFORMBASE)" "VSS_IRIX"
	$(CC) -c $(CFLAGS)  $<
	-@chmod g+w $@
else
	-@if test ! -d obj ; then rm -rf obj; mkdir obj; fi
	$(CC) -c $(CFLAGS) $(VER_NORMAL) $<
	-@chmod g+w $@; mv $@ obj
ifeq "$(TARGETS4)" "1"
	-@if test ! -d obj_o32 ; then rm -rf obj_o32; mkdir obj_o32; fi
	-@if test ! -d obj_n32 ; then rm -rf obj_n32; mkdir obj_n32; fi
	-@if test ! -d obj_mips4_n32 ; then rm -rf obj_mips4_n32; mkdir obj_mips4_n32; fi
	-@if test ! -d obj_64 ; then rm -rf obj_64; mkdir obj_64; fi
	$(CC) -c $(CFLAGS) $(VER_O32BIT) $<
	-@chmod g+w $@; mv $@ obj_o32
	$(CC) -c $(CFLAGS) $(VER_32BIT) $<
	-@chmod g+w $@; mv $@ obj_n32
	$(CC) -c $(CFLAGS) $(VER_MIPS4_32BIT) $<
	-@chmod g+w $@; mv $@ obj_mips4_n32
	$(CC) -c $(CFLAGS) $(VER_64BIT) $<
	-@chmod g+w $@; mv $@ obj_64
	-@touch $@; chmod g+w $@
endif
endif

%.o: %.c
ifneq "$(PLATFORMBASE)" "VSS_IRIX"
	$(CCC) -c $(cFLAGS) $<
	-@chmod g+w $@
else
	-@if test ! -d obj ; then rm -rf obj; mkdir obj; fi
	$(CCC) -c $(cFLAGS) $(VER_NORMAL) $<
	-@chmod g+w $@; mv $@ obj
ifeq "$(TARGETS4)" "1"
	-@if test ! -d obj_o32 ; then rm -rf obj_o32; mkdir obj_o32; fi
	-@if test ! -d obj_n32 ; then rm -rf obj_n32; mkdir obj_n32; fi
	-@if test ! -d obj_mips4_n32 ; then rm -rf obj_mips4_n32; mkdir obj_mips4_n32; fi
	-@if test ! -d obj_64 ; then rm -rf obj_64; mkdir obj_64; fi
	cc -Xcpluscomm -c $(cFLAGS) $(VER_O32BIT) $<
	-@chmod g+w $@; mv $@ obj_o32
	cc -Xcpluscomm -c $(cFLAGS) $(VER_32BIT) $<
	-@chmod g+w $@; mv $@ obj_n32
	cc -Xcpluscomm -c $(cFLAGS) $(VER_MIPS4_32BIT) $<
	-@chmod g+w $@; mv $@ obj_mips4_n32
	cc -Xcpluscomm -c $(cFLAGS) $(VER_64BIT) $<
	-@chmod g+w $@; mv $@ obj_64
	-@touch $@; chmod g+w $@
endif
endif
