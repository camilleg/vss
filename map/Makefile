include ../Rules.dso

DSO = map.a
OBJS = mapdso.o mapActor.o \
	expMapActor.o linearMapActor.o segmentMapActor.o hidimMapActor.o \
	planeMapActor.o \
	ga.o gacli.o mc.o mcmap.o mcmap2.o mcpath.o mcspace.o mcsurf.o \
	sammon.o edahiro.o \
	ch.o fg.o hull.o io.o mymain.o pointops.o rand.o

ifeq "$(PLATFORMBASE)" "VSS_LINUX"
CFLAGS += -fno-fast-math
# -march=pentiumpro
# trying without -mno-ieee-fp
# bad data happens, with -ffast-math
endif

include ../Rules.make
