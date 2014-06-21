/* utils.c - Personal utilities for Gtk+ development
 *
 * Copyright (C) 2000 Patrice St-Gelais
 * patrstg@users.sourceforge.net
 * www.oricom.ca/patrice.st-gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <png.h>
#include <libintl.h>

#ifndef _UTILS
#define _UTILS 1

#define _(x) (gchar*)gettext(x)
#define gettext_noop(x) (x)

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef CANCEL_YESNO
#define CANCEL_YESNO -1
#endif

#ifndef FILESEP
#define FILESEP '/'
#endif

#define ALIGN_BOTTOM 0.0
#define ALIGN_TOP 1.0
#define ALIGN_LEFT 0.0
#define ALIGN_RIGHT 1.0
#define ALIGN_CENTER 0.5

//	Supposed to be defined in the calling application
// #ifndef DEF_PAD
// #define DEF_PAD 10
// #endif

extern gint DEF_PAD;

#define WARNING 1
#define FAIL 2
#define ABORT 3
#define SORRY 4
#define INFO 5

//	Cosine interpolate pseudo-code from http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
//	Use with float / double values!
/*
     function Cosine_Interpolate(a, b, x)
           ft = x * 3.1415927
           f = (1 - cos(ft)) * .5

           return  a*(1-f) + b*f
     end of function
*/
#define COSINE_INTERPOLATE(a,b,x) ((a)+(0.5-0.5*cos((x)*PI))*((b)-(a)))

typedef struct {

 /* 	void = omenu_struct_type, needed when connecting 
	menuitem signal (inheritance needed...) */

	void *parent_omenu;
	int num;
	GtkWidget *menuitem;
	char *lbl;
	void (*callback_fn) (gpointer user_data, GtkWidget *menu_item);
} omenu_list_type;

typedef struct {
	char *current_lbl;
	GtkWidget *omenu_ptr;
	int max_index;
	int def_index;		// default
	int current_index;	// corresponds to current_lbl
	omenu_list_type *list;
	gpointer data;		// any callback data for the whole menu
} omenu_struct_type;

typedef struct {
	gint (*callb_fn) (GtkWidget *, gpointer);
	gpointer callb_data;
} cb_args_struct;

char *default_dir;

/****************************************************************************/

char * init_default_dir();

int findnextrec(FILE *file, char *rec);

char * remove_spaces_both_ends(char *str);

void my_msg(gchar *message, gint flag_exit);
void my_msg_with_image(gchar *title, gchar *message, GtkWidget *imgwidget);

void modal_dialog(GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer),
			gint (*cancel_callb) (GtkWidget *, gpointer), 
			gpointer, GtkWindowPosition,
			gboolean if_ok_default) ;
 
void modal_dialog_with_titles (GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer),
			gchar *ok_title,
			gint (*cancel_callb) (GtkWidget *, gpointer), 
			gchar *cancel_title,
			gpointer, GtkWindowPosition,
			gboolean if_ok_default) ;
			
void modal_dialog_with_titles_window_provided (GtkWidget *window,
			GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gchar *ok_title,
			gint (*cancel_callb) (GtkWidget *, gpointer),
			gchar *cancel_title,
			gpointer callb_data,
			GtkWindowPosition pos,
			gboolean if_ok_default);
					
void modal_dialog_showhide_with_titles(GtkWidget *window, 
			GtkWidget *wdg, 
			gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gchar *ok_title,
			gint (*cancel_callb) (GtkWidget *, gpointer), 
			gchar *cancel_title,
			gpointer, GtkWindowPosition,
			gboolean if_ok_default) ;
						
void modal_dialog_showhide(GtkWidget *window, GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gint (*cancel_callb) (GtkWidget *, gpointer), 
			gpointer, GtkWindowPosition,
			gboolean if_ok_default) ;

GtkWidget *create_widget_from_xpm (GtkWidget *reference_window, gchar **data);

GtkWidget *define_label_in_table(char *txt,GtkWidget *table, gint colfrom, gint colto, 
	gint linefrom, gint lineto, gint pad);

GtkWidget *define_label_in_table_align(char *txt,GtkWidget *table, gint colfrom, gint colto, 
	gint linefrom, gint lineto, gint pad, gfloat horiz, gfloat vertic);

GtkWidget *define_label_in_box(char *txt, GtkWidget *box, gint expand, gint fill, gint pad);

GtkWidget *define_button_in_table (char *txt, GtkWidget *table, gint colfrom,
	gint colto, gint lineform, gint lineto, gint pad);

GtkWidget *define_check_button_in_table (char *txt, GtkWidget *table, gint colfrom,
	gint colto, gint lineform, gint lineto, gint pad);

GtkWidget * define_check_button_in_box (char *txt, GtkWidget *box, gint expand, gint fill, gint pad);

GtkWidget * define_scale_in_table(GtkObject *adj, GtkWidget *table, gint colfrom, gint colto, 
		gint linefrom, gint lineto, gint digits, gint pad);

GtkWidget * define_scale_in_box(GtkObject *adj, GtkWidget *box,  gint digits, gint pad);

GtkWidget * define_entry_in_table(char *txt,GtkWidget *table, gint colfrom, gint colto,
	gint linefrom, gint lineto, int length, int widget_len, gint pad) ;

GtkWidget * define_entry_in_table_align(char *txt,GtkWidget *table, gint colfrom, gint colto,
	gint linefrom, gint lineto, gint length, gint widget_len, 
	gint pad, gfloat horiz, gfloat vertic) ;

