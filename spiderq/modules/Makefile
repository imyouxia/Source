# Spiderq Makefile
# Copyright (C) 2012-2013 Qteqpid <glloveyp at 163 dot com>
# This file is released under the BSD license, see the COPYING file

SPIDERQ_MAJOR=0
SPIDERQ_MINOR=1
DYLIBSUFFIX=so
DYLIBMAJORSUFFIX=$(SPIDERQ_MAJOR)
DYLIBMINORSUFFIX=$(SPIDERQ_MAJOR).$(SPIDERQ_MINOR)

SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SOURCES))
DYLIBNAMES=$(patsubst %.cpp,%.$(DYLIBSUFFIX),$(SOURCES))


# Fallback to gcc when $CC is not in $PATH.
CXX:=g++
OPTIMIZATION?=-O2 -DNDEBUG
WARNINGS=-Wall
CXXFLAGS=-I../src
REAL_CXXFLAGS=$(OPTIMIZATION) -fPIC $(CXXFLAGS) $(WARNINGS) $(DEBUG) $(PROF)


all: $(DYLIBNAMES) 

#$(DYLIBNAMES): $(OBJS)
%.so:%.o
	$(CXX) -shared -Wl,-soname,$@.$(DYLIBMINORSUFFIX) $(LDFLAGS) $< -o $@ 


.cpp.o:
	$(CXX)  -c $(REAL_CXXFLAGS) $<

clean:
	rm -rf $(DYLIBNAMES) $(OBJS)


# Installation related variables and target
PREFIX:=/etc/spiderq
MODULEDIR=$(PREFIX)/modules

ifeq ($(uname_S),SunOS)
  INSTALL?= cp -r
endif
INSTALL?= cp -af

install: $(DYLIBNAMES) 
	@mkdir -p $(MODULEDIR)
	-$(INSTALL) $^ $(MODULEDIR)

debug:
	$(MAKE) OPTIMIZATION="" DEBUG="-g -ggdb -g3"

gprof:
	$(MAKE) PROF="-pg" 

.PHONY: all clean debug gprof install
