###############################################################################
# $Id$

bin_PROGRAMS=game

EXTRA_DIST = *.h autoexec.lua game.rc game.ico game.dsp game.dsw

if NO_X
game_PLATFORM_SOURCES = syswin.cpp
else
game_PLATFORM_SOURCES = syslinux.cpp
endif

game_SOURCES = 3ds.cpp \
               camera.cpp \
               frustum.cpp \
               groundtiled.cpp \
               guids.cpp \
               heightmap.cpp \
               input.cpp \
               main.cpp \
               mesh.cpp \
               ms3d.cpp \
               raycast.cpp \
               scenegroup.cpp \
               scenenode.cpp \
               script.cpp \
               scriptvm.cpp \
               sim.cpp \
               stripify.cpp \
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
               -L@top_srcdir@/render \
               -L@top_srcdir@/tech \
               -L@top_srcdir@/techguid

if HAVE_RC
RES = game.$(OBJEXT)
game.$(OBJEXT): game.rc
	$(RC) -i $< -o $@
else
RES =
endif

game_LDADD = $(RES) $(CORE_LIBS) $(GL_LIBS) -llua -llualib -ltinyxml -lrender -ltech -ltechguid

INCLUDES = $(GL_CFLAGS) \
           -I@top_srcdir@/3rdparty/lua/include \
           -I@top_srcdir@/3rdparty/tinyxml \
           -I@top_srcdir@/render/include \
           -I@top_srcdir@/tech/include
