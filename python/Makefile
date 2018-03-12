include ../../Rules.dso

#	change these three if necessary, or define them as 
#	environment variables and make -e.
PythonTools := /apps/MAYAVI/DIST/Python-2.5/Tools
PythonPrefix := /usr/bin

DSO := python.so
OBJS := PyInterp.o PyActor.o vssSrv_wrap.o freezer/config.o freezer/M_*.o

include ../../Rules.make

#	lots of python baggage:
PY_VERSION		:=	1.5
ifeq "$(PLATFORMBASE)" "VSS_IRIX"
	PYCONFIG := $(PythonPrefix)/lib32/python$(PY_VERSION)/config
else
	PYCONFIG := $(PythonPrefix)/lib/python$(PY_VERSION)/config
endif
PYTHON_INCLUDE 	:= $(PythonPrefix)/include/python$(PY_VERSION)
LIBPYTHON		:= -lpython$(PY_VERSION)

includePython	:= -DHAVE_CONFIG_H -I$(PYTHON_INCLUDE) -I$(PYCONFIG)
linkPython		:= -L$(PYCONFIG) $(LIBPYTHON) 

cFLAGS	+= $(includePython)
CFLAGS	+= $(includePython)
LIBS	+= $(linkPython) 

# is this still necessary?
ifeq "$(OSBASE)" "linux"
	CFLAGS  += -DNO_NAMESPACES
endif

#	SWIG for Python:
SWIG	:= swig
SWIGOPT	:= -python -c++

vssSrv_wrap.c++: vssSrv.i
	$(SWIG) $(SWIGOPT) -o $@ vssSrv.i

#	freezing:
PYTHON		:= $(PythonPrefix)/python
FREEZE_DIR	:= ./freezer
FREEZE		:= $(PYTHON) $(PythonTools)/freeze/freeze.py
MAKE_HEADER	:= $(PYTHON) ./make_header.py 
FREEZE_ME	:= vssObjFreeze.py
# FREEZE_ME	:= freeze_me.py


#	need to build this target first!
frozen: $(FREEZE_DIR)/$(FREEZE_ME)
	cd $(FREEZE_DIR); $(FREEZE) $(FREEZE_ME); $(MAKE)
	$(MAKE_HEADER) $(FREEZE_DIR)/frozen.c ./frozen.h

cleanfreeze:
	-rm -f frozen.h
	cd $(FREEZE_DIR); $(MAKE) clean; rm *.c Makefile

# dependfile is busted
PyInterp.o: PyInterp.h PyInterp.c++ frozen.h
PyActor.o: PyInterp.h PyActor.c++ PyActor.h
