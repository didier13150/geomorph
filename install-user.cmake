#! /bin/sh

# Basic script for installing a Geomorph user
# Should be executed under the user account
# Should be executed before the first launch of Geomorph,
# otherwise Geomorph will try to execute the parts
# automatically if the directory or the RC file are not found

VERSION=0.60.1
DATADIR=/usr/local/share/geomorph/$VERSION

if [ -d $HOME/geomorph ]
then
	$DATADIR/install-step1-dir
	exit
fi

$DATADIR/install-step1-dir
$DATADIR/install-step2-rcfile
$DATADIR/install-step3-menu
$DATADIR/install-step4-desktop

# Check Pov-Ray

if [ ! `which povray` ]
then
	echo "********** WARNING **********"
	echo "A 'povray' executable was not found."
	echo "POV-Ray was not installed, or the executable has another name."
	echo "If you want to render the Geomorph scenes with the provided scripts,"
	echo "please be sure that the package is installed globally"
	echo "and for you as a user."
fi


if [ ! -d $HOME/.povray ]
then
	echo "No .povray directory was found in your home directory!"	
	echo "Apparently POV-Ray is not installed for you."
	echo "To execute POV-Ray from Geomorph, please be sure"
	echo "that POV-Ray can find a povray.conf file,"
	echo "and that this file contains the "$HOME" directory"
	echo "in the [Permitted Paths] section."
	echo "This file can be created by using the install script"
	echo "from the original POV-Ray package, with the option user."
else

	if [ ! -f $HOME/.povray/3.6/povray.conf ]
	then
		echo "Assuming you are using POV-Ray 3.6, no povray.conf file"
		echo "was found in "$HOME"/.povray/3.6/ "
		echo "To execute POV-Ray from Geomorph, please be sure"
		echo "that POV-Ray can find a povray.conf file,"
		echo "and that this file contains the "$HOME" directory"
		echo "in the [Permitted Paths] section."
		echo "This file can be created by using the install script"
		echo "from the original POV-Ray package, with the option user."
	fi
fi

