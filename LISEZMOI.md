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

Installation de Geomorph depuis le paquet source
------------------------------------------------

1. Ouvrir une console et détarrer l'archive:  
`tar zxf geomorph-0.60.tgz`
2. Entrer dans le répertoire Geomorph:  
`cd geomorph-0.60`
3. Construction du logiciel:  
`./bootstrap.sh`  
`./configure`  
`make`  
4. Devenir le super-utilisateur "root":  
`su`
5. Installer le programme:  
`make install`
6. Quitter le compte super-utilisateur root:  
`exit`
7. Installer en temps qu'utilisateur:  
`./install-user`

Ce script créé un répertoire /home/$USER/geomorph, un fichier de configuration dans ce répertoire et install les raccourcis pour les bureaux Gnome and KDE.

Si vous mettez à jour depuis une version 0.2x, 0.3x ou 0.4x, vous devez mettre à jour le fichier geomorphrc:
./update-rc

Si vous êtes un utilisateur expérimenté et que vous voulez choisir le chemin de la structure nécessaire à l'application, vous pouvez éditer le script, créer manuellement le répertoire ainsi que les fichiers nécessaires à l'exécution de l'application, et créer un fichier de configuration adapté pour Geomorph.

`make uninstall` en tant que root désinstallera Geomorph.  Dans tout les cas, le répertoire et les fichiers de Geomorph dans les répertoires utilisateurs ne seront pas effacé.


Installation de Geomorph depuis le paquet binaire
-------------------------------------------------

1. Ouvrir une console et détarrer l'archive:  
`tar zxf geomorph-0.60-bin.tgz`
2. Entrer dans le répertoire Geomorph:  
`cd geomorph-0.60`
3. Devenir le super-utilisateur "root":  
`su`
4. Installer le programme:  
`./install-bin`
5. Quitter le compte super-utilisateur root:  
`exit`
6. Installer en temps qu'utilisateur:  
`./install-user`
Voir les commentaires de l'étape 7 de l'installation depuis les sources.

`make uninstall` en tant que root désinstallera Geomorph.  Dans tout les cas, le répertoire et les fichiers de Geomorph dans les répertoires utilisateurs ne seront pas effacé.


Installing the docs
-------------------

La documentation depuis la version 0.30 et supérieur est disponible séparément dans une archive nommée geomorph-doc-AAAA-MM.tgz.

1. Ouvrir une console et détarrer l'archive:  
`tar zxf geomorph-doc-AAAA-MM.tgz `
2. Entrer dans le répertoire documentation de Geomorph:   
`cd geomorph-doc`
3. Devenir le super-utilisateur "root":  
`su`
4. Exécuter le script d'installation:  
`./install-doc`
5. Quitter le compte super-utilisateur root:  
`exit`

Le script d'installation copie les fichiers dans /usr/local/share/geomorph/doc et essaie de modifier la variable "doc_dir" du fichier "geomorphrc" (si le script peut trouver ce fichier!).

Si une version plus ancienne de la documentation existe dans /usr/local/share/geomorph/doc, elle sera écrasée. Le script n'écrasera la documentation installée avec une version plus ancienne. Ce répertoire contient un fichier "VERSION" avec la date de la version et le script créera un fichier d'horodatage avec la date d'installation.
