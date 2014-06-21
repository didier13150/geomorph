geomorph
========

Height field  generator and editor for the Linux operating system.

It's a working copy from Patrice St-Gelais's application <patrstg@users.sourceforge.net>

Introduction
------------

A height field is a kind of topographic map.  It is a 2D projection of a 3D landscape.

Geomorph generates square images and shows a 3D preview of the resulting landscape.

These 2D images can be processed with a tool like Povray for rendering the landscape.

For more details, see:
1. ./index.html in the directory where the documentation archive is unpacked.
2. /usr/local/share/geomorph/doc/index.html if the documentation is installed.
3. http://geomorph.sourceforge.net

Installing Geomorph from the source package
--------------------------------------------------

1. Open a console and untar the archive:  
`tar zxf geomorph-0.60.tgz`
2. Change to the Geomorph directory:  
`cd geomorph-0.60`
3. Build the software:  
`./bootstrap.sh`  
`./configure`  
`make`  
4. Logon as "root":  
`su`
5. Install the executable:  
`make install`
6. Leave the root account:  
`exit`
7. Install you as a user:  
`./install-user`

This script creates a /home/.../geomorph directory, a configuration file in this directory and installs shortcuts on the Gnome and KDE desktops.

If you upgrade from 0.2x to 0.3x or 0.4x, you should also update the geomorphrc file:
./update-rc

If you are experimented and want to choose yourself were to create the required structures, you can check the script, create the required directory and files manually, then create a compliant Geomorph configuration file.

`make uninstall` as root would uninstall Geomorph.  However, the users won't be uninstalled.


Installing Geomorph from the binary package
--------------------------------------------------

1. Open a console and untar the archive:  
`tar zxf geomorph-0.60-bin.tgz`
2. Change to the Geomorph directory:  
`cd geomorph-0.60`
3. Log on as "root":  
`su`
4. Install the program:  
`./install-bin`
5. Leave the root account:  
`exit`
6. Install you as a user:  
`./install-user`
See the comments for step 7 of the source install process.

`./uninstall` as root in the same directory would uninstall Geomorph, but without removing any of the users' directories and configuration files.


Installing the docs
-------------------

The documentation of version 0.30 and later is available separately, in an achive whose name is geomorph-doc-AAAA-MM.tgz.

1. Open a console and untar the archive:  
`tar zxf geomorph-doc-AAAA-MM.tgz `
2. Change to the Geomorph documentation directory:   
`cd geomorph-doc`
3. Log on as "root":  
`su`
4. Execute the install script:  
`./install-doc`
5. Leave the root account:  
`exit`

The install script copies the files in /usr/local/share/geomorph/doc and tries to modify the "doc_dir" variable in the configuration file "geomorphrc" (if the script can find this file!).

If an older version of the documentation exists in /usr/local/share/geomorph/doc, it is overwritten. The script won't overwrite the installed documentation with an older one. This directory contains a file named "VERSION" with the date of the version, and the script creates here a "stamp" file with the installation date.

