###############################################################################
# $Id$

bin_PROGRAMS=game

EXTRA_DIST = *.h *.lua *.rc *.ico *.dsp *.dsw *.vcproj *.sln

if NO_X
game_PLATFORM_SOURCES = syswin.cpp
else
game_PLATFORM_SOURCES = syslinux.cpp
endif

game_SOURCES = cameracontroller.cpp \
               cmds.cpp \
               groundtiled.cpp \
               heightmap.cpp \
               main.cpp \
               script.cpp \
               ui.cpp \
               uievent.cpp \
               uimgr.cpp \
               uiparse.cpp \
               uirender.cpp \
               uiwidgets.cpp \
               uiwidgutils.cpp \
               $(game_PLATFORM_SOURCES)

game_LDFLAGS = -L$(top_builddir)/3rdparty/lua/src \
               -L$(top_builddir)/3rdparty/lua/src/lib \
               -L$(top_builddir)/3rdparty/tinyxml \
               -L$(top_builddir)/engine \
               -L$(top_builddir)/render \
               -L$(top_builddir)/tech \
               -L$(top_builddir)/allguids

if HAVE_RC
RES = game.$(OBJEXT)
game.$(OBJEXT): $(top_srcdir)/game/game.rc
	cp $(top_srcdir)/game/game.ico $(top_builddir)/game
	$(RC) -i $< -o $@
else
RES =
endif

game_LDADD = $(RES) $(CORE_LIBS) $(GL_LIBS) -llua -llualib -ltinyxml -lrender -lengine -ltech -lallguids

INCLUDES = $(GL_CFLAGS) \
           -I$(top_srcdir)/3rdparty/lua/include \
           -I$(top_srcdir)/3rdparty/tinyxml \
           -I$(top_srcdir)/engine/include \
           -I$(top_srcdir)/render/include \
           -I$(top_srcdir)/tech/include
