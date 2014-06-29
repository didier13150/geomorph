#! /bin/sh

# Trying to install the KDE and GNOME desktop shortcut

VERSION=@GEOMORPH_VERSION@
DATADIR=@DATADIR@
DSKT=$DATADIR/geomorph.desktop

xdg-desktop-icon install --novendor $DSKT

