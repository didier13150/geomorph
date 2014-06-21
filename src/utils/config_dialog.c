/* config_dialog.c - GTK+ dialog for option files utilities
 *
 * Copyright (C) 2002 Patrice St-Gelais
 * patrstg@users.sourceforge.net
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
#include <string.h>
#include <math.h>
#include <glib.h>
#include "./x_alloc.h"
#include "./config_rc_file.h"
#include "./config_dialog.h"
#include "./utils.h"
#include "./filesel.h"

#include "../icons/open.xpm"

#define NB_CONFIG_EXT 2
omenu_list_type config_ext_list[NB_CONFIG_EXT] = {
	{ NULL,0,NULL,gettext_noop("<no extension>"),filetype_sel_callback_fn }, 
	{ NULL,0,NULL,gettext_noop("RC"),filetype_sel_callback_fn } 
};

#define NB_BLANK_EXT 1
omenu_list_type blank_ext_list[NB_BLANK_EXT] = {
	 { NULL,0,NULL,gettext_noop("<no extension>"),filetype_sel_callback_fn }
};

static gchar *SEP="#";

void update_block_page( option_block_type *allowed_block,
			option_block_type *current_block) {
	gint i,j, field_length;
	gchar *value;
	for (i=0; i<allowed_block->nb_options; i++) {
		// 1. Find value in current_opt
		// 2. Update Widget
		// 3. Reset the length of the widget when needed

		for (j = 0; j<current_block->nb_options; j++) {
			if (!strcmp((current_block->option_list+j)->label,
				(allowed_block->option_list+i)->label))
				break;
		}
		if (j==current_block->nb_options) {  // Option not found
			value = (gchar *) x_malloc(1, "gchar (value in config file)");
			value[0] = '\0';
		}
		else
			value = 	(current_block->option_list+j)->value;
//	Required redundancy between allowed_options and current_options
		(allowed_block->option_list+i)->value = value;
		if (GTK_IS_ENTRY((allowed_block->option_list+i)->widget )) {
			field_length = MAX(15,3*strlen(value));
			gtk_entry_set_max_length(GTK_ENTRY((allowed_block->option_list+i)->widget ),
				field_length);
			gtk_widget_set_usize(GTK_WIDGET((allowed_block->option_list+i)->widget ),
				field_length*3,0);	
			gtk_entry_set_text(GTK_ENTRY((allowed_block->option_list+i)->widget), value);
		}
		
		else if (GTK_IS_ADJUSTMENT((allowed_block->option_list+i)->widget )) {
			gtk_adjustment_set_value((allowed_block->option_list+i)->widget , atof(value));
		}
		else if (GTK_IS_OPTION_MENU((allowed_block->option_list+i)->widget )) {
			set_option_menu_from_label ((allowed_block->option_list+i)->widget,
				value);
		}
		else {
			printf(_("No interface widget found for %s - contact the programmer!\n"),(allowed_block->option_list+i)->label);
			return;
		}
	}
}

void update_options(option_file_type *allowed_opt, 
		option_file_type *input_opt) {
//	Updates the widgets with options found in "input_opt"
//	Note for the future:  recreate the display if allowed_opt is NULL
	gint i,bi;
	for (i=0; i<allowed_opt->nb_blocks; i++) {
		bi = get_block_index((allowed_opt->option_block+i)->block_name,
			input_opt);
		update_block_page(allowed_opt->option_block+i,
			input_opt->option_block+i);
	}

}
void open_rc (GtkWidget *wdg, gpointer callb_data) {
	gchar *dname=NULL, *buffer=NULL, *msg=NULL, *title;
	rc_callback_data *data;
	option_file_type *tmp;
	data = (rc_callback_data *) callb_data;
	if (*data->path_n_file)
		split_dir_file(*data->path_n_file, &dname, &buffer, FILESEP);
//	We're supposed to free "buffer" here since it's a copy and it's not required,
//	but it gives a seg fault (???).
	buffer = 	GetFilename(	config_ext_list, 
				NB_CONFIG_EXT,
				_("Open"),
				dname,
				EXISTING_FILE,
				_(config_ext_list[0].lbl));
	if (buffer)
		tmp = load_config_file(buffer,data->allowed_opt,&msg);
	else
		return;
	if (!tmp) {
		my_msg(msg,WARNING);
		return;
	}
	(*data->path_n_file) = buffer;
	free_options_except_values(*data->current_opt_ptr);
	(*data->current_opt_ptr) = tmp;
	title = (gchar *) x_malloc(strlen(*data->path_n_file)+1+strlen(_("Options file %s")), "gchar (Options file title)");
	sprintf(title,_("Options file %s"),*data->path_n_file);
	gtk_window_set_title(GTK_WINDOW(data->window),title);
	update_options(data->allowed_opt,*data->current_opt_ptr);
//	Reinitialize the current options for the application ("transmit" them)
//	Up-to-date options are in data->allowed_opt
	(*data->process_options) (data->allowed_opt);
	data->if_modified = FALSE;
	return;
}

gint save_options (rc_callback_data *data) {
	gchar *msg=NULL;
	save_all_options(*data->path_n_file,
		data->allowed_opt,
		data->save_comments,
		&msg);
	if (msg) {
		my_msg(msg,WARNING);
		return FALSE;
	}
	else {
		(*data->process_options) (data->allowed_opt);
		data->if_modified = FALSE;
		return TRUE;
	}
}

void save_rc_as (GtkWidget *wdg, gpointer callb_data) {
	gchar *dname=NULL, *new_file=NULL, *previous_file=NULL, *title;
	rc_callback_data *data;
	data = (rc_callback_data *) callb_data;
	if (*data->path_n_file)
		split_dir_file(*data->path_n_file, &dname, &new_file, FILESEP);
	new_file = GetFilename(	config_ext_list, 
				NB_CONFIG_EXT,
				_("Save as..."), 
				dname,
				NEW_FILE,
				_(config_ext_list[0].lbl));
	if (!new_file) 
		return;
	previous_file = (*data->path_n_file) ;
	(*data->path_n_file) = new_file;
	if (!save_options(data))
		 (*data->path_n_file) = previous_file;
	title = x_malloc(strlen(*data->path_n_file)+1+strlen(_("Options file %s")),"gchar (Options file title)");
	sprintf(title,_("Options file %s"),*data->path_n_file);
	gtk_window_set_title(GTK_WINDOW(data->window),title);
	return;
}

void save_rc (GtkWidget *wdg, gpointer callb_data) {
	rc_callback_data *data;
	data = (rc_callback_data *) callb_data;
	if (*data->path_n_file)
		save_options(data);
	else
		save_rc_as(wdg,callb_data);
	return;
}

void set_save_comments (GtkWidget *wdg, gpointer callb_data) {
	rc_callback_data *data;
	data = (rc_callback_data *) callb_data;
	data->save_comments = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg));
}

void process_entry (GtkWidget *wdg, gpointer callb_data) {
	option_type *data;
	data = (option_type *)callb_data;
	if (data->value)
		x_free(data->value);
	data->value = gtk_editable_get_chars(GTK_EDITABLE(wdg),0,-1);
}

void switch_modified (GtkWidget *wdg, gpointer callb_data) {
	*((gboolean *) callb_data) = TRUE;
}

void choose_file (GtkWidget *wdg, gpointer callb_data) {	
	gchar *buffer, *buf_msg;
	const gchar *buf_utf8;
	option_type *data;
	data = (option_type *)callb_data;

	buffer = GetFilename(blank_ext_list, 
		NB_BLANK_EXT,
		_("Open"), 
		NULL,
		EXISTING_FILE,
		blank_ext_list[0].lbl);
	if (!buffer)
		return;
	if (directory_exists(buffer)) { // If it's a directory, it's not a file!
		buf_msg = (gchar *) x_malloc(strlen(_("%s is not a file"))+1+strlen(buffer), "const gchar (buf_msg - is not a file in choose config file)");
		sprintf(buf_msg,_("%s is not a file"),buffer);
		my_msg(buf_msg,WARNING);
		return;
	}
	
	buf_utf8 = g_filename_to_utf8(buffer,-1,NULL,NULL,NULL);
	data->value = (gchar *) x_realloc(data->value, strlen(buffer)+1, "gchar - (data->value in config - choose file)");
	strcpy(data->value,buffer);
	gtk_entry_set_text(GTK_ENTRY(data->widget),buf_utf8);
	gtk_widget_set_usize(GTK_WIDGET(data->widget), 
		2.5*MAX(15,3*strlen(buf_utf8)),0);
}

void choose_dir (GtkWidget *wdg, gpointer callb_data) {
	gchar *buffer, *new_buf=NULL;
	const gchar *buf_utf8;
	option_type *data;
	data = (option_type *)callb_data;
	buffer = GetDirname(blank_ext_list, 
		NB_BLANK_EXT,
		_("Open"),
		NULL,
		EXISTING_FILE,
		blank_ext_list[0].lbl);
	if (!buffer)
		return;
	// Directory existence is tested in GetDirname
	new_buf = add_filesep(buffer);
	buf_utf8 = g_filename_to_utf8(new_buf,-1,NULL,NULL,NULL);
	data->value = (gchar *) x_realloc(data->value, strlen(new_buf)+1,"gchar - (data->value in config - choose dir)" );
	strcpy(data->value,new_buf);
	gtk_entry_set_text(GTK_ENTRY(data->widget),buf_utf8);
	gtk_widget_set_usize(GTK_WIDGET(data->widget), 
		2.5*MAX(15,3*strlen(buf_utf8)),0);

	if (new_buf && (new_buf!=buffer) )
		x_free(new_buf);
}

gint get_decimals (option_type *opt) {
	// Try to guess decimals from domain range or current value
	gchar *reference_string, *dec;
	if (strcmp(opt->field_type,"float"))
		return 0;
	if (opt->field_domain)
		reference_string = opt->field_domain;
	else
		if (opt->value)
			reference_string = opt->value;
		else
			return 2; // default
	dec = strrchr(reference_string,'.');
	return MAX(0,strlen(1+dec));
}

void scale_integer (GtkWidget *wdg, gpointer callb_data) {
	//	Update the current variable value with the scale result
	opt_callb_data *data;
	data = (opt_callb_data *)callb_data;
	if (! ((data->opt->value) && (strlen(data->opt->value)>=data->lng)) )
		data->opt->value = (gchar *) x_realloc(data->opt->value,data->lng+1, "gchar (data->opt->value in config - scale_integer)");
	sprintf(data->opt->value,data->format,
		(gint) GTK_ADJUSTMENT(wdg)->value);
}

void scale_float (GtkWidget *wdg, gpointer callb_data) {
	//	Update the current variable value with the scale result
	opt_callb_data *data;
	data = (opt_callb_data *)callb_data;
	if (! ((data->opt->value) && (strlen(data->opt->value)>=data->lng)) )
		data->opt->value = x_realloc(data->opt->value,data->lng+1, "gchar (data->opt->value in config - scale_float)");
	sprintf(data->opt->value,data->format,
		(gfloat) GTK_ADJUSTMENT(wdg)->value);
}

void build_float_format (opt_callb_data *opt) {
	// Deduce format from field domain
	gfloat max;
	max = atof(1+strrchr(opt->opt->field_domain,'-'));
	if (!max)
		max = 1000000000;	// Default
	opt->lng = 1 + (gint) log10(max);
	opt->dec = get_decimals(opt->opt);
	opt->format = (gchar *) x_malloc(10, "gchar (opt->format (float) in config file)");
	sprintf(opt->format,"%%%d.%df",opt->lng,opt->dec);
}

void build_integer_format (opt_callb_data *opt) {
	// Deduce format from field domain
	gint max;
	max = atoi(1+strrchr(opt->opt->field_domain,'-'));
	if (!max)
		max = 1000000000;	// Default
	opt->lng = 1 + (gint) log10(max);
	opt->dec = 0;
	opt->format = (gchar *) x_malloc(10, "gchar (opt->format (integer) in config file)");
	sprintf(opt->format,"%%%dd",opt->lng);
}

GtkObject *build_adjustment (option_type *opt) {
	GtkObject *adj;
	gfloat val, min, max, step_increment, page_increment;
	if (opt->value)
		val = atof(opt->value);
	else
		val = 0.0;
	min = atof(opt->field_domain);
	max = atof(1+strrchr(opt->field_domain,'-'));
	if (!strcmp(opt->field_type,"float")) {
		step_increment = 1.0/MAX(1.0,pow(10.0,get_decimals(opt)));
		page_increment = 1.0;
	}
	else {	// integer
		step_increment = 1.0;
		page_increment = 1.0;
	}
	adj = gtk_adjustment_new (val,
		min, max, 
		step_increment, 
		step_increment, 0.0001);  // page_increment == step_increment, for now...
	return adj;
}

gboolean get_element_index_in_domain (gchar *domain, 
	gchar *element, 
	gchar *sep,
	guint *index) {
	// Get index of "element" in "domain" given separator "sep" used in "domain"
	// Example:
	// Domain = "64#128#256#512"
	// Element = "256"
	// Sep = "#"
	// Index = 2
	gchar *domain_copy, *next;
	domain_copy = (gchar *) x_malloc (1+strlen(domain), "gchar (domain_copy in get_element_index_in_domain)");
	strcpy(domain_copy,domain);
	next = strtok(domain_copy, sep);
	*index = 0;
	while (next) {
		if (!next)
			continue;
		if (!strcmp(next,element)) {
			x_free(domain_copy);
			return TRUE;
		}
		(*index)++;
		next = strtok(NULL,sep);	
	}
	x_free(domain_copy);
	return FALSE;
}

gchar * get_string_from_element_index_in_domain (gchar *domain, 
	gint index, gchar *sep) {
	// Creates a new string corresponding to "index" in "domain", given "sep"
	// Example:
	// Domain = "64#128#256#512"
	// Index = 2
	// Sep = "#"
	// Output string = "256"
	gchar *domain_copy, *next, *output_string;
	gint i;
	domain_copy = (gchar *) x_malloc (1+strlen(domain), "gchar (domain_copy in get_string_from_element_index_in_domain)");
	strcpy(domain_copy,domain);
	next = strtok(domain_copy, sep);
	for (i=0; i<index; i++) {
		if (!next)
			break;
		next = strtok(NULL,sep);
	}
	if (next) {
		output_string = (gchar *) x_malloc(strlen(next)+1, "gchar (output_string in get_string_from_element_index_in_domain)");
		strcpy (output_string, next);
	}
	else {
		output_string = NULL;
	}
	x_free(domain_copy);
	return output_string;
}

void changed_omenu_callb(GtkWidget *wdg, gpointer data)
{
	//	Function connected to the "changed" signal of the option menu
	
	gint index;
	gchar *option_string;
	option_type *opt;
	opt = (option_type *) data;
	
	index = gtk_option_menu_get_history (GTK_OPTION_MENU(wdg));
	if (index==-1)
		return; // No change
	
	if (!index)
		option_string = _("<no value>");
		
	// We get the string for index+1 because we added the "no value" string at
	// the beginning
	option_string = get_string_from_element_index_in_domain (opt->field_domain, index-1, SEP);
		
	if (!option_string) {
		opt->value[0] = '\0';  // Empty string
		return; 
	}
	else {
/**************************************/
// We couldn't free value, because sometimes it's a static char
// but sometimes it is not... so there is a small memory leak to deal with here
//		if (opt->value)
//			x_free(opt->value);
		opt->value = option_string;
	}
}

