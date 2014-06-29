#! /bin/sh

# Trying to install Geomorph in the Gnome or KDE menu

VERSION=@GEOMORPH_VERSION@
DATADIR=@DATADIR@
DSKT=$DATADIR/geomorph.desktop

xdg-desktop-menu install --novendor $DSKT

