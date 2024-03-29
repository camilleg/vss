# srv/Makefile

# This public release of the source code excludes
# the Chant and JmaxTest actors, which incorporated source code copyrighted by IRCAM,
# and the SpacePad actor, which incorporated source code copyrighted by Ascension Technology.

VERSION = 4.2.1

TOPDIR = $(PWD)

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

OBJSRV := \
  misc.o \
  parseActorMessage.o \
  platform.o \
  VActor.o \
  VAlgorithm.o \
  VGenActor.o \
  VHandler.o \
  vssMsg.o \
  vssSrv.o \

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
  distance \
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
  parallelport \
  penta \
  piano \
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
  thresh \

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
  distance/distance.a \
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
  parallelport/ledlpt.a \
  penta/pentatonic.a \
  piano/piano.a \
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
  thresh/thresh.a

# CXXFLAGS="$(CFLAGS)" would fail because ./configure clobbers that.
# Fake it by making CFLAGS part of what ./configure thinks is the compiler.
stk4/stk.a:
	cd stk-4.4.4 && ./configure CXX="$(CC) $(CFLAGS)" && cd src && make
	cd stk4 && make

now=\"$(shell date +"%Y-%m-%d\ %H:%M")\"
# 'T' deliberately omitted

# Explicity make inside each subdir, to get its ./.depend.
# gmake passes down "-j" implicitly via $(MAKEFLAGS), but not "-j <number>".
# ($(MAKE) ... & ) would build subdirs in parallel, to better exploit all cores,
# but then the linker would run before */*.a has been built.
$(TARGET): vssBuild.c++ $(OBJSRV) $(SUBDIRS) stk4/stk.a
	@set -e; for i in $(SUBDIRS); do ( $(MAKE) -s -C $$i | grep -v 'Nothing to be done for' || true ); done
	$(CC) -o $@ $(CFLAGS) -D__TIMESTAMP_ISO8601__=$(now) vssBuild.c++ $(OBJSRV) $(SUBLIBS) $(VSSLIBS) $(LDFLAGS)
ifeq "$(PLATFORMBASE)" "VSS_IRIX"
	strip -fs $@
endif

clean:
	-@rm -f stk-4.4.4/src/{Debug,Release}/*
	-@(cd stk-4.4.4; if [ -f Makefile ]; then ( make distclean | grep -v 'directory' | grep -v /bin/rm | grep -v ' make ' || true ); fi; )
	-rm -rf $(TARGET) *.o */*.o */*.a .depend */.depend

-include $(patsubst %.o,.depend/%.d,$(OBJSRV))

.PHONY: all clean sane $(SUBDIRS)