void changed_switch_modified_omenu (GtkWidget *wdg, gpointer callb_data) {
//	"wdg" is an option menu
//	callb_data is a gboolean*
//	This function must be collected to the "changed" signal
	*((gboolean *) callb_data) = TRUE;
}

void build_option_menu (option_type *opt, gboolean*if_modified_ptr) {
	//	Build an option menu from the field_domain list
	//	Allocate the value field with enough room for any option menu label
	guint lng = 0, index = 0;
	gchar *domain,*next, *txt;
	GSList *group = NULL;
	GtkWidget *menu, *menuitem;

	opt->widget = gtk_option_menu_new();
	menu = gtk_menu_new();
	gtk_option_menu_set_menu (GTK_OPTION_MENU(opt->widget),menu);
	gtk_widget_show(opt->widget);

	menuitem = gtk_radio_menu_item_new_with_label (group,_("<no value>"));
	group = gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menuitem));
	gtk_menu_append(GTK_MENU(menu),menuitem);
	gtk_widget_show(menuitem);
//	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
//		GTK_SIGNAL_FUNC(omenu_callb), (gpointer) opt);

//	gtk_signal_connect (GTK_OBJECT ( menuitem ),
//		"activate", GTK_SIGNAL_FUNC (switch_modified_opt_menu),
//			if_modified_ptr);
	domain = (gchar *) x_malloc(strlen(opt->field_domain)+1, "gchar (domain in build_option_menu)");
	strcpy(domain, opt->field_domain);
	next = strtok(domain, SEP);
	while (next) {
		lng = MIN(lng,strlen(next));
		menuitem = gtk_radio_menu_item_new_with_label (group,next);
		group = gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menuitem));
		gtk_menu_append(GTK_MENU(menu),menuitem);
		gtk_widget_show(menuitem);
