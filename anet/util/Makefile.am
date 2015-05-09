AM_CPPFLAGS=-I$(top_srcdir)/src
LDADD=../src/anet/libanet.la -lpthread
AM_LDFLAGS=-static-libtool-libs

noinst_PROGRAMS=echoserver echoclient httpserver helloworld_s helloworld_c httpserver_new helloworld_httpd \
		helloworld_wget helloworld_httpd_2 helloworld_s_2 benchmark helloworld_wget_syn \
		helloworld_c_syn echoclient_syn fork_test wait signal_test

echoserver_SOURCES=echoserver.cpp
echoclient_SOURCES=echoclient.cpp
echoclient_syn_SOURCES=echoclient_syn.cpp
httpserver_SOURCES=httpserver.cpp
httpserver_new_SOURCES=httpserver_new.cpp

helloworld_s_LDFLAGS=
helloworld_s_SOURCES=helloworld_s.cpp
helloworld_c_LDFLAGS=
helloworld_c_SOURCES=helloworld_c.cpp

helloworld_httpd_SOURCES=helloworld_httpd.cpp
helloworld_wget_SOURCES=helloworld_wget.cpp

helloworld_httpd_2_SOURCES=helloworld_httpd_2.cpp
helloworld_s_2_SOURCES=helloworld_s_2.cpp
benchmark_SOURCES=benchmark.cpp

helloworld_wget_syn_SOURCES=helloworld_wget_syn.cpp

helloworld_c_syn_LDFLAGS=
helloworld_c_syn_SOURCES=helloworld_c_syn.cpp

fork_test_SOURCES=fork_test.cpp
wait_SOURCES=wait.cpp
signal_test_SOURCES=signal_test.cpp