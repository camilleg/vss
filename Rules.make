# Rules to make */*.a and ./vss.

all: $(DSO)

clean_dso:
	-rm -f $(DSO) $(OBJS)

.SUFFIXES: .c .c++ .C .o .a .l .y
.PHONY: clean all

clean:
	-rm -f $(DSO) $(OBJS)

# non-windows ar might also want $(LDFLAGS) $(LIBS)
$(DSO): $(OBJS)
	$(AR) r $@ $(OBJS)
	-@chmod a+r $@
#ifeq "$(PLATFORMBASE)" "VSS_IRIX"
#	strip -fs $@
#endif

# noninteractive just-make-a-beep test
sane:
	@$(AUDTEST) sanity.aud

-include $(patsubst %.o,.depend/%.d,$(OBJS))
