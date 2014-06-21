noinst_LIBRARIES = libfourier.a

libfourier_a_SOURCES = fft.c fft.h fourier.h fourier.c \
	fourier_dialog.h fourier_dialog.c

AM_CPPFLAGS = @GTK_CFLAGS@ @GTKGLEXT_CFLAGS@
LDADD = @GTK_LIBS@ @GTKGLEXT_LIBS@ ../utils/libutils.a
CLEANFILES = *~
DISTCLEANFILES = .dep/*.P