//		gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
//			GTK_SIGNAL_FUNC(omenu_callb), (gpointer) opt);
//		gtk_signal_connect (GTK_OBJECT ( menuitem ), 
//			"activate", GTK_SIGNAL_FUNC (switch_modified_opt_menu), 
//				if_modified_ptr);
		next = strtok(NULL, SEP);	
	}
	if (! ((opt->value) && (strlen(opt->value)>=lng)) )
		opt->value = x_realloc(opt->value,lng+1, "gchar (opt->value in build_option_menu)");
	x_free(domain);
	
//	Activate the current item

	if ( (opt->value) && strlen(opt->value)) {
		txt = alltrim(opt->value);
		if (get_element_index_in_domain (opt->field_domain, txt, SEP, &index))
			// We add 1 to index because of the "no value" entry,
			// which is not in the field domain
			gtk_option_menu_set_history (GTK_OPTION_MENU(opt->widget),index+1);
		else	// Invalid value = no value
			gtk_option_menu_set_history (GTK_OPTION_MENU(opt->widget),0);
//		**** Check why it seg faults
//		x_free(txt); 
	}
	else // No value
		gtk_option_menu_set_history (GTK_OPTION_MENU(opt->widget),0);
	gtk_signal_connect (GTK_OBJECT(opt->widget), "changed", GTK_SIGNAL_FUNC (changed_omenu_callb), (gpointer) opt);
	gtk_signal_connect (GTK_OBJECT(opt->widget), "changed", GTK_SIGNAL_FUNC (changed_switch_modified_omenu), if_modified_ptr);
	
}

