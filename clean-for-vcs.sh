#!/bin/sh

make clean
make distclean
find . -name 'Makefile' -delete
find . -name 'Makefile.in' -delete
rm -rf autom4te.cache m4
rm -f aclocal.m4 compile config.* configure depcomp missing install-sh
