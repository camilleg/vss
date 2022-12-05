# Rules to make */*.a.
# Included by */Makefile.
ifndef TOPDIR
  TOPDIR := $(shell cd ..; pwd)
endif
include ../Rules.common

all: $(DSO)

$(DSO): $(OBJS)
	$(AR) r $@ $(OBJS)

clean_dso:
	-rm -f $(DSO) $(OBJS)
clean: clean_dso
	-rm -rf .depend

# noninteractive just-make-a-beep test
sane:
	@$(AUDTEST) sanity.aud

-include $(patsubst %.o,.depend/%.d,$(OBJS))

.SUFFIXES: .c .c++ .C .o .a .l .y
.PHONY: all clean clean_dso sane