gint get_widget_type (option_type *opt) {
//	Deduce widget type (depends on field_type, field_domain and
//	current implementation choices ... or laziness)
	if (!strcmp(opt->field_type,"file"))
		return FILE_ENTRY;
	if (!strcmp(opt->field_type,"dir"))
		return DIR_ENTRY;
	if (!opt->field_domain)
		return GENERIC_ENTRY;	
	if ( (!strcmp(opt->field_type,"integer") ) ||
	     (!strcmp(opt->field_type,"float") ) )
		if (strchr(opt->field_domain,'-'))
			return SCALE_ENTRY;
	if (strchr(opt->field_domain,'#'))
		return OPT_MENU;
	return GENERIC_ENTRY;	
}

void create_block_page (option_block_type *allowed_block,
			GtkWidget *notebook,
			GtkWidget *window,
			gboolean *if_modified_ptr) {
	//	Create a new page for the current block
	//	Descriptions and domain validations come from allowed_block
	gint i, field_length, widget_length, widget_type;
	GtkWidget *vbox, *table, *open_icon, *wdg, *scrolled_window;
	GtkObject *adj;
	opt_callb_data *callb_data;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (scrolled_window);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled_window), vbox);

//	define_label_in_box (allowed_block->block_name,
//		vbox, FALSE, TRUE, DEF_PAD);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), 
		scrolled_window, gtk_label_new(_(allowed_block->block_description)));
	
	gtk_widget_set_size_request (scrolled_window, -1, gdk_screen_get_height(gdk_screen_get_default())/2);

	table = gtk_table_new(allowed_block->nb_options, 3, FALSE);
	gtk_widget_show(GTK_WIDGET(table));
	gtk_container_add(GTK_CONTAINER(vbox),GTK_WIDGET(align_widget(table,0.5,0.0)));

	for (i=0; i<allowed_block->nb_options; i++) {
		// Define label and define widget
		widget_type = get_widget_type (allowed_block->option_list+i);
		if ( (allowed_block->option_list+i)->value)
			field_length = 
				MAX(20,3*strlen( (allowed_block->option_list+i)->value));
		else
			field_length = 20;
		widget_length = field_length*3;
		define_label_in_table_align((allowed_block->option_list+i)->field_description,
			table,0, 1, i, i+1, DEF_PAD, ALIGN_LEFT, ALIGN_CENTER);

		if ((widget_type == DIR_ENTRY) || (widget_type == FILE_ENTRY)) {
			widget_length = field_length*2.5;
			field_length = 255;	// Magic number... should be enough for a path!
			open_icon = gtk_button_new();
			gtk_widget_show(GTK_WIDGET(open_icon));
			gtk_container_add(GTK_CONTAINER(open_icon), 
				create_widget_from_xpm(window,(gchar **)open_xpm));
			gtk_table_attach (GTK_TABLE (table), 
				open_icon, 2, 3, i, i+1, 0, 0, DEF_PAD, DEF_PAD);
			if (widget_type == DIR_ENTRY)
				gtk_signal_connect (GTK_OBJECT (open_icon), "clicked",
		       			GTK_SIGNAL_FUNC(choose_dir), 
					allowed_block->option_list+i);
			else
				gtk_signal_connect (GTK_OBJECT (open_icon), "clicked",
		       			GTK_SIGNAL_FUNC(choose_file), 
					allowed_block->option_list+i);
		}
		else if (widget_type == SCALE_ENTRY) {
			callb_data = (opt_callb_data *) x_malloc(sizeof(opt_callb_data), "opt_callb_data");
			callb_data->opt = allowed_block->option_list+i;
			adj = build_adjustment(allowed_block->option_list+i);
			if (adj) {
				(allowed_block->option_list+i)->widget = adj; 
				if (!strcmp((allowed_block->option_list+i)->field_type,"float")) {
					build_float_format (callb_data);
					}
				else {
					build_integer_format (callb_data);
				}
    				wdg = gtk_hscale_new (GTK_ADJUSTMENT (adj));
				gtk_scale_set_digits (GTK_SCALE (wdg), callb_data->dec); 
				gtk_widget_set_usize(wdg,200,0);
				gtk_widget_show(GTK_WIDGET(wdg));
				gtk_table_attach(GTK_TABLE(table), wdg, 1, 2,
					i, i+1, GTK_FILL, GTK_FILL, DEF_PAD, DEF_PAD);

				if (callb_data->dec)
					gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
						GTK_SIGNAL_FUNC(scale_float), 
						(gpointer) callb_data);
				else
					gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
						GTK_SIGNAL_FUNC(scale_integer), 
						(gpointer) callb_data);
				gtk_signal_connect (GTK_OBJECT ( adj ), 
					"value_changed", 
					GTK_SIGNAL_FUNC (switch_modified), 
					if_modified_ptr);
				continue;
			}
		}
		else if (widget_type == OPT_MENU) {
			build_option_menu(allowed_block->option_list+i, if_modified_ptr);
			gtk_table_attach(GTK_TABLE(table), 
					(allowed_block->option_list+i)->widget, 1, 2,
					i, i+1, GTK_FILL, GTK_FILL, DEF_PAD, DEF_PAD);

			continue;
		}
		//	Defaults to GENERIC_ENTRY

		(allowed_block->option_list+i)->widget =
			define_entry_in_table_align((allowed_block->option_list+i)->value,
				table, 1, 2, i, i+1, field_length, widget_length, 
				DEF_PAD, ALIGN_LEFT, ALIGN_CENTER);
		gtk_signal_connect (GTK_OBJECT ( (allowed_block->option_list+i)->widget ), 
			"changed", GTK_SIGNAL_FUNC (process_entry), 
			allowed_block->option_list+i);
		gtk_signal_connect (GTK_OBJECT ( (allowed_block->option_list+i)->widget ), 
			"changed", GTK_SIGNAL_FUNC (switch_modified), 
			if_modified_ptr);
	}
}

