Geomorph "à faire"
==================

Mis à jour le 20 septembre 2003

Les priorités peuvent changer, selon les commentaires des utilisateurs.

Prioritaire
-----------
* Tester un algorithme de génération de relief érodé à partir de règles de réécriture

Moyennement prioritaire
-----------------------
* Offrir de rogner / mettre à l'échelle une partie carrée de l'image
* Érosion par l'eau:
	* Améliorer l'algorithme en prenant en considération l'addition des gouttes (reproduire des flux importants comme ceux des riviéres)
* Terrasses:
	* Faire varier la normale de la stratification (rotation de la stratification sur les 2 axes du plan-sol)
* Documentation technique
* Rafraîchissement automatique de la liste de documents des sous-dialogues
* Ajouter des outils de sélection (masques)
* Prévoir une fonction copier / couper / coller (sur les sélections / masques)
* Povray:  permettre de démarrer un rendu à partir d'un fichier INI
* Ondes:
	* Ajouter des ondes circulaires
	* Pour toutes les ondes, proposer un facteur de turbulence
	* Proposer un gradient d'amplitude, radial ou droit, linéaire, quadratique ou autre
* Permettre la variation aléatoire de la longueur d'onde, pas seulement de l'amplitude
* Un facteur de turbulence global
* Développer un navigateur pour l'historique (pile annuler / refaire)

Pas prioritaire
---------------
* Permettre le rognage et la mise à l'échelle lors de la lecture des fichiers
* Transformer le fichier d'options en XLML
* Contrôler la luminosité et le contraste à partir d'un histogramme
* Développer des outils pour la gestion de fichiers INI Povray
* Inscrire le statut par défaut affiché / caché dans "geomorphrc"
* Ajouter une liste des derniers fichiers ouverts dans le menu
* Proposer un pavage en miroir
* Tester d'autres algorithmes de subdivision (par triangles, cercles, hexagones)

Bogues
------
* L'exécution de Povray gèle Geomorph si Povray n'est pas trouvé
		
Points à vérifier, incohérences, options non fonctionnelles
-----------------------------------------------------------
* Translation: l'affichage du document semble rafraîchi deux fois
* Les icônes cacher / montrer (flèches) ne sont pas toujours activées correctement la première fois que le dialogue est montré
* Terminologie dans l'addition de surfaces: remplacer "pente" par "niveau"?
* Les noms de documents dans les listes de documents des sous-dialogues de la boîte à outils ne sont pas toujours uniques
* Empêcher de changer la grandeur de la fenêtre de menu
* "Annuler" après avoir désactivé le pavage dans la fenêtre de document ne rétablit pas la coche dans la case "pavage" (mais la valeur interne est TRUE)
* Concevoir une façon de voir en 2D les très hautes fréquences (amplitudes inférieures à 256 sur une échelle de 0 à 65535)
* Vérifier si la fonction draw_hf_free libère toute la mémoire qu'elle devrait libérer
* Vérifer la fusion avec l'opérateur "soustraction" dans "lisser"

*************************************

Section à consolider
--------------------

Fonctionnalités à ajouter ou examiner
-------------------------------------

* Applicateur de textures
    * Permettre la génération d'un fichier "material map", format PNG indexé
    * Nouvel icône dans le menu outils, à côté du crayon et de l'édition
    * Deux variables de contrôle, trois niveaux chacune:
        * Hauteur:  neige, verdure, rochers/sable (niveau de la mer)
        * Pente:
            * forte = rochers;
            * moyenne = taches de verdure - neige;
            * faible = verdure ou neige, selon la hauteur
    * Traduire ces textures par superposition de couleurs sur l'image principale et sur la prévisualisation Open GL
    * Penser à la sauvegarde et à la lecture de ces images
    * Fournir un fichier .POV de démonstration
* Nouvelles transformations
    * Fissures (glaciers)
* Nouveaux types d'image
    * Pincement en arborescence (simulation de relief érodé)
    * Subdivision triangulaire
* Masque de sélection... possibilité d'implanter couper / coller
* Outil de fusion, découpage, changement d'échelle - à côté du crayon et de la bouteille de chimie
* Exécution POVRAY avec point d'interrogation:  pour choisir le fichier .INI
* Découpage carré, changement d'échelle avec échantillonnage
* Trait uniforme pour le dessin
* Fonctions manquantes:
    * Création et gestion du fichier POVINI
    * Couper / copier / coller / coller spécial:  ==> Retirés dans la v. 0.1
    * Imprimer:  ==> Retiré dans la v. 0.1
* Implanter la reconstruction d'images à partir d'une image arbitraire (?)

BUGS
====

Résolus
-------
* Crevasses: crash sur 2048x2048
* Rotation: bouton +90 ne s'affiche pas - affichage erratique en général.
* Puissance: curseur reste collé - "button_release_event"
* Fourier: curseur reste aussi collé
* Mise à l'échelle des failles: déficiente (non reproductible -probablement réglé avec la correction de l'erreur de segmentation à 2048x2048).
* Oubli d'effacer la chaudière (jaune) quand on change de bouton de groupe de menus édition / transformation
* Erreur de segmentation quand on clique sur la bordure supérieure de la fenêtre
* Rotation: erreur de segmentation lors d'annulations à répétition
* Rotation: à l'envers (inversion sin/cos)
* Rotation: segfault en mode acceptation manuelle, après "undo"..
* Normaliser: valeur fusion source et résultat - à 100% pour la plupart
* Conversion de PNG 8 bits à 16 bits: ne semble plus fonctionner! => utiliser 8 bits seulement* Plus flou: fusion source et résultat: devrait être à 100% résultat
* Fusion source et résultat: réinitialiser les tampons en changeant d'outil
* Fusion: accepter applique 2x la fonction
* Puissance: passage de puissance 1 à 2: HF noir?? - faire remettre à 0!
* Vérifier la réinitialisation des tampons en passant d'un outil à l'autre
* Crayon par défaut: addition plutôt qu'intersection
* Fusion: problème d'acceptation double (semble lié au tampon résultat non vidé)
* Bruit: diminution de l'échelle: semble reprendre le résultat
* Eau: "undo" la réapplique sans changer la case à cocher.
* Dessin: en changeant d'outil, on ne "commit" pas les changements (hf_backup)??
* Acceptation: vider le tampon de résultat (i.e. résultat == source)
* Erreur dans le script install-step1-dir - condition toujours vraie parce que pas d'espaces autour de =

Non Reproductible
-----------------
* Focus des fenêtres: encore des difficultés... modifs non effectuées dans celle qui semble avoir le focus.
* Tester: faire "érosion par gouttes" suivi de "érosion par gravité" - apparemment pas de "commit" sur la transaction - affiche la mauvaise image
* "Abandon" après plusieurs opérations, sans poser de questions sur les fichiers non sauvegardés
- Bug: répéter érosion par gravité

Non-résolus
-----------
* Valider la gestion des changements avec le sous-dialogue Fourier. Les changements devraient être confirmés.
* Fourier - réutilisation du résultat: problème d'affichage
* Dureté: mettre au 0.01 plutôt que 0.1 sur clic
* Événement d'autoacceptation trop sensible dans fusion...
* Erreur de segmentation après "press fault context"
- Erreur de segmentation dans clonage
- Création de répertoire dans "sauver sous"
- Dialogue séparé de fusion (?) - l'actuel n'est vraiment pas intuitif

Messages à ajouter
------------------
* hf.c: "Image converted to 16 bits B&W without alpha under a new name"



