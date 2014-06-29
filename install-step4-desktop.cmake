#! /bin/sh

# Trying to install the KDE and GNOME desktop shortcut

VERSION=0.60
DATADIR=/usr/local/share/geomorph/$VERSION
DSKT=$DATADIR/geomorph.desktop

xdg-desktop-icon install --novendor $DSKT