void create_block_page_without_dictionary (
			option_block_type *current_block,
			GtkWidget *notebook,
			gboolean *if_modified_ptr) {
	//	Create a new page for the current block
	//	Descriptions and domain validations come from allowed_block
	//	**** The options subsystem cannot be used without dictionary for now!
	//	**** Other functions are incomplete... consistency is not assured
	gint i, field_length;
	gchar *value;
	GtkWidget *vbox, *table, *scrolled_window;
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (scrolled_window);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled_window), vbox);

	if (current_block->block_name)
		gtk_notebook_append_page (GTK_NOTEBOOK(notebook), 
			scrolled_window, gtk_label_new(current_block->block_name));
	else
		gtk_notebook_append_page (GTK_NOTEBOOK(notebook), 
			scrolled_window, gtk_label_new("*"));

	table = gtk_table_new(current_block->nb_options, 2, FALSE);
	gtk_widget_show(GTK_WIDGET(table));
	gtk_container_add(GTK_CONTAINER(vbox),GTK_WIDGET(align_widget(table,0.5,0.0)));

	for (i=0; i<current_block->nb_options; i++) {
		value = 	(current_block->option_list+i)->value;
		define_label_in_table_align((current_block->option_list+i)->label,
			table,0, 1, i, i+1, DEF_PAD, ALIGN_LEFT, ALIGN_CENTER);
		field_length = MAX(15,3*strlen(value));
		(current_block->option_list+i)->widget =
			define_entry_in_table_align(value,table, 
				1, 2, i, i+1, field_length, field_length*3, 
				DEF_PAD, ALIGN_LEFT, ALIGN_CENTER);
		gtk_signal_connect (GTK_OBJECT ( (current_block->option_list+i)->widget ), 
			"changed", GTK_SIGNAL_FUNC (process_entry), 
			current_block->option_list+i);
		gtk_signal_connect (GTK_OBJECT ( (current_block->option_list+i)->widget ), 
			"changed", GTK_SIGNAL_FUNC (switch_modified), 
			if_modified_ptr);
	}
}

