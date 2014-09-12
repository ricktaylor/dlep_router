#! /bin/sh
echo Running autotools...
mkdir -p build-aux
aclocal && \
autoheader && \
automake --foreign --add-missing && \
autoconf
