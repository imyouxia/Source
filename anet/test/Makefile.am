AM_CPPFLAGS=-I$(top_srcdir)/src
LDADD=../src/anet/libanet.la -lcppunit -lpthread -ldl

test_sources= sockettf.cpp \
              sockettf.h \
              packetqueuetf.cpp \
              packetqueuetf.h \
              epollsocketeventtf.cpp \
              epollsocketeventtf.h \
              channelpooltf.cpp\
              channelpooltf.h\
              channeltf.cpp\
              channeltf.h\
              connectiontf.cpp\
              connectiontf.h\
              databuffertf.cpp\
              databuffertf.h\
              tcpconnectiontf.cpp\
              tcpconnectiontf.h\
              tcpcomponenttf.cpp\
              tcpcomponenttf.h\
              transporttf.cpp\
              transporttf.h\
              defaultpacketstreamertf.cpp\
              defaultpacketstreamertf.h\
              httprequestandresponsepackettf.cpp\
              httprequestandresponsepackettf.h\
              httppacketstreamertf.cpp\
              httppacketstreamertf.h\
              controlpackettf.cpp\
              controlpackettf.h\
              defaultpackettf.cpp\
              defaultpackettf.h\
              httppackettf.cpp\
              httppackettf.h\
              streamingcontexttf.cpp\
              streamingcontexttf.h\
              httpstreamingcontexttf.cpp\
              httpstreamingcontexttf.h\
              httpstreamertf.cpp\
              httpstreamertf.h\
              maliciousstreamer.cpp\
              maliciousstreamer.h\
              testadapter.h\
              synchronoustf.h\
              synchronoustf.cpp\
              filecontroltf.h\
              filecontroltf.cpp\
              $(NULL)

check_PROGRAMS=dotest
dotest_SOURCES=dotest.cpp $(test_sources)
dotest_LDFLAGS=-static-libtool-libs

##we can add tcmalloc checking in configure.ac
TESTS_ENVIRONMENT=LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64 LD_PRELOAD=/usr/local/lib/libtcmalloc.so HEAPCHECK=normal
TESTS=$(check_PROGRAMS)
