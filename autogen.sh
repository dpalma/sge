# $Id$
#! /bin/sh
aclocal -I ./config
libtoolize --force --copy
autoheader 
automake --add-missing --copy
autoconf