GtkWidget * define_entry_in_box(char *txt, GtkWidget *box, gint expand, gint fill, 
		gint pad, int length, int widget_len);

GtkWidget * define_radio_button_in_box (GtkWidget *box, GSList **group, char *txt,
	void (*callback_fn) (GtkWidget *, gpointer ),
	int if_default );

GtkWidget * define_radio_button_in_box_with_data (GtkWidget *box, GSList **group, char *txt,
	void (*callback_fn) (GtkWidget *, gpointer ),
	gpointer callb_data, int if_default );

GtkWidget * define_radio_button_in_table_with_data (GtkWidget *box, GSList **group, char *txt,
	void (*callback_fn) (GtkWidget *, gpointer ),
	gpointer callb_data, int if_default, gint colfrom, gint colto, gint linefrom, gint lineto );

int create_omenu (omenu_struct_type **new_omenu,
	omenu_list_type *option_list,
	int max_index,
	int default_index);
int create_omenu_with_callb_data (omenu_struct_type **new_omenu,
	omenu_list_type *option_list,
	int max_index,
	int default_index,
	gpointer data);
int find_omenu_option (omenu_struct_type *omenu_str, char *label);
int find_omenu_option_num (omenu_struct_type *omenu_str, int value);
int find_omenu_num (omenu_list_type *option_list, int value, int max);

void set_option_menu_from_label(GtkWidget *omenu, gchar *label);

void set_sensitive_callb (GtkWidget *wdg, gpointer widget);
void unset_sensitive_callb (GtkWidget *wdg, gpointer widget);
void sensitive_if_true(GtkWidget *check_button, gpointer widget);
void unsensitive_if_true(GtkWidget *check_button, gpointer widget);
void show_if_true(GtkWidget *check_button, gpointer widget) ;
void hide_if_true(GtkWidget *check_button, gpointer widget) ;
GtkWidget *align_widget(GtkWidget *, gfloat horizontal, gfloat vertical);
void zero_callb (GtkWidget *wdg, gpointer adj) ;

char *strlow(char *);
char *strupp(char *);
gboolean is_integer(gchar *);
gboolean is_float(gchar *);
gdouble string_to_double (gchar *buf);
int filexists(char *);
int directory_exists(char *);
void split_dir_file(gchar *path_n_file, gchar **dir_name, gchar **file_name, gchar separator);
gchar *get_dir_name(gchar *path_n_file, gchar sep);
gchar *get_file_name(gchar *path_n_file, gchar sep) ;
gchar *add_filesep(gchar *dir_name);
gchar *concat_dname_fname (gchar *dname, gchar *fname);
int yes_no(gchar *, int);
int yes_no_cancel(gchar *, int);

int log2i(gint);

//	Executes a command with execv
//	"com" must contain the full path
//	Returns the PID of the child
int execution(char *com, char **argum) ;
//	Executes a command with execvp
//	Path searched, but it seems it could allow Trojan horses under Linux
int executionp(char *com, char **argum) ;

//	Emits a "dummy" expose event (with a 0x0 area)
void emit_expose_event (GtkWidget *window) ;

//	Prints a GList (prec - data - next for each node)
void print_list (GList *list) ;

//	Toolbar for hiding/showing a dialog (container widget)
GtkWidget *hideshow_dialog_new(GtkWidget *reference_window, 
				GtkOrientation orientation, 
				GtkWidget *wdg_to_show_hide, 
				gboolean *show_flag);

// Simple hide/show callbacks
void hide_callb(GtkWidget *wdg, gpointer data); 
void show_callb(GtkWidget *wdg, gpointer data); 

//	File types handling
void write_png(char *file_name, int depth, unsigned char *image_buf, int width, int height);
void write_png_with_text(char *file_name, int depth, unsigned char *image_buf,
	int width, int height, png_text* text, int nbkeys);
gboolean read_png_header(gchar *file_name,  gint *depth,
		gint *width, gint *height, gint *color_type, gint *interlace_type) ;
gboolean read_png(gchar *file_name, gint *depth, unsigned char **image_buf,
		gint *width, gint *height, gint *color_type, gint *interlace_type);
gboolean read_png_with_text(gchar *file_name, gint *depth, unsigned char **image_buf,
		gint *width, gint *height, gint *color_type,
		gint *interlace_type, png_text **text, gint *nbkeys);
void free_png_text (png_text **txt, gint nbkeys);

//	Only to economize some boring code
GtkWidget *frame_new(gchar *label, gint pad) ;

//	"Universal" callbacks
gint gint_adj_callb (GtkWidget *wdg, gpointer data);
gint gfloat_adj_callb (GtkWidget *wdg, gpointer data);
gint gdouble_adj_callb (GtkWidget *wdg, gpointer data);
gint toggle_check_button_callb (GtkWidget *checkbutton, gpointer data);
gint gboolean_set_true (GtkWidget *wdg, gpointer data);
gint gboolean_set_false (GtkWidget *wdg, gpointer data);
gint gboolean_set_true_event (GtkWidget *wdg, GdkEventButton *, gpointer data);
gint gboolean_set_false_event (GtkWidget *wdg, GdkEventButton *, gpointer data);

//	Obvious...
void swap_buffers (gpointer *a, gpointer *b);
void order_int (gint *i1, gint *i2);

void synchro_scr_win (GtkWidget *scr_win1, GtkWidget *scr_win2);
gint adjust_adj_callb (GtkAdjustment *adj_trigger, gpointer adj_toadjust);

GtkWidget *xpm_label_box(GtkWidget *window, gchar **xpm, gchar *label_text );

#endif  // _UTILS



