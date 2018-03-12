include ../Rules.dso

DSO  = stk.a

STK_DIR = ../stk-4.4.4/
STK_OBJS = $(STK_DIR)/src/Release/*.o
# Debug/*.o have format Mach-O, rather than the expected ELF.

OBJS = stkAlg.o stkHand.o stkActor.o $(STK_OBJS)

CFLAGS += -I$(STK_DIR)/include

# the flags are from STK/syntmono/Makefile
CFLAGS += -D__OS_Linux_ -D__VSS_ $(INC)

#$(STK_DIR)/%.o:	$(STK_DIR)/%.cpp $(STK_DIR)/%.h
#	$(CC) $(CFLAGS) -c $(<) -o $@

include ../Rules.make

#clean_stk:
#	-rm $(STK_OBJS)