gint ok_options_callb(GtkWidget *wdg, gpointer callb_data) {
	gchar *msg;
	rc_callback_data *data;
	data = (rc_callback_data *) callb_data;
	if (data->if_modified) {
		msg = (gchar *) x_malloc(strlen(*data->path_n_file)+strlen(_("<%s> has been modified. Save?"))+1, "const gchar (msg in ok_options_callb)");
		sprintf(msg,_("<%s> has been modified. Save?"),*data->path_n_file);
		if (yes_no(msg,TRUE)) {	
//	Up-to-date options are in data->allowed_opt	
			(*data->process_options) (data->allowed_opt);
			data->if_modified = FALSE;
			save_options(data);
		}
//		const gchar...
//		if (msg)
//			x_free(msg);	
	}
	return FALSE;
}

GtkWidget * options_dialog_new( char **path_n_file,
			option_file_type **current_opt_ptr,
			option_file_type *allowed_opt,
			void (*process_options) (option_file_type *) ) {

//	Builds a standard dialog for an option file, with the specs
//	given in "allowed_opt" and the data given in "current_opt"
//	2002.09.01 change:  data in current_opt is copied in allowed_opt at load time
//	The resulting window is permanent and should be
//	reactivated this way:
//		gtk_widget_show(GTK_WIDGET(returned_window));
//		gtk_grab_add(GTK_WIDGET(returned_window));

//	If "allowed_opt" is null, we display all variables with entry fields,
//	without validation and without comments
//	Now the process...
//	1.	Create a window
//		This window will contain a modal dialog
//		It uses the default OK button of the modal "show-hide" utility
//	2.	Create a notebook
//	3.	Loop on blocks in allowed_opt (or current_opt)
//		3.1 For each block, create a notebook page
//		3.2 For each option in the block, create a widget
//		2005-01-24 generic entry tests not yet implemented
//		Using option menus or adjustment minimize this need
//		... integer -> is_integer test
//		... float -> is_float test (to improve)
//		... file -> filexists test
//		... dir -> direxists test (to build)
//		... char -> no particular test
//		... domain == list:  build an option menu
//		... domain == range / integer or float:  build a scale
//		... domain == range / char or domain == NULL:  simple entry
//		Use generic callbacks, with data == &option->value
//	4.	Insert the notebook in a vbox
//	5.	Append a "save" button, a "save with comments" button
//		and a "load" button to the vbox
//	6.	Embed the vbox in a modal dialog with "modal_dialog_showhide"

	gint i;
	GtkWidget *notebook,*button, *hbox, *vbox;
	static rc_callback_data data;
	option_file_type *current_opt;
	gchar *title;
	
//	Initialize the callback data	
	data.path_n_file = path_n_file;
	data.current_opt_ptr = current_opt_ptr;
	data.allowed_opt = allowed_opt;
	data.save_comments = TRUE;
	data.if_modified = FALSE;
	data.process_options = process_options;

	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL); 
