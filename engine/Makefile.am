###############################################################################
# $Id$

lib_LTLIBRARIES = libengine.la

EXTRA_DIST = engine.dsp engine.vcproj include/*.h src/*.h

libengine_la_SOURCES = src/3ds.cpp \
                       src/animation.cpp \
                       src/frustum.cpp \
                       src/input.cpp \
                       src/luainterp.cpp \
                       src/mesh.cpp \
                       src/ms3d.cpp \
                       src/ms3dread.cpp \
                       src/ray.cpp \
                       src/scene.cpp \
                       src/scenecamera.cpp \
                       src/sceneentity.cpp \
                       src/sceneentityenum.cpp \
                       src/scenelayer.cpp \
                       src/scenemesh.cpp \
                       src/scriptvar.cpp \
                       src/sim.cpp \
                       src/skeleton.cpp \
                       src/stdhdr.cpp \
                       src/submesh.cpp

libengine_la_LIBADD = $(GL_LIBS) $(CORE_LIBS) -L@top_srcdir@/3rdparty/lua/lib -llua -llualib

INCLUDES = $(GL_CFLAGS) \
           -DENGINE_EXPORTS \
           -I ./include \
           -I ./src \
           -I @top_srcdir@/3rdparty/lua/include \
           -I @top_srcdir@/tech/include \
           -I @top_srcdir@/render/include
