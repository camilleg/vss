# util/Makefile

.SUFFIXES: .c .c++ .o .a .l .y
SRVDIR := ..
VERSION= 4.0
TOPDIR := $(shell cd ..; pwd)
all: do-it-all
include $(SRVDIR)/Rules.common

CFLAGS := -O3 -Wall -Werror -pedantic -I../cli
cFLAGS := $(CFLAGS)
#  -DVERBOSE

ifeq "$(PLATFORMBASE)" "VSS_SOLARIS"
LD := g++
LIBSND := libsnd.a
endif

ifeq "$(PLATFORMBASE)" "VSS_LINUX"
LD := g++
LIBSND := libsnd.a
endif

ifeq "$(PLATFORMBASE)" "VSS_MAC"
LD := g++
LIBSND := libsnd.a
endif

ifeq "$(PLATFORMBASE)" "VSS_IRIX"
LD := CC
CFLAGS += -DVSS_OLD_H_FILES
ifeq "$(PLATFORM)" "VSS_IRIX_53"
CC := cc
LD := CC
CFLAGS += -mips2
LIBSND := libsnd.a
else
ifeq "$(PLATFORM)" "VSS_IRIX_62"
CFLAGS += -mips2
LIBSND := libsnd.a
else
LD += -n32
CFLAGS += -n32 -mips3
LIBSND := libsnd_n32.a
endif
endif
endif

ifeq "$(PLATFORMBASE)" "VSS_WINDOWS"
LD := g++
CC := gcc
LIBSND := libsnd.a
endif

INC			:= ../cli/vssClient.h
LIB			:= ../cli/$(LIBSND)
LIBS		:= $(LIB)

#	sockets stuf for Solaris
ifeq "$(PLATFORMBASE)" "VSS_SOLARIS"
LIBS += -lxnet
endif

.PHONY: audpanel_always

TARGETS		:= audtest vssping vssflush vsskill dynamic

OBJSND		:= vsssend.o
OBJPNG		:= vssping.o
OBJFLSH		:= vssflush.o
OBJKILL		:= vsskill.o
OBJTEST		:= audtest.o
OBJDYNAMIC	:= dynamic.o

do-it-all: $(TARGETS)

clean:
	-rm -rf $(TARGETS) $(TARGETS:=.exe) *.o .depend

# Stripped audtest dumps core sometimes?
audtest: $(OBJTEST) $(LIB) $(INC)
	$(LD) -s -o $@ $(OBJTEST) $(LIBS)

vssping: $(OBJPNG) $(LIB) $(INC)
	$(LD) -s -o $@ $(OBJPNG) $(LIBS)

vssflush: $(OBJFLSH) $(LIB) $(INC)
	$(LD) -s -o $@ $(OBJFLSH) $(LIBS)

vsskill: $(OBJKILL) $(LIB) $(INC)
	$(LD) -s -o $@ $(OBJKILL) $(LIBS)

dynamic: $(OBJDYNAMIC) $(LIB) $(INC)
	$(LD) -s -o $@ $(OBJDYNAMIC) $(LIBS)
