ADNSDIR=$(BASEDIR)/../adns
LIBS:=

include $(BASEDIR)/../config.make

UTILS-OBJ:=string.o debug.o url.o connexion.o text.o \
    histogram.o webserver.o PersistentFifo.o hashDup.o mypthread.o
INTERF-OBJ:=input.o useroutput.o output.o
FETCH-OBJ:=site.o sequencer.o hashTable.o checker.o file.o \
	fetchOpen.o fetchPipe.o
MAIN-OBJ:=global.o main.o

ABS-UTILS-OBJ:=utils/string.o utils/debug.o utils/url.o \
    utils/connexion.o utils/text.o utils/histogram.o \
    utils/webserver.o utils/PersistentFifo.o utils/hashDup.o \
    utils/mypthread.o
ABS-INTERF-OBJ:=interf/input.o interf/useroutput.o interf/output.o
ABS-FETCH-OBJ:=fetch/site.o fetch/sequencer.o fetch/hashTable.o \
    fetch/checker.o fetch/file.o fetch/fetchOpen.o fetch/fetchPipe.o
ABS-MAIN-OBJ:=$(MAIN-OBJ)

CFLAGS:=-O3 -Wall -D_REENTRANT
CXXFLAGS:= -Wno-deprecated -Wall -O3 -D_REENTRANT -I- -I$(BASEDIR) -I$(ADNSDIR)
RM:=rm -f

first: all

dep-in:
	makedepend -f- -I$(BASEDIR) -Y *.cc 2> /dev/null > .depend

clean-in:
	$(RM) *.o
	$(RM) *~
	$(RM) *.bak

distclean-in: clean-in
	$(RM) .depend

redo-in: all

debug-in: CXXFLAGS += -g
debug-in: redo-in

prof-in: CXXFLAGS += -pg -DPROF
prof-in: redo-in

include .depend
