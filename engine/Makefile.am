###############################################################################
# $Id$

lib_LTLIBRARIES = libengine.la

EXTRA_DIST = engine.dsp engine.vcproj include/*.h src/*.h

libengine_la_SOURCES = src/3ds.cpp \
                       src/mesh.cpp \
                       src/ms3d.cpp \
                       src/stdhdr.cpp

libengine_la_LIBADD = $(GL_LIBS)

INCLUDES = $(GL_CFLAGS) \
           -DTECH_EXPORTS \
           -I ./include \
           -I ./src \
           -I @top_srcdir@/tech/include \
           -I @top_srcdir@/render/include
