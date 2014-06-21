
SUBDIRS = m4  src po scenes

localedir = $(datadir)/locale

geomorphdata_DATA =   geomorphrc* LISEZMOI README FAQ FAQ-fr TODO AFAIRE splash.jpg

geomorphdata_SCRIPTS = install-user install-step* update-rc v0_30_new_sections

pkgdata_DATA = *.xpm *.desktop

DEFS = -DLOCALEDIR=\"$(localedir)\"-DGEOMORPHDATADIR=\"$(geomorphdatadir)\" @DEFS@

EXTRA_DIST = config.rpath  $(geomorphdata_DATA) $(pkgdata_DATA) $(geomorphdata_SCRIPTS)

ACLOCAL_AMFLAGS = -I m4
