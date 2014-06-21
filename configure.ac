dnl Process this file with autoconf to produce a configure script.
dnl Modified 2005-01-02 for GTK2, 
dnl 2005-11-06 for replacing Gtkglarea by Gtkglext

AC_INIT(geomorph, 0.60.1)
AC_CONFIG_SRCDIR(src/app/main.c)
AM_INIT_AUTOMAKE
LT_INIT

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_LN_S
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_CC_C_O

geomorphdatadir=$datadir/geomorph/$VERSION
AC_SUBST(geomorphdatadir)

AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external])

dnl Checks for libraries.

PKG_CHECK_MODULES(PNG,libpng)
AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)

PKG_CHECK_MODULES(GTK,gtk+-2.0)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES([GTKGLEXT],[gtkglext-1.0])

dnl Jpeg lib Not needed for 0.1x - 0.2x versions of Geomorph
dnl AC_CHECK_LIB(jpeg, jpeg_read_raw_data)
AC_CHECK_LIB(m, cos):
dnl Check for libpng now done with the PKG_CONFIG system
dnl AC_CHECK_LIB(png, png_read_data)
AC_CHECK_LIB(z, gzread)

AC_CHECK_LIB(gtk-x11-2.0, gtk_main)


AC_CHECK_LIB(GL, glLoadIdentity)
AC_CHECK_LIB(GLU, gluLookAt)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_SYS_LONG_FILE_NAMES
AC_C_CONST

dnl Checks for library functions.
AC_CHECK_FUNCS(strerror)

AC_OUTPUT([src/app/Makefile src/utils/Makefile Makefile src/hf/Makefile src/Makefile
	src/fourier/Makefile src/colmap/Makefile scenes/Makefile po/Makefile.in])
	
dnl	scenes/Makefile doc/Makefile po/Makefile.in])

