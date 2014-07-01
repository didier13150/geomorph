Geomorph TODO list
==================

See also NOTES (French only, sorry)
and news page on http://geomorph.sourceforge.net

Updated 2008-01
Things done from 2003 have been removed

Todo
----
* Test a new generation algorithm: eroded terrain from a branching structure
* Add simple square cropping / scaling
* Water erosion:
    * Improve the algorithm by considering the addition of the drops (reproduce huge flows like rivers)
* Terraces:
    * Allow to change the normal of the stratification (rotate the transformation on both axis of the ground plane)
* Technical documentation
* Automatically refresh document lists in subdialogs
* Add selection masks / tools
* Add copy / cut / paste
* Povray: allow to choose (render from) INI files
* Waves:
    * Add circular waves
    * For all waves, propose a turbulence factor (warp or randomise the waves)
    * Propose an amplitude gradient (should be radial for circular waves), linear or quadratic or else
* Implement a navigator for the undo/redo stack
* Allow scaling and cropping when reading a file.
* Migrate the options file to XML.
* Use an histogram to facilitate interaction in the brigthness / contrast subdialog.
* Develop tools for managing Povray INI files.
* Control default hiding / showing of subdialogs in "geomorphrc"
* Add a "recent files" list in the menu.
* Propose a "mirror tiling"
* Test other subdivision algorithms (triangular, circular, hexagonal)

Bugs
----
* Executing Povray freezes Geomorph when the Povray program doesn't exist

Things to check, inconsitencies, features not working
-----------------------------------------------------
* Translate: the document drawing area seems to be refreshed twice
* Placement of the "Apply" or "Accept" buttons: should they be always at the window's bottom?
* Check if the hide/show arrows are always correctly toggled, when a dialog is displayed for the first time.
* Wording, in surface sum generator: slope == level?
* Check if these options should be added in "geomorphrc":
    * Draw pen wrap on/off
    * Default pen level
    * Default pen merge operator (+/x)
* In document lists in tools subdialogs, sometimes document names are not unique
* Forbid size change of the menu window
* "Undo" after unchecking "tiling" (wrap) in the document window does not check "tiling" (but the internal value of "tiling" is TRUE).
* The accept callback should remove the tiling parameter after some processes like rotate, if not a multiple of 90 degrees, or honeycombs.
* Design a way to see or temporarily boost the very high frequencies (amplitudes less than 256 on an 0-65535 scale)
* Check if draw_hf_free frees all the memory it should free
* Check merging with "subtract" in the "smooth" tool dialog
