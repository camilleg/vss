include ../Rules.dso

DSO = fm.a
OBJS = fmdso.o fmAlg.o fmHand.o fmActor.o fmmAlg.o fmmHand.o fmmActor.o

ifeq "$(PLATFORMBASE)" "VSS_LINUX"
# -O2 or higher causes fmmactor to emit only nan's.
CFLAGS += -ffast-math -O1
endif

include ../Rules.make
