#!/bin/sh
# $Id$

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to build this project."
	exit 1
}

(libtool --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have libtool installed to build this project."
	exit 1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have automake installed to build this project."
	exit 1
}

aclocal -I ./config \
&& libtoolize --force --copy \
&& autoheader \
&& automake --add-missing --copy \
&& autoconf
