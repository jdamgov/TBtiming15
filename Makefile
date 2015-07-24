SHELL         = bash
BASEDIR       = $(shell pwd)
#H4DQMSRCDIR   = ../H4DQM/src
SRCDIR        = src
TESTDIR        = test
LIBDIR        = lib
BINDIR        = bin
LIBNAME       = FastTimeTestBeamAnalysis

SRCFILES=$(wildcard $(SRCDIR)/*.cc)
OBJFILES=$(patsubst %.cc,%.o,$(SRCFILES)) 
#SRCFILES += $(H4DQMSRCDIR)/Waveform.cpp $(H4DQMSRCDIR)/WaveformFit.cpp $(H4DQMSRCDIR)/WaveformUtils.cpp
#OBJFILES += $(H4DQMSRCDIR)/Waveform.o   $(H4DQMSRCDIR)/WaveformFit.o   $(H4DQMSRCDIR)/WaveformUtils.o

USERINCLUDES += -I$(BASEDIR) 
#-I../H4DQM
EXTERNALLIBS = `root-config --glibs` -lMathCore -lMathMore -lz -lm 
CXX          = g++
CXXFLAGS     = -g -Wall `root-config --cflags` -fPIC -O2 -ggdb -std=gnu++0x 
CXXFLAGS    += $(USERINCLUDES)

all: info clean prepare $(OBJFILES) 
#	$(CXX) -shared -fPIC -o $(LIBDIR)/lib$(LIBNAME).so $(SRCDIR)/*.o $(H4DQMSRCDIR)/*.o $(EXTERNALLIBS)
	$(CXX) -shared -fPIC -o $(LIBDIR)/lib$(LIBNAME).so $(SRCDIR)/*.o $(EXTERNALLIBS)
	$(CXX) -o $(BINDIR)/RunH4treeReco $(CXXFLAGS) $(TESTDIR)/RunH4treeReco.cpp $(EXTERNALLIBS) -L$(BASEDIR)/$(LIBDIR) -l$(LIBNAME)

info:
	@echo "--------------------------"
	@echo "Compiling @ $(BASEDIR)"
	@echo "Source are: $(SRCFILES)"
	@echo "--------------------------"

clean:
	$(shell	rm $(SRCDIR)/*.o)
	$(shell	rm $(LIBDIR)/*.so)
	$(shell	rm $(BINDIR)/*)

prepare:
	$(shell mkdir -p $(LIBDIR))
	$(shell mkdir -p $(BINDIR))

$(OBJFILES): | obj

obj:
	@mkdir -p $@

obj/%.o : %.cc
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@




