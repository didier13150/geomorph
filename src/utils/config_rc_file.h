/* config_rc_file.h - headers for utilities processing an options file
 *
 * Copyright (C) 2002 Patrice St-Gelais
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _CONFIG_RC_FILE
#define _CONFIG_RC_FILE 1

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BLOCK_NOT_FOUND
#define BLOCK_NOT_FOUND -1
#endif

#ifndef FILESEP
#define FILESEP '/'
#endif

#include <stdio.h>

typedef struct {
	char *label;
	char *value;  // A char, whatever field_type it is
	void * widget; // A slot for the corresponding interface element
		// Generally a GtkWidget* (entry or option_menu)
		// Could also be a GtkObject*, like an adjustment in a scale
//	Variables used when option_type* defines an options directory
	char *field_type;	
		// Can be any of "dir", "file","integer","float","char"
	char *field_domain;
		// Syntax for choice in a list:
		//	"64#128#256"	... an option menu
		//	"blue#green#red" ... an option menu
		// Syntax for range:
		// 	"1-10" 		... a scale
		//	"1.0-2.0"	... a scale
		//	"apple-zebra"	... a validated entry (not yet implemented)
		// NULL for field_type "file" or "dir"
		//		... an entry besides an "open file" icon
	char *field_description;
} option_type;

typedef struct {
	char *block_name;
	char *block_description;
	int nb_options;
	option_type *option_list;
} option_block_type;

typedef struct {
	int nb_blocks;
	option_block_type *option_block;
} option_file_type;

char *alltrim(char *);
int create_block(char *, char *, 
	option_file_type *,option_file_type *);
void create_option(char *, option_block_type * current_block,
 		option_block_type *allowed_options_block);
void load_options(FILE *, option_file_type *all_options,
		option_file_type *allowed_options);
void print_options(FILE *, option_file_type *);
void print_allowed_options(FILE *fp,
			option_file_type *allowed_options,
			int if_comments);

void free_options(option_file_type *);
void free_options_except_values(option_file_type *);

int get_block_index (char *block_label, option_file_type *opt) ;
char * get_option (option_file_type *all_options, char *block_label, char *option_label);

void save_config_file(char *output_file_name, option_file_type *opt,
		char **msg) ;
void save_all_options(char *output_file_name, 
		option_file_type *allowed_opt,
		int if_comments,
		char **msg) ;
option_file_type * load_config_file (char *input_file_name, 
		option_file_type *allowed_options,
		char **msg) ;
		
int put_option (option_file_type *allowed_options, 
	char *block_label, 
	char *option_label, 
	char *value);

option_type *copy_options (option_type *, int nbopt);

#endif // _CONFIG_RC_FILE
