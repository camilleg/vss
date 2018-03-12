# Rules to make srv/*/*.a and srv/vss.

all: $(DSO)

clean_dso:
	-rm -f $(DSO) $(OBJS)

.SUFFIXES: .c .c++ .C .o .a .l .y
.PHONY: clean depend all

TOPDIR := $(shell cd ../..; pwd)
ifeq "$(PLATFORMBASE)" "VSS_SOLARIS"
CFLAGS += -fpic
cFLAGS += -fpic
# may need -lgcc with this, see "GCC Command Options" -nostdlib
# at http://gcc.gnu.org/onlinedocs/gcc_2.html
LDFLAGS += -nostdlib
endif

clean:
	-rm -f $(DSO) $(OBJS)
depend:
	$(CC) -M $(CFLAGS) $(wildcard *.c++ *.C) | $(DEPENDFILTER) > dependfile
ifneq "$(wildcard *.c)" ""
	$(CCC)   -M $(cFLAGS) $(wildcard *.c) | $(DEPENDFILTER) >> dependfile
endif

# non-windows ar might also want $(LDFLAGS) $(LIBS)
$(DSO): $(OBJS)
	$(AR) r $@ $(OBJS)
	-@chmod a+r $@
#ifeq "$(PLATFORMBASE)" "VSS_IRIX"
#	strip -fs $@
#else
#ifeq "$(PLATFORMBASE)" "VSS_SOLARIS"
#	strip $@
#else
#	strip -s $@
#endif
#endif

# noninteractive just-make-a-beep test
sane:
	@$(AUDTEST) sanity.aud

include dependfile
