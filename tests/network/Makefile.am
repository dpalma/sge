###############################################################################
# $Id$

bin_PROGRAMS = network

EXTRA_DIST = *.h *.dsp *.dsw *.vcproj *.sln

network_SOURCES = main.cpp \
                  socket.cpp \
                  thread.cpp

network_LDFLAGS = -L@top_srcdir@/tech \
                  -L@top_srcdir@/allguids

network_LDADD = $(CORE_LIBS) -ltech -lallguids -lpthread

INCLUDES = $(GL_CFLAGS) \
           -I@top_srcdir@/tech/include
