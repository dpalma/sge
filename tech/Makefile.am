###############################################################################
# $Id$

lib_LTLIBRARIES = libtech.la

EXTRA_DIST = *.dsp *.vcproj include/*.h src/*.h

if NO_X
libtech_PLATFORM_SOURCES = src/windoww32.cpp
else
libtech_PLATFORM_SOURCES = src/windowx11.cpp
endif

libtech_la_SOURCES = src/bmp.cpp \
                     src/comtools.cpp \
                     src/dictionary.cpp \
                     src/dictionarystore.cpp \
                     src/globalobjreg.cpp \
                     src/config.cpp \
                     src/fileiter.cpp \
                     src/filepath.cpp \
                     src/filespec.cpp \
                     src/filereadwrite.cpp \
                     src/graphtest.cpp \
                     src/hash.cpp \
                     src/imagedata.cpp \
                     src/matrix4.cpp \
                     src/memmanager.cpp \
                     src/parse.cpp \
                     src/quat.cpp \
                     src/prime.cpp \
                     src/rand.cpp \
                     src/resmgr.cpp \
                     src/techdebug.cpp \
                     src/techtime.cpp \
                     src/tga.cpp \
                     $(libtech_PLATFORM_SOURCES)

libtech_la_LIBADD = $(GL_LIBS) $(CORE_LIBS)

INCLUDES = $(GL_CFLAGS) \
           -DTECH_EXPORTS \
           -I$(top_srcdir)/tech/include \
           -I$(top_srcdir)/tech/src \
           -I$(top_srcdir)/3rdparty/zlib/include
