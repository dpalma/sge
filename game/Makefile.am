###############################################################################
# $Id$

bin_PROGRAMS = game

EXTRA_DIST = *.rc *.ico *.dsp *.dsw *.vcproj *.sln *.h

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
               $(game_PLATFORM_SOURCES)

game_LDFLAGS = -L$(top_builddir)/3rdparty/lua/src \
               -L$(top_builddir)/3rdparty/lua/src/lib \
               -L$(top_builddir)/3rdparty/tinyxml \
               -L$(top_builddir)/3rdparty/zlib/contrib/minizip \
               -L$(top_builddir)/engine \
               -L$(top_builddir)/render/gl \
               -L$(top_builddir)/tech \
               -L$(top_builddir)/allguids

if HAVE_RC
RES = game.$(OBJEXT)
game.$(OBJEXT): $(top_srcdir)/game/game.rc
	if test "$(top_srcdir)" != "$(top_builddir)"; then \
		cp $(top_srcdir)/game/game.ico $(top_builddir)/game; \
	fi;
	$(RC) -i $< -o $@
else
RES =
endif

install-exec-hook:
	$(ECHO) "data=$(DESTDIR)$(prefix)/data" > $(DESTDIR)$(bindir)/game.cfg

game_LDADD = $(RES) $(CORE_LIBS) $(GL_LIBS) -lminizip -llua -llualib -ltinyxml -lrendergl -lengine -ltech -lallguids

INCLUDES = $(GL_CFLAGS) \
           -I$(top_srcdir)/3rdparty/lua/include \
           -I$(top_srcdir)/3rdparty/tinyxml \
           -I$(top_srcdir)/engine/include \
           -I$(top_srcdir)/render/include \
           -I$(top_srcdir)/tech/include
