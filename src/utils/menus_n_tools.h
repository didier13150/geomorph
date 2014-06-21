/* menu_n_tools.h - headers for menus and toolbars
 * One application = a menu / tool bar + a list of documents
 *
 * Copyright (C) 2001 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         www.oricom.ca/patrice.st-gelais
 *gram is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _MENU_N_TOOLS
#define _MENU_N_TOOLS 1

#include <gtk/gtk.h>

// Common structures used to build menus and toolbars
typedef struct {
	gchar *group;
	gchar *label;
	gchar *tooltip;
	gchar accel;
	gchar **icon; // xpm
	GdkCursorType cursor_type; // Default:  GDK_LEFT_PTR
	void (*command_callb) (GtkWidget *, gpointer);
	GtkWidget *menu_item;
	GtkWidget *toolbar_item;
	gboolean iftoggle; // TRUE => GtkCheckedMenuItem or toggle button in a toolbar
} command_item_struct;

//	For GList in menu_struct

typedef struct {
	GList *itemndx;  	// list of index in menu_struct ->tools
	GtkWidget *sm_item;  	// Menu item Widget (submenu in the menu bar)
	GtkWidget *sm_menu;	// Submenu Widget (which is set to sm_item)
} submenu_struct;

typedef struct {
	gint nbtools;
	command_item_struct *tools;
	GList *submenus;  	// list of (submenu_struct *)
	GtkWidget *bar; 		// Menu bar widget
} menu_struct;

typedef struct {
	gint nbtools;
	command_item_struct *tools;
	gboolean ifradio;		// TRUE:  the bar contains radio buttons
	GtkToolbarStyle tbstyle;
	GtkOrientation tborient;
	GtkWidget *toolbarwdg;	// Tool bar widget
} toolbar_struct;

// Prototypes

menu_struct *menu_new(gint , command_item_struct *, 
	GtkAccelGroup *, gpointer );
void menu_free(menu_struct *);

toolbar_struct *toolbar_new(gint , command_item_struct *, GtkTooltips *, 
	GtkWidget *, gpointer , GtkOrientation, GtkToolbarStyle, gboolean);

toolbar_struct *multi_toolbar_new(gint , command_item_struct *, GtkTooltips *, 
	GtkWidget *, gpointer , GtkOrientation, GtkToolbarStyle, gboolean);

GtkWidget *standard_toolbar_new (gint nbitems, command_item_struct *items,
	GtkTooltips *tooltips, GtkWidget *window, gpointer callback_data,
	GtkOrientation tborient, GtkToolbarStyle tbstyle, gboolean ifradio);

void set_toolbar_button_from_label (GtkWidget *tb, gchar *label, gboolean value) ;
void set_toolbar_button_from_order (GtkWidget *tb, gint order, gboolean value) ;

void toolbar_free(toolbar_struct *);

#endif // _MENU_N_TOOLS










