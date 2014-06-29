#! /bin/sh

# Installing a Geomorph user, part 1:  the directory with default POV scenes

VERSION=0.60.1
DATADIR=/usr/local/share/geomorph/$VERSION
SOURCE_DIR=$DATADIR/scenes
CURDIR=`pwd`

cd ${SOURCE_DIR}

LSCN=$(ls -F -1 *.pov)
LINC=$(ls -F -1 *.inc)

cd ${CURDIR}

if [ -d $HOME/geomorph ]
then
# Directory exists, rename old scenes before copying

# Behaviour not implemented: If it's a second update for the same version, we don't create a _old file

# Implemented: we create a _old file only if files are different

	if [ -f $HOME/geomorph/.version ]
	then
		OLD_VERSION=`cat $HOME/geomorph/.version`
	else
		OLD_VERSION=0.01
	fi

#	V=$((${VERSION/\./0}))
# 	OV=$((${OLD_VERSION/\./0}))

	for SCN in $LSCN
	do
		if [ -f $HOME/geomorph/$SCN ]
		then	
			if (! cmp $HOME/geomorph/$SCN $SOURCE_DIR/$SCN) ; then
			BNAME=$(basename $SCN .pov)
#			if [ ${V} -gt ${OV} ] ; then
				echo $BNAME'.pov -> '$BNAME'_old.pov'
				mv -f $HOME/geomorph/$SCN $HOME/geomorph/$BNAME'_old.pov'
			fi
		fi
		cp -f $SOURCE_DIR/$SCN $HOME/geomorph/$SCN

	done
	for INC in $LINC
	do
		if [ -f $HOME/geomorph/$INC ]
		then
			if [ ${INC} = "global_settings.inc" ] ; then
				continue
			fi
			if (! cmp $HOME/geomorph/$INC $SOURCE_DIR/$INC) ; then
			BNAME=$(basename $INC .inc)
#			if [ ${V} -gt ${OV} ] ; then
				echo $BNAME'.inc -> '$BNAME'_old.inc'
				mv -f $HOME/geomorph/$INC $HOME/geomorph/$BNAME'_old.inc'
			fi
		fi
		cp -f $SOURCE_DIR/$INC $HOME/geomorph/$INC

	done
else
# Directory does not exist

	mkdir -v $HOME/geomorph
	cp -fv $SOURCE_DIR/* $HOME/geomorph
	
	if [ -f $HOME/.povray/3.6/povray.ini ]
	then
		A=`grep Display_gamma $HOME/.povray/3.6/povray.ini | wc -l`
		if [ $A -gt 0 ] ; then
			echo `grep Display_gamma $HOME/.povray/3.6/povray.ini | sed s/Display_gamma/global_settings/ | sed s/\$/\}/ | sed s/=/\ {assumed_gamma\ / | sed s/\;//` > $HOME/geomorph/global_settings.inc
		fi
	fi
fi

# Added 2008-03-28 - We create a geomorph/tmp directory
# for animations and the like

if [ ! -d $HOME/geomorph/tmp ]
then
	mkdir -v $HOME/geomorph/tmp
fi

echo $VERSION > $HOME/geomorph/.version

