###############################################################################
# $Id$

bin_PROGRAMS=game

EXTRA_DIST = *.h *.lua game.rc game.ico game.dsp game.dsw game.vcproj game.sln

if NO_X
game_PLATFORM_SOURCES = syswin.cpp
else
game_PLATFORM_SOURCES = syslinux.cpp
endif

game_SOURCES = cameracontroller.cpp \
               frustum.cpp \
               groundtiled.cpp \
               heightmap.cpp \
               input.cpp \
               main.cpp \
               scenecamera.cpp \
               scenemesh.cpp \
               scenenode.cpp \
               script.cpp \
               scriptvm.cpp \
               ui.cpp \
               uievent.cpp \
               uimgr.cpp \
               uiparse.cpp \
               uirender.cpp \
               uiwidgets.cpp \
               uiwidgutils.cpp \
               $(game_PLATFORM_SOURCES)

game_LDFLAGS = -L@top_srcdir@/3rdparty/lua/lib \
               -L@top_srcdir@/3rdparty/tinyxml \
               -L@top_srcdir@/engine \
               -L@top_srcdir@/render \
               -L@top_srcdir@/tech \
               -L@top_srcdir@/allguids

if HAVE_RC
RES = game.$(OBJEXT)
game.$(OBJEXT): game.rc
	$(RC) -i $< -o $@
else
RES =
endif

game_LDADD = $(RES) $(CORE_LIBS) $(GL_LIBS) -llua -llualib -ltinyxml -lrender -lengine -ltech -lallguids

INCLUDES = $(GL_CFLAGS) \
           -I@top_srcdir@/3rdparty/lua/include \
           -I@top_srcdir@/3rdparty/tinyxml \
           -I@top_srcdir@/engine/include \
           -I@top_srcdir@/render/include \
           -I@top_srcdir@/tech/include
