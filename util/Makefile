.SUFFIXES: .c .c++ .o .a .l .y
VERSION= 4.2.1
SRVDIR := ..
TOPDIR := $(shell cd ..; pwd)

# Sets cFLAGS, CFLAGS and LDFLAGS, possibly for debugging via ../Rules.debug.
include $(SRVDIR)/Rules.common

cFLAGS += -Werror -pedantic -I../cli
# -DVERBOSE prints tektronix-green diagnostics.

INC := ../cli/vssClient.h
LIBS := ../cli/libsnd.a

TARGETS		:= audtest dynamic vssflush vsskill vssping

OBJTEST		:= audtest.o
OBJDYNAMIC	:= dynamic.o
OBJFLSH		:= vssflush.o
OBJKILL		:= vsskill.o
OBJPNG		:= vssping.o

all: $(TARGETS)

clean:
	-rm -rf $(TARGETS) $(TARGETS:=.exe) *.o .depend

audtest: $(OBJTEST) $(LIB) $(INC)
	$(CC) -o $@ $(OBJTEST) $(LIBS) $(LDFLAGS)

vssping: $(OBJPNG) $(LIB) $(INC)
	$(CC) -o $@ $(OBJPNG) $(LIBS) $(LDFLAGS)

vssflush: $(OBJFLSH) $(LIB) $(INC)
	$(CC) -o $@ $(OBJFLSH) $(LIBS) $(LDFLAGS)

vsskill: $(OBJKILL) $(LIB) $(INC)
	$(CC) -o $@ $(OBJKILL) $(LIBS) $(LDFLAGS)

dynamic: $(OBJDYNAMIC) $(LIB) $(INC)
	$(CC) -o $@ $(OBJDYNAMIC) $(LIBS) $(LDFLAGS)
