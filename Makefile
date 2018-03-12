# srv/Makefile

# This public release of the source code excludes
# the Chant and JmaxTest actors, which incorporated source code copyrighted by IRCAM,
# and the SpacePad actor, which incorporated source code copyrighted by Ascension Technology.

VERSION= 4.2

TOPDIR := $(shell cd ..; pwd)

.PHONY: depend sane clean

include Rules.common

ifeq "$(PLATFORMBASE)" "VSS_WINDOWS"
TARGET := vss.exe
else
TARGET := vss
endif

all: $(TARGET)

# noninteractive just-make-a-beep test
sane:
	@$(AUDTEST) sanity.aud

include dependfile


## ;;in linux, misc.o and fft.o may need CFLAGS += -O1
#ifeq "$(PLATFORMBASE)" "VSS_LINUX"
#  CFLAGS += -O1
#  cFLAGS += -O1
#endif

# for gdb
#CFLAGS += -g -O0
#cFLAGS += -g -O0

OBJSRV :=  vssSrv.o platform.o vssMsg.o VActor.o parseActorMessage.o \
                VAlgorithm.o VGenActor.o VHandler.o misc.o fft.o
ifeq "$(PLATFORMBASE)" "VSS_WINDOWS"
OBJSRV += vssWindows.o winplatform.o
endif

ifeq "$(PLATFORMBASE)" "VSS_IRIX"
OBJSRV += underflow.o
endif

SUBDIRS := \
  add \
  analog \
  analysis \
  basic \
  basiciter \
  chua \
  control \
  crowd \
  debug \
  delay \
  env \
  filter \
  fm \
  gran \
  input \
  later \
  logistic \
  map \
  mixer \
  msg \
  noise \
  osc \
  particle \
  penta \
  pnoise \
  process \
  reverb \
  ring \
  samp \
  seq \
  shimmer \
  sm \
  stereo \
  stk4 \
  tb303 \
  test \
  thresh \

# stk \
# stk3 \

# dsos/python is broken.

SUBLIBS := \
  add/add.a \
  analog/analog.a \
  analysis/analyzer.a \
  basic/basic.a \
  basiciter/basicIterator.a \
  chua/chua.a \
  control/control.a \
  crowd/crowd.a \
  debug/debug.a \
  delay/delay.a \
  env/env.a \
  filter/filter.a \
  fm/fm.a \
  gran/gran.a \
  input/input.a \
  later/later.a \
  logistic/logistic.a \
  map/map.a \
  mixer/mixer.a \
  msg/msgGroup.a \
  noise/noise.a \
  osc/opensoundctrl.a \
  particle/particle.a \
  penta/pentatonic.a \
  pnoise/pnoise.a \
  process/process.a \
  reverb/reverb.a \
  ring/ring.a \
  samp/samp.a \
  seq/seq.a \
  shimmer/shimmer.a \
  sm/sm.a \
  stereo/stereo.a \
  stk4/stk.a \
  tb303/tb303.a \
  test/test.a \
  thresh/thresh.a \

stk4/stk.a:
	cd stk-4.4.4 && ./configure && cd src && make
	cd stk4 && make

# bug: if "subdirs:" didn't update anything,
# i.e. $@ is older than all $(SUBLIBS), then this should also do nothing.
# Hardcode that as a bash "if" in here?
$(TARGET): vssBuild.c++ $(OBJSRV) subdirs stk4/stk.a
	$(CC) -o $@ $(CFLAGS) vssBuild.c++ $(OBJSRV) $(SUBLIBS) $(VSSLIBS) $(LDFLAGS)
	-@chmod a+rx $@
ifeq "$(PLATFORMBASE)" "VSS_IRIX"
	strip -fs $@
else
ifeq "$(PLATFORMBASE)" "VSS_SOLARIS"
	strip $@
#else
#	strip -s $@
endif
endif

# ( command || echo -n ) forces command's return code to be zero.
# "echo -n" is a no-op which returns 0.
# gmake passes down "-j" implicitly via $(MAKEFLAGS), but not "-j <number>".
# Workaround it with our own $(MAKEPARALLEL).
subdirs:
	@set -e; for i in $(SUBDIRS); do ( $(MAKE) -s $(MAKEPARALLEL) -C $$i | grep -v 'Nothing to be done for' || echo -n ); done

depend:
	$(CC) -M $(CFLAGS) *.c++ | $(DEPENDFILTER) > dependfile
	@for i in $(SUBDIRS); do $(MAKE) -s -C $$i depend || exit 1; done
ifneq "$(wildcard *.c)" ""
	$(CCC) -M $(cFLAGS) $(wildcard *.c) | $(DEPENDFILTER) >> dependfile
endif

clean:
	-rm -f stk-4.4.4/src/{Debug,Release}/*
	-(cd stk-4.4.4 && make distclean)
	-rm -rf $(TARGET) *.o */*.o */*/*.o */*.a */dependfile dependfile core core.* vss.exe.stackdump
	touch dependfile
	for i in $(SUBDIRS); do touch $$i/dependfile; done
