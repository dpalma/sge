###############################################################################
# $Id$

# Don't require GNU-convention files like README, NEWS, ChangeLog, AUTHORS
AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I ./config

EXTRA_DIST = bootstrap data

dist-hook:
	rm -rf `find $(distdir)/data -name CVS`

SUBDIRS = 3rdparty allguids tech render engine game

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure

SUPERCLEANFILES = */Makefile.in \
                  config.log \
                  config.status \
                  libtool \
                  config/compile \
                  config/config.guess \
                  config/config.h.in \
                  config/config.sub \
                  config/depcomp \
                  config/install-sh \
                  config/ltmain.sh \
                  config/missing \
                  config/mkinstalldirs \
                  config/stamp-h1

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# completely get rid of all the files produced by autoheader,
# automake, autoconf, aclocal, and libtool
super-clean: maintainer-clean
	rm -f $(SUPERCLEANFILES)
