###############################################################################
# $Id$

lib_LTLIBRARIES = libtech.la

EXTRA_DIST = tech.dsp tech.vcproj include/*.h src/*.h

if NO_X
libtech_PLATFORM_SOURCES = src/windoww32.cpp
else
libtech_PLATFORM_SOURCES = src/windowx11.cpp
endif

libtech_la_SOURCES = src/comtools.cpp \
                     src/globalobjreg.cpp \
                     src/config.cpp \
                     src/configstore.cpp\
                     src/fileiter.cpp \
                     src/filepath.cpp \
                     src/filespec.cpp \
                     src/filereadwrite.cpp \
                     src/graphtest.cpp \
                     src/hash.cpp \
                     src/matrix4.cpp \
                     src/memmanager.cpp \
                     src/parse.cpp \
                     src/quat.cpp \
                     src/prime.cpp \
                     src/rand.cpp \
                     src/resmgr.cpp \
                     src/scriptvar.cpp \
                     src/techdebug.cpp \
                     src/techtime.cpp \
                     $(libtech_PLATFORM_SOURCES)

libtech_la_LIBADD = $(GL_LIBS) $(CORE_LIBS)

INCLUDES = $(GL_CFLAGS) -DTECH_EXPORTS -I ./include -I ./src
