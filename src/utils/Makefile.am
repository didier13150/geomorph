noinst_LIBRARIES = libutils.a
libutils_a_SOURCES = utils.c config_rc_file.c config_dialog.c filesel.c \
	writepng.c menus_n_tools.c readpng.c scale_buttons.c scale_buttons.h \
	config_dialog.h config_rc_file.h filesel.h menus_n_tools.h utils.h \
	x_alloc.h x_alloc.c
INCLUDES =  @GTK_CFLAGS@
LDADD = @GTK_LIBS@ @GTKGLEXT_LIBS@  -lgtkglext-x11-1.0 -lgdkglext-x11-1.0 
CLEANFILES = *~
DISTCLEANFILES = .dep/*.P

