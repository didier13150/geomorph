geomorph
========

Geomorph est un générateur et un éditeur d'images de relief ("height fields") fonctionnant sous Linux.

C'est une copie de travail de l'application de Patrice St-Gelais <patrstg@users.sourceforge.net>

Introduction
------------

Une image de relief est une sorte de carte topographique artificielle.

C'est une projection 2D d'un paysage 3D.

Geomorph génère des images carrées et montre un aperçu 3D du paysage qui en résulte.

Ces images peuvent être traitées à l'aide d'un logiciel d'illumination comme Povray
("renderer") pour donner un paysage réaliste.

Pour davantage d'informations, voir:
1. ./index.html  dans le répertoire où l'archive de la documentation a été décomprimée,
2. /usr/local/share/geomorph/doc/index.htmlsi la documentation a été installée.
3. http://geomorph.sourceforge.net

Installing Geomorph from the source package
--------------------------------------------------

1. Open a console and untar the archive:

tar zxf geomorph-0.60.tgz

2. Change to the Geomorph directory:

cd geomorph-0.60

3. Build the software:

./bootstrap.sh
./configure
make

4. Logon as "root":

su

5. Install the executable:

make install

6. Leave the root account:

exit

7. Install you as a user:

./install-user

This script creates a /home/.../geomorph directory, a configuration file in this directory and installs shortcuts on the Gnome and KDE desktops.

If you upgrade from 0.2x to 0.3x or 0.4x, you should also update the geomorphrc file:
./update-rc

If you are experimented and want to choose yourself were to create the required structures, you can check the script, create the required directory and files manually, then create a compliant Geomorph configuration file.

8. "make uninstall" as root would uninstall Geomorph.  However, the users won't be uninstalled.


Installing Geomorph from the binary package
--------------------------------------------------

1. Open a console and untar the archive:

tar zxf geomorph-0.60-bin.tgz

2. Change to the Geomorph directory:

cd geomorph-0.60

3. Log on as "root":

su

4. Install the program:

./install-bin

5. Leave the root account:

exit

6. Install you as a user:

./install-user

See the comments for step 7 of the source install process.

7. "./uninstall" as root in the same directory would uninstall Geomorph, but without removing any of the users' directories and configuration files.


Installing the docs
-------------------

The documentation of version 0.30 and later is available separately, in an achive whose name is geomorph-doc-AAAA-MM.tgz.

1. Open a console and untar the archive:

tar zxf geomorph-doc-AAAA-MM.tgz 

2. Change to the Geomorph documentation directory: 

cd geomorph-doc

3. Log on as "root":
 
 su
 
4. Execute the install script:

./install-doc

5. Leave the root account:

exit

The install script copies the files in /usr/local/share/geomorph/doc and tries to modify the "doc_dir" variable in the configuration file "geomorphrc" (if the script can find this file!).

If an older version of the documentation exists in /usr/local/share/geomorph/doc, it is overwritten. The script won't overwrite the installed documentation with an older one. This directory contains a file named "VERSION" with the date of the version, and the script creates here a "stamp" file with the installation date.