//	We realize the window because we need to define pixmaps in it
	gtk_widget_realize(GTK_WIDGET(data.window));
	title = (gchar *) x_malloc(strlen(*path_n_file)+1+strlen(_("Options file %s")), "gchar (title in options_dialog_new)");
	sprintf(title,_("Options file %s"),*path_n_file);
   
//	Vbox in which we insert the notebook, the 2 "save" and the "open" buttons
//	This widget would be embedded in the modal dialog

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

//	Create a new notebook, place the position of the tabs
 	notebook = gtk_notebook_new ();
    	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_box_pack_start_defaults(GTK_BOX(vbox),notebook);
    	gtk_widget_show(notebook);

//	Dereference the current options before processing them
	current_opt = (option_file_type *) *current_opt_ptr;
//	Loop on blocks and compute each block as a notebook page

	if (allowed_opt)
		for (i=0; i<allowed_opt->nb_blocks; i++) {
			create_block_page(allowed_opt->option_block+i,
				notebook,
				data.window,
				&data.if_modified);
		}
	else
		//	Only display the fields as text entries
		for (i=0; i<current_opt->nb_blocks; i++) {
			create_block_page_without_dictionary(
				current_opt->option_block+i,
				notebook,
				&data.if_modified);
		}
	gtk_notebook_set_page(GTK_NOTEBOOK(notebook),0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label(_("Open"));
	gtk_widget_show(GTK_WIDGET(button));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) open_rc, &data);
	gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label(_("Save"));
	gtk_widget_show(GTK_WIDGET(button));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) save_rc, &data);
	gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label(_("Save as..."));
	gtk_widget_show(GTK_WIDGET(button));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) save_rc_as, &data);
	gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	if (allowed_opt) {
	//	If a dictionary is given, we offer to save descriptions
	//	coming from this dictionary as comments

		button = gtk_check_button_new_with_label(_("Save with comments"));
		gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
		gtk_widget_show(GTK_WIDGET(button));
		gtk_signal_connect (GTK_OBJECT(button), "toggled",
			(GtkSignalFunc) set_save_comments, &data);
	}

 	gtk_box_pack_start (GTK_BOX (vbox), 
		align_widget(hbox,ALIGN_CENTER, ALIGN_CENTER),
		FALSE, FALSE, DEF_PAD);

	modal_dialog_showhide(data.window, vbox, title, ok_options_callb, NULL, &data, 
		GTK_WIN_POS_CENTER, TRUE);

	// Required for GTK 2
	data.if_modified = FALSE;
	
	return data.window;
}


