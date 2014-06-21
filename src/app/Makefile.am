bin_PROGRAMS = geomorph
geomorph_SOURCES = app.c main.c document.c doctype.c thisappinit.c \
	app.h doc.h globals.h main.h thisappinit.h stack.h stack.c
geomorph_CPPFLAGS =  @GTK_CFLAGS@ @GTKGLEXT_CFLAGS@
LDADD = @GTK_LIBS@ @GTKGLEXT_LIBS@ ../hf/libhf.a ../fourier/libfourier.a ../utils/libutils.a -lGL -lGLU -lpng  -lgtkglext-x11-1.0 -lgdkglext-x11-1.0 
DEPS = ../hf/libhf.a ../fourier/libfourier.a ../utils/libutils.a
CLEANFILES = *~ geomorph *.o
DISTCLEANFILES = .dep/*.P

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" -DGEOMORPHDATADIR=\"$(geomorphdatadir)\" @DEFS@


