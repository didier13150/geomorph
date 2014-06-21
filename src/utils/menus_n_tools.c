/* menus_n_tools.c - menus and toolbar constructors / destructors
 * One application = a menu / tool bar + a list of documents
 *
 * Copyright (C) 2001 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include "utils.h"
#include "x_alloc.h"
#include "menus_n_tools.h"
// #define _(x) pr(x)

gchar *pr(gchar *x) {
	printf(x);
	return (gchar *) gettext(x);
}

submenu_struct *submenu_new (gint ndx, gchar *label, GtkWidget *bar) {

	// Creates and shows a submenu (to be inserted in a menu bar)
	// ndx is the index in ((command_item_struct *)tools) of the first item in the submenu
	submenu_struct *subm_buf;
// printf("SUBMENU_NEW: Adding label: %s\n",label);
	if (NULL == (subm_buf = (submenu_struct *) x_malloc(sizeof(submenu_struct), "submenu_struct"))) 
		my_msg("Allocation: submenu_struct ???\n",ABORT);
	subm_buf->itemndx = (GList *)NULL;
	subm_buf->itemndx = g_list_append(subm_buf->itemndx, (gpointer) ndx);
	subm_buf->sm_item = gtk_menu_item_new_with_label(label);
	gtk_menu_bar_append (GTK_MENU_BAR (bar), subm_buf->sm_item);
	gtk_widget_show(subm_buf->sm_item);
	subm_buf->sm_menu = gtk_menu_new();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (subm_buf->sm_item), subm_buf->sm_menu);
// printf("Added label: %s\n",label);
	return subm_buf;
}

menu_struct *menu_new(gint nbitems, command_item_struct *items, 
			GtkAccelGroup *accel_grp,
			gpointer callback_data) {

//	Initializes a menu bar and submenus from a command_item_struct table
//	Initializes substructures in menu

	gint i=0;
	gchar *cur_group = "*";
	menu_struct *menu;
	GtkWidget *item_buf;
// printf("MENU_NEW\n");
	
	if (NULL == (menu = (menu_struct *)x_malloc(sizeof(menu_struct), "menu_struct")))
		my_msg("Allocation: menu ???\n",ABORT);

	menu->nbtools = nbitems;
	menu->tools = items;
	menu->submenus = (GList *) NULL;
	menu->bar = gtk_menu_bar_new();
	gtk_widget_show(menu->bar);

	for (i=0; i<nbitems; i++) {

// printf ("Current group: %d - %s / %s, label: %s, tooltip: %s, \n",
//	i, cur_group, (items+i)->group, (items+i)->label, (items+i)->tooltip);

		if ( strcmp(cur_group,(items+i)->group)) {
			// Add an item (submenu) to the menu bar when the label changes
			cur_group = (items+i)->group;
			menu->submenus = 
				g_list_append(menu->submenus,
					(gpointer) submenu_new(i, _(cur_group), menu->bar) );

// printf("Longueur liste:  %d; noeud courant: %d \n",g_list_length(menu->submenus), menu->submenus);
		}

		// Add submenu item to the current submenu
		if ( ! (items+i)->label) {
		// Add a separator
			item_buf = gtk_menu_item_new();	
		}
		else {
			if ((items+i)->iftoggle) {
				item_buf = gtk_check_menu_item_new_with_label(_((items+i)->label));
			}
			else {
				item_buf = gtk_menu_item_new_with_label(_((items+i)->label));	
			}
			gtk_signal_connect (GTK_OBJECT (item_buf), "activate",
				GTK_SIGNAL_FUNC((items+i)->command_callb),callback_data);
		}
		gtk_menu_append (GTK_MENU ( ((submenu_struct*) g_list_last(menu->submenus)->data)->sm_menu),
			item_buf);
		// Keep the indexes for future use (?)
		((submenu_struct *) g_list_last(menu->submenus)->data)->itemndx = 
			g_list_append( ((submenu_struct *) g_list_last(menu->submenus)->data)->itemndx  , (gpointer) i);
		(items+i)->menu_item = item_buf;  // For future use (ex. freeing the widget)
		if ((items+i)->accel)
			gtk_widget_add_accelerator (item_buf, "activate", accel_grp, 
				(items+i)->accel, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
		
		gtk_widget_show (item_buf);

	}
	return menu;
}

void set_toolbar_button_from_label (GtkWidget *tb, gchar *label, gboolean value) {
	GtkToolbarChild *child;
	GList *node, *list;
	gchar *lbl;
	for (node = (GList *)GTK_TOOLBAR(tb)->children; node; node = node->next) {
		child = (GtkToolbarChild *) node->data;
		for (list = gtk_container_children(GTK_CONTAINER(GTK_BIN(child->widget)->child));
			 list ; list = list->next) {
  			if (GTK_IS_LABEL(GTK_OBJECT(list->data)))
				gtk_label_get(GTK_LABEL(list->data), &lbl);
		}
// printf("LABEL: %s; LBL: %s\n",_(label),lbl);
		if (!strcmp(_(label),lbl)) {
//			gtk_widget_activate(GTK_WIDGET(child->widget));
// printf("Setting %s to %d\n",lbl,value);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( child->widget), value);
			return;
		}
	}
}

void set_toolbar_button_from_order (GtkWidget *tb, gint order, gboolean value) {
//	"order" starts at 0
	GtkToolbarChild *child;
	child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(tb)->children,order);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( child->widget), value);
}

GtkWidget *standard_toolbar_new (gint nbitems, command_item_struct *items,
	GtkTooltips *tooltips, GtkWidget *window, gpointer callback_data,
	GtkOrientation tborient, GtkToolbarStyle tbstyle, gboolean ifradio) {
	// 2004-01-16 Variation on toolbar_new
	// More generic
	// Returns a simple GtkWidget - no specialized struct here

	gint i, index_toggled = 0;
	gchar *cur_group;
	GtkWidget *lastitem=NULL, *toolbar;

	cur_group = items->group;
	toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar),tborient);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),tbstyle);
	gtk_widget_show (toolbar);

// printf("TOOLBAR_NEW avant boucle;  nbitems: %d\n",nbitems);
	for (i=0; i<nbitems; i++) {

// printf ("Current group: %s ;", cur_group);

		if ( strcmp(cur_group,(items+i)->group)) {
			// Add double space when menu item in menu bar changes
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar));
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar));
			cur_group = (items+i)->group;
		}
// printf("i: %d\n",i);
		// Add submenu item to the current submenu
		if ( ! (items+i)->label) {
		// Add space as separator
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar));
		}
		else {
			if (ifradio) {
// printf("IFRADIO\nlabel: %s\ntooltip: %s\nwindow: %d\ncommand_callb: %d\ncallb_data: %d\nicon: %d\n",
// (items+i)->label, (items+i)->tooltip,window, (items+i)->command_callb, callback_data, (items+i)->icon);
				(items+i)->toolbar_item = gtk_toolbar_append_element (
					GTK_TOOLBAR(toolbar),
					GTK_TOOLBAR_CHILD_RADIOBUTTON,
					lastitem,
					_((items+i)->label),
					_((items+i)->tooltip),
					NULL,
					create_widget_from_xpm(window,(items+i)->icon),
					GTK_SIGNAL_FUNC((items+i)->command_callb),
					callback_data);
// printf("IFRADIO fin\n");
				lastitem = (items+i)->toolbar_item;
				if ((items+i)->iftoggle )
					index_toggled = i;
			} // if (ifradio)
			else {
				(items+i)->toolbar_item = gtk_toolbar_append_element (
					GTK_TOOLBAR(toolbar),
					(items+i)->iftoggle?GTK_TOOLBAR_CHILD_TOGGLEBUTTON :
						GTK_TOOLBAR_CHILD_BUTTON,
					NULL,
					_((items+i)->label),
					_((items+i)->tooltip),
					NULL,
					create_widget_from_xpm(window,(items+i)->icon),
					GTK_SIGNAL_FUNC((items+i)->command_callb),
					callback_data);
			} // else (ifradio)
		} // else (! (items+i)->label)
	} // for
//	Seg fault with some toolbars, probably because
//	of a callback trying to process a NULL pointer
//	if (ifradio)
//		set_toolbar_button_from_order(toolbar->toolbarwdg, index_toggled, TRUE);

	return toolbar ;
}

toolbar_struct *toolbar_new( gint nbitems, command_item_struct *items,
	GtkTooltips *tooltips, GtkWidget *window, gpointer callback_data,
	GtkOrientation tborient, GtkToolbarStyle tbstyle, gboolean ifradio)  {

	gint i, index_toggled = 0;
	gchar *cur_group;
	GtkWidget *lastitem=NULL;

	toolbar_struct *toolbar;
	if (NULL == (toolbar = (toolbar_struct *) x_malloc(sizeof(toolbar_struct), "toolbar_struct")))
		my_msg("Allocation: toolbar ???\n",ABORT);

	cur_group = items->group;
	toolbar->tborient = tborient;
	toolbar->tbstyle = tbstyle;
	toolbar->ifradio = ifradio;
	toolbar->toolbarwdg = gtk_toolbar_new ();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar->toolbarwdg),tborient);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar->toolbarwdg),tbstyle);
	gtk_widget_show (toolbar->toolbarwdg);

// printf("TOOLBAR_NEW avant boucle;  nbitems: %d\n",nbitems);
	for (i=0; i<nbitems; i++) {

// printf ("Current group: %s ;", cur_group);

		if ( strcmp(cur_group,(items+i)->group)) {
			// Add double space when menu item in menu bar changes
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar->toolbarwdg));
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar->toolbarwdg));
			cur_group = (items+i)->group;
		}
// printf("i: %d\n",i);
		// Add submenu item to the current submenu
		if ( ! (items+i)->label) {
		// Add space as separator
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar->toolbarwdg));
		}
		else {
			if (ifradio) {
// printf("IFRADIO\nlabel: %s\ntooltip: %s\nwindow: %d\ncommand_callb: %d\ncallb_data: %d\nicon: %d\n",
// (items+i)->label, (items+i)->tooltip,window, (items+i)->command_callb, callback_data, (items+i)->icon);
				(items+i)->toolbar_item = gtk_toolbar_append_element (
					GTK_TOOLBAR(toolbar->toolbarwdg),
					GTK_TOOLBAR_CHILD_RADIOBUTTON,
					lastitem,
					_((items+i)->label),
					_((items+i)->tooltip),
					NULL,
					create_widget_from_xpm(window,(items+i)->icon),
					GTK_SIGNAL_FUNC((items+i)->command_callb),
					callback_data);
// printf("IFRADIO fin\n");
				lastitem = (items+i)->toolbar_item;
				if ((items+i)->iftoggle )
					index_toggled = i;
			}
			else
				(items+i)->toolbar_item = gtk_toolbar_append_element (
					GTK_TOOLBAR(toolbar->toolbarwdg),
					(items+i)->iftoggle?GTK_TOOLBAR_CHILD_TOGGLEBUTTON :
						GTK_TOOLBAR_CHILD_BUTTON,
					NULL,
					_((items+i)->label),
					_((items+i)->tooltip),
					NULL,
					create_widget_from_xpm(window,(items+i)->icon),
					GTK_SIGNAL_FUNC((items+i)->command_callb),
					callback_data);
		}
	}
//	Seg fault with some toolbars, probably because
//	of a callback trying to process a NULL pointer
//	if (ifradio)
//		set_toolbar_button_from_order(toolbar->toolbarwdg, index_toggled, TRUE);

	return toolbar ;
}

toolbar_struct *multi_toolbar_new( gint nbitems, command_item_struct *items,
	GtkTooltips *tooltips, GtkWidget *window, gpointer callback_data,
	GtkOrientation tborient, GtkToolbarStyle tbstyle, gboolean ifradio)  {

//	Builds a toolbar OR a box of toolbars,
//	depending on the separators in command_item
//	(we create a new line / column each time we encounter a separator)
	gint i, nbbars=0;
	gchar *cur_group;
	GtkWidget *lastitem=NULL, *tb=NULL, *box=NULL;
	
	toolbar_struct *toolbar;
	if (NULL == (toolbar = (toolbar_struct *) x_malloc(sizeof(toolbar_struct), "toolbar_struct")))
		my_msg("Allocation: toolbar ???\n",ABORT);

	cur_group = items->group;
	toolbar->tborient = tborient;
	toolbar->tbstyle = tbstyle;
	toolbar->ifradio = ifradio;
	tb = gtk_toolbar_new ();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(tb),tborient);
	gtk_toolbar_set_style(GTK_TOOLBAR(tb),tbstyle);
	gtk_widget_show(tb);
	box = tb;

// printf("TOOLBAR_NEW avant boucle;  nbitems: %d\n",nbitems);
	for (i=0; i<nbitems; i++) {

// printf ("Current group: %s ;", cur_group);

		if ( strcmp(cur_group,(items+i)->group)) {
			// Add double space when menu item in menu bar changes
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar->toolbarwdg));
			gtk_toolbar_append_space (GTK_TOOLBAR(toolbar->toolbarwdg));
			cur_group = (items+i)->group;
		}
// printf("i: %d\n",i);
		// Add submenu item to the current submenu
		if ( ! (items+i)->label) {
			// Add space as separator
			// ... 2002.04.14 - Create a new line / column!
			// gtk_toolbar_append_space (GTK_TOOLBAR(toolbar->toolbarwdg));
			if (!nbbars) {
				if (tborient == GTK_ORIENTATION_HORIZONTAL)
					box = gtk_vbox_new(FALSE,0);
				else
					box = gtk_hbox_new(FALSE,0);
				gtk_widget_show(GTK_WIDGET(box));
			}
			nbbars++;
			gtk_box_pack_start(GTK_BOX(box),tb,FALSE,FALSE,0);
			tb = gtk_toolbar_new ();
			gtk_toolbar_set_orientation(GTK_TOOLBAR(tb),tborient);
			gtk_toolbar_set_style(GTK_TOOLBAR(tb),tbstyle);
			gtk_widget_show(tb);
		}
		else {
			if (ifradio) {
// printf("IFRADIO\nlabel: %s\ntooltip: %s\nwindow: %d\ncommand_callb: %d\ncallb_data: %d\nicon: %d\n",
// (items+i)->label, (items+i)->tooltip,window, (items+i)->command_callb, callback_data, (items+i)->icon);
				(items+i)->toolbar_item = gtk_toolbar_append_element (
					GTK_TOOLBAR(tb),
					GTK_TOOLBAR_CHILD_RADIOBUTTON,
					lastitem,
					_((items+i)->label),
					_((items+i)->tooltip),
					NULL,
					create_widget_from_xpm(window,(items+i)->icon),
					GTK_SIGNAL_FUNC((items+i)->command_callb),
					callback_data);
// printf("IFRADIO fin\n");
				lastitem = (items+i)->toolbar_item;
			}
			else
				(items+i)->toolbar_item = gtk_toolbar_append_element (
					GTK_TOOLBAR(tb),
					(items+i)->iftoggle?GTK_TOOLBAR_CHILD_TOGGLEBUTTON :
						GTK_TOOLBAR_CHILD_BUTTON,
					NULL,
					_((items+i)->label),
					_((items+i)->tooltip),
					NULL,
					create_widget_from_xpm(window,(items+i)->icon),
					GTK_SIGNAL_FUNC((items+i)->command_callb),
					callback_data);
		}
	}
	if (!nbbars)
		toolbar->toolbarwdg = tb;
	else {
		gtk_box_pack_start(GTK_BOX(box),tb,FALSE,FALSE,0);
		toolbar->toolbarwdg = box;
	}
	return toolbar ;
}


void menu_free(menu_struct *menu) {

	gint i;
	GList *node;
	// We free the menuitem widgets in the commands table (probably not necessary...)
	for (i=0; i<menu->nbtools; i++) {
		if ((menu->tools+i)->menu_item) 
			gtk_widget_destroy((menu->tools+i)->menu_item);
	}
	
	// We free the menu bar struct
	
	for (node = g_list_nth (menu->submenus,1); node; node = node->next) {
		g_list_free(((submenu_struct*)node->data)->itemndx);
		gtk_widget_destroy( ((submenu_struct*)node->data)->sm_item);
		gtk_widget_destroy( ((submenu_struct*)node->data)->sm_menu);
	}
	g_list_free(menu->submenus);

	// We free the whole structure
	x_free(menu);
}

void toolbar_free(toolbar_struct *toolbar) {
	gint i;
	// We free the menuitem widgets in the commands table
	for (i=0; i<toolbar->nbtools; i++) {
		if ((toolbar->tools+i)->toolbar_item) 
			gtk_widget_destroy((toolbar->tools+i)->toolbar_item);
	}
	
	// Freeing the whole structure
	gtk_widget_destroy(toolbar->toolbarwdg);
	x_free(toolbar);
}


