/* config_rc_file.c - utilities which process an options file
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

// New version of config_file.c, 2002.08
//	- added features for validating and displaying fields in a GTK+ interface
//	   (see config_rc_file.h and config_dialog.c)
//	- removed the "read" and "write" functions (too cumbersome)

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "config_rc_file.h"
#include "utils.h"
#include "x_alloc.h"

char *alltrim(char *some_string) {
	// Trim "some_string" and return a new string (newly allocated)
	// Remove comments at the end (begining by ";" or "//")
	// Future improvement:  don't remove comments inside simple / double quotes...
	char *return_string;
	int i, max, start=0, stop=0;
	int start_not_found = TRUE;
	max = strlen(some_string);
	
	for (i=max; i>=0; i--) {
		if ((' '!=some_string[i]) && ('\0'!=some_string[i])
			&& ('\n'!=some_string[i])) {
				stop = i;
				break;
		}
	}

	if ((' '!=some_string[stop]) && ('\0'!=some_string[stop])
			&& ('\n'!=some_string[stop])) {
		max = stop + 1;
	}
	else {
		max = stop;
	}

	for (i=0; i<=stop; i++) {
		if (start_not_found)
			if (' '!=some_string[i]) {
				start = i;
				start_not_found = FALSE;
			}
//	Testing for comments
		if (';'==some_string[i])
			max = i;		
		else
			if ((i+1)<max)
				if (('/'==some_string[i]) && ('/'==some_string[i+1]))
					max = i;
	}
	if (max<=start) {
		return_string = (char *)(intptr_t) x_malloc(1, "char (return_string in alltrim in config_rc_file.c)");
		return_string[0] = '\0';
		return return_string;
	}
	return_string = (char *)(intptr_t) x_malloc(1+max-start, "char (return_string in alltrim in config_rc_file.c)");
	strncpy(return_string,some_string+start,max-start);
	return_string[max-start]='\0';
	return return_string;
}

option_type *copy_options (option_type *opt, int nbopt) {

	// Does what the name says
	int l, i;
	option_type *new;
	new = (option_type *) x_malloc( nbopt * sizeof(option_type), "option_type (new in copy_options)" );
	
	for (i=0; i<nbopt; i++) {
		if (opt[i].label) {
			l = strlen(opt[i].label)+1;
			new[i].label = (char *) x_malloc(l, "char (label in copy_options)");
			memcpy(new[i].label, opt[i].label, l);
		}
		else
			new[i].label = NULL;
		
		if (opt[i].value) {
			l = strlen(opt[i].value)+1;
			new[i].value = (char *) x_malloc(l, "char (value in copy_options)");
			memcpy(new[i].value, opt[i].value, l);
		}
		else
			new[i].value = NULL;
			
		new[i].widget = opt[i].widget;
				
		if (opt[i].field_type) {
			l = strlen(opt[i].field_type)+1;
			new[i].field_type = (char *) x_malloc(l, "char (field_type in copy_options)");
			memcpy(new[i].field_type, opt[i].field_type, l);
		}
		else
			new[i].field_type = NULL;
			
		if (opt[i].field_domain) {
			l = strlen(opt[i].field_domain)+1;
			new[i].field_domain = (char *) x_malloc(l, "char (field_domain in copy_options)");
			memcpy(new[i].field_domain, opt[i].field_domain, l);
		}
		else
			new[i].field_domain = NULL;
		
		if (opt[i].field_description) {	
			l = strlen(opt[i].field_description)+1;
			new[i].field_description = (char *) x_malloc(l, "char (field_description in copy_options)");
			memcpy(new[i].field_description, opt[i].field_description, l);
		}
		else
			new[i].field_description = NULL;
		
	}
	return new;
}

int create_block(char *lbracket, char *rbracket, 
		option_file_type *current_options,
		option_file_type *allowed_options) {

	// 	Creates and allocates a new bloc
	//	Returns the block index in allowed_options
	int i;
	if (current_options->nb_blocks)
		current_options->option_block = (option_block_type *)
			x_realloc(current_options->option_block,
			  (current_options->nb_blocks+1)*sizeof(option_block_type), "option_block_type (realloc current_options->option_block in create_block)");
	else
		current_options->option_block = (option_block_type *)
			x_malloc(sizeof(option_block_type), "option_block_type (malloc current_options->option_block in create_block)");
	strcpy(rbracket,"\0");
	current_options->option_block[current_options->nb_blocks].nb_options = 0;
	current_options->option_block[current_options->nb_blocks].block_name =
		(char *) x_malloc (strlen(lbracket+1)+1, "char (block_name)");
	strcpy(current_options->option_block[current_options->nb_blocks].block_name,lbracket+1);
	// Find current block in allowed_options
	for (i=0; i<allowed_options->nb_blocks;i++)
		if (!strcmp(allowed_options->
		    option_block[i].block_name,
		    current_options->option_block[current_options->nb_blocks].block_name))
			break;
	current_options->nb_blocks++;
	if (i>=allowed_options->nb_blocks) {
		
		printf(_("Options block <<%s>> unknown; <<%s>> used\n"),lbracket+1,
			allowed_options->option_block[0].block_name);
		return 0;
	}
	else
		return i;
}

void create_option(char *buf_ptr, option_block_type *cur_bl,
	option_block_type *allowed_options) {

	// Add an option value
	// Modified 2007-03 for UTF8 conversion related to files and directories

	int i;
	char *equal_sign, buf[255], *buf_utf8;
	equal_sign = strchr(buf_ptr,'=');
	if (equal_sign) {
		if (cur_bl->nb_options)
			cur_bl->option_list = (option_type *) x_realloc(cur_bl->option_list,
				(cur_bl->nb_options+1)*sizeof(option_type), "option_type (realloc cur_bl->option_list in create_option)");
		else
			cur_bl->option_list = (option_type *) x_malloc(sizeof(option_type), "option_type (malloc cur_bl->option_list in create_option)");
		strncpy(buf,buf_ptr,equal_sign-buf_ptr);
		buf[equal_sign-buf_ptr]='\0';
		cur_bl->option_list[cur_bl->nb_options].label = alltrim(buf);
		cur_bl->option_list[cur_bl->nb_options].value = alltrim(equal_sign+1);
/*		
 printf("BLOCK NAME: %s; LABEL READ: %s;  VALUE: %s; L: %d\n",
 		cur_bl->block_name, 
		cur_bl->option_list[cur_bl->nb_options].label,
		cur_bl->option_list[cur_bl->nb_options].value,
		strlen(	cur_bl->option_list[cur_bl->nb_options].value));
*/		
		for (i=0; i<allowed_options->nb_options; i++)
			if (!strcmp(cur_bl->option_list[cur_bl->nb_options].label,
				allowed_options->option_list[i].label))
				break;
		if (i>=allowed_options->nb_options) {
			printf(_("Options file: <<%s>> unknown\n"),cur_bl->option_list[cur_bl->nb_options].label);
			allowed_options->option_list[i].value = NULL;
		}
		else	// Redundancy, for managing display updates
			// (the widget structure is defined in allowed_options)
			{		
			// Here we know if the value is a file or a dir name,
			// which needs a conversion to UTF8
			if (cur_bl->option_list[cur_bl->nb_options].value && strstr("#dir#file",allowed_options->option_list[i].field_type)) {
				buf_utf8 = g_filename_to_utf8(cur_bl->option_list[cur_bl->nb_options].value, -1, NULL, NULL,NULL) ;
				allowed_options->option_list[i].value = (gchar *) x_malloc(strlen(buf_utf8)+1, "gchar (value in create_option)");
				strcpy(allowed_options->option_list[i].value,buf_utf8);
				}
			else
				allowed_options->option_list[i].value = 
					cur_bl->option_list[cur_bl->nb_options].value;
			}
/*				
 printf("BLOCK NAME in ALLOWED_OPTIONS: %s; LABEL READ: %s;  VALUE: %s; L: %d\n",
 		allowed_options->block_name, 
		allowed_options->option_list[i].label,
		allowed_options->option_list[i].value,
		strlen(	allowed_options->option_list[i].value));
*/		
		cur_bl->nb_options++;
	}
	else {
		printf(_("Options file: <<%s>> unknown\n"),buf_ptr);
	}  // equal sign else
}

void print_options (	FILE *fp, 
			option_file_type *current_options) {
	// Prints an option file
	int i,j;
	char *opt;
//	printf("PRINT_OPTIONS\n");
	for (i=0; i<current_options->nb_blocks; i++) {
		fprintf(fp,"\n[%s]\n",
			current_options->option_block[i].block_name);
		for (j=0; j<current_options->option_block[i].nb_options; j++) {
			if (current_options->option_block[i].option_list[j].value) {	
				if (strstr("#dir#file",current_options->option_block[i].option_list[j].field_type))
					opt = g_filename_from_utf8(current_options->option_block[i].option_list[j].value,-1,NULL,NULL,NULL);
				else
					opt = current_options->option_block[i].option_list[j].value;
					
				fprintf(fp,"%s = %s\n",
				  current_options->option_block[i].option_list[j].label, opt);
			}
			else
				fprintf(fp,"%s = \n",
				  current_options->option_block[i].option_list[j].label);
		}
	}
}

void print_allowed_options (	FILE *fp, 
				option_file_type *allowed_options,
				int if_comments) {
	// Prints an option file with all allowed options
	// Print description as comments if if_comments == TRUE
	// 2005-11-09: convert field description from UTF8 to current locale
	// --> with GTK 2, results of gettext (_()) are UTF8 strings
	// 2007-03: we also translate the directory and file names to current locale
	int i,j;
	char *opt,*buf=NULL, *comments=NULL, *opt_ok;
	for (i=0; i<allowed_options->nb_blocks; i++) {
		buf = (char *) x_realloc(buf,strlen(allowed_options->option_block[i].block_name)+3, "char (buf in print_allowed_options)");
		sprintf(buf, "[%s]", allowed_options->option_block[i].block_name);
		if (if_comments) {
			fprintf(fp,"\n%-33s ; %s\n", buf, g_locale_from_utf8( _(allowed_options->option_block[i].block_description), -1, NULL, NULL, NULL));
			}
		else
			fprintf(fp,"\n%s\n",buf);
		for (j=0; j<allowed_options->option_block[i].nb_options; j++) {
			opt = allowed_options->option_block[i].option_list[j].value;
			if (opt && strstr("#dir#file",allowed_options->option_block[i].option_list[j].field_type))
				opt_ok = g_filename_from_utf8(opt,-1,NULL,NULL,NULL);
			else
				opt_ok = opt;
			if (if_comments) {
				comments = (char *) x_realloc(comments, 3+strlen(
				  _(allowed_options->option_block[i].option_list[j].field_description)), "char (comments in print_allowed_options)");
				sprintf(comments, "; %s",
				  _(allowed_options->option_block[i].option_list[j].field_description));
				fprintf(fp,"%-20s = %-10s %s\n",
				  allowed_options->option_block[i].option_list[j].label,
				  opt_ok, g_locale_from_utf8(comments, -1, NULL, NULL, NULL));
			}
			else {
				if (opt_ok && strlen(opt_ok)) {	// Don't print if the string is empty
					comments = x_realloc(comments, 2, "char (comments in print_allowed_options)");
					strcpy(comments," ");
					fprintf(fp,"%-20s = %-10s %s\n",
					  allowed_options->option_block[i].option_list[j].label,
					  opt_ok, comments);
				}
			}
		}
	}
	if (buf) x_free(buf);
	if (comments) x_free(comments);
}

int get_block_index (char *block_label, option_file_type *opt) {
	//	Get block index of "block_label" in "opt" - 0 being the 1st item
	//	If not found, return BLOCK_NOT_FOUND, which is supposed
	//	to be a negative number or a very high positive one
	int i;
	for (i=0; i<opt->nb_blocks; i++) {
		if (!strcmp(opt->option_block[i].block_name, block_label)) 
			break;
	}
	if (i==opt->nb_blocks)
		return BLOCK_NOT_FOUND;
	else
		return i;
}

int put_option (option_file_type *options, char *block_label, char *option_label, char *val) {
	// Writes an option value in "options"
	// The option indexed by block_label and option_label must exists
	// Returns TRUE if done, FALSE if not
	// Modified 2007-03 for UTF8 conversion related to files and directories
	int i,j;
	const gchar *buf_utf8;
	gchar *value = 0;
	if (!options)
		return FALSE;
	for (i=0; i<options->nb_blocks; i++) {
		if (block_label) {
			if (!strcmp(options->option_block[i].block_name,
				block_label)) {
				for (j=0; j<options->option_block[i].nb_options; j++) {
//				   printf("OPTION_LABEL: %s\n",current_options->option_block[i].option_list[j].label);
	  			   if (!strcmp(options->option_block[i].option_list[j].label,
					option_label)) {
						// Tilt!
						if (options->option_block[i].option_list[j].value)
							x_free (options->option_block[i].option_list[j].value);
						if (val && strstr("#dir#file#",options->option_block[i].option_list[j].field_type)) {
							buf_utf8 = g_filename_from_utf8(val, -1, NULL, NULL, NULL);
							value = (gchar *) x_malloc(strlen(buf_utf8)+1, "gchar (value in put_option)");
							strcpy(value, buf_utf8);
						}
						else
							value = val;
						options->option_block[i].option_list[j].value = value;
						return TRUE;
					}
				}  // option loop
			} // end if strcmp
		} // end if block_label
		else {  // Block label is NULL
			for (j=0; j<options->option_block[i].nb_options; j++) {
//				printf("OPTION_LABEL: %s; ",current_options->option_block[i].option_list[j].label);
	  			if (!strcmp(options->option_block[i].option_list[j].label,
					option_label)) 
					// Tilt!
					if (options->option_block[i].option_list[j].value)
						x_free (options->option_block[i].option_list[j].value);
					options->option_block[i].option_list[j].value = value;
					return TRUE;
			}
		} // end else
	}
	return FALSE;
}

char * get_option (option_file_type *current_options, char *block_label, char *option_label) {
	// Gets one specific option
	// Sequential search
	// If the block_label is not given, search all blocks
	// Returns the pointer of the found string - doesn't allocate a new one
	// Returns NULL if option not found
	// Modified 2007-03 for UTF8 conversion related to files and directories
	int i,j;
	char *answer = NULL;
//	printf("BLOCK / OPTION to find: %s.%s\n",block_label, option_label);
	if (!current_options)
		return NULL;
	for (i=0; i<current_options->nb_blocks; i++) {
//		printf("BLOCK_NAME: %s\n",current_options->option_block[i].block_name);
		if (block_label) {
			if (!strcmp(current_options->option_block[i].block_name,
				block_label)) {
				for (j=0; j<current_options->option_block[i].nb_options; j++) {
//				   printf("OPTION_LABEL: %s\n",current_options->option_block[i].option_list[j].label);
	  			   if (!strcmp(current_options->option_block[i].option_list[j].label,
					option_label)) // Tilt!
						break;
				}  // option loop
				break; // exit from block loop
			} // end if strcmp
		} // end if block_label
		else {  // Block label is NULL
			for (j=0; j<current_options->option_block[i].nb_options; j++) {
//				printf("Block label is NULL - OPTION_LABEL: %s; ",current_options->option_block[i].option_list[j].label);
	  			if (!strcmp(current_options->option_block[i].option_list[j].label,
					option_label)) // Tilt!
						break;
			}
			if (j<current_options->option_block[i].nb_options)
				break; // from block loop
		} // end else
	} // end block loop
	if (i < current_options->nb_blocks)
		if (j < current_options->option_block[i].nb_options)
			answer = current_options->option_block[i].option_list[j].value;
//	printf("VALUE: %s\n",answer);
	return answer;
}

void free_options (option_file_type *current_options) {

	int i,j;
	void *ptr;
	for (i=0; i<current_options->nb_blocks; i++) {
		for (j=0; j<current_options->option_block[i].nb_options; j++) {
			ptr = current_options->option_block[i].option_list[j].label;
			if (ptr) x_free(ptr);
			ptr = current_options->option_block[i].option_list[j].value;
			if (ptr) x_free(ptr);
		}
		ptr = current_options->option_block[i].block_name;
		if (ptr) x_free(ptr);
		ptr = current_options->option_block[i].option_list;
		if (ptr) x_free(ptr);
	}
	x_free(current_options->option_block);
}

void free_options_except_values (option_file_type *current_options) {
//	In the GUI subsystem, values are copied in a "dictionary" ("allowed_options")
//	and freed in the widget callbacks at each load of a new file and 
//	each widget interaction (for instance, a new entry is typed), so
//	we must avoid freeing these values more than once
	int i,j;
	void *ptr;
	for (i=0; i<current_options->nb_blocks; i++) {
		for (j=0; j<current_options->option_block[i].nb_options; j++) {
			ptr = current_options->option_block[i].option_list[j].label;
			if (ptr) x_free(ptr);
		}
		ptr = current_options->option_block[i].block_name;
		if (ptr) x_free(ptr);
		ptr = current_options->option_block[i].option_list;
		if (ptr) x_free(ptr);
	}
	x_free(current_options->option_block);
}
void load_options (FILE *fp, option_file_type *current_options,
			option_file_type *allowed_options) {

//	Load options from a file without executing them
	int current_allowed_block = 0;
	char buf[255];
	char *buf_ptr;
	char *rbracket,*lbracket;
	char default_block[]=" Default ";
//	if (current_options)	// not supposed to be required 
//		free_options(current_options);
	current_options->nb_blocks = 0;
	while (fgets(buf,255,fp)) {
		buf_ptr = alltrim(buf);
		if (strlen(buf_ptr)) {
			rbracket = strchr(buf_ptr,']');
			lbracket = strchr(buf_ptr,'[');
			if ((lbracket < rbracket) && rbracket && lbracket) {
				current_allowed_block = 
					create_block(lbracket, rbracket, current_options, allowed_options);
			}
			else {
				if (!current_options->nb_blocks) 
					// Create a default block:
					current_allowed_block = create_block(default_block,
						default_block+strlen(default_block)-1,
						current_options,allowed_options);
						
				create_option(buf_ptr, 
					&current_options->option_block[current_options->nb_blocks-1],
					&allowed_options->option_block[current_allowed_block]);
				
			}
		} // strlen(buf_ptr) test
		x_free(buf_ptr);
	} // while fgets
}

option_file_type * load_config_file (char *input_file_name, 
		option_file_type *allowed_options,
		char **msg) {
	FILE *fin;
	char *buf;
	option_file_type *input_options = NULL;
	if (!(fin=fopen(input_file_name,"rb"))) {
		buf = (char *) x_malloc(strlen(_("Not able to open the options file %s for reading."))+strlen(input_file_name)+1, "const char (buf - msg in load_config_file)");
		sprintf(buf,_("Not able to open the options file %s for reading."),input_file_name);
		if (msg)
			(*msg) = buf;
	}
	else {
		input_options = (option_file_type *) x_calloc(sizeof(option_file_type),1, "option_file_type");
		load_options(fin, input_options, allowed_options);
		fclose(fin);
	}
//	print_options (stdout, allowed_options);
	return input_options;
}

void save_config_file(char *output_file_name, option_file_type *opt,
		char **msg) {
	FILE *fin;
	char *buf;
	if (!(fin=fopen(output_file_name,"wb"))) {
		buf = (char *) x_malloc(strlen(_("Not able to open the options file %s for writing."))+strlen(output_file_name)+1,"const char (buf - msg in save_config_file)" );
		sprintf(buf,_("Not able to open the options file %s for writing."),output_file_name);
		if (msg)
			(*msg) = buf;
	}
	else {
		print_options(fin,opt);
		fclose(fin);
	}
}

void save_all_options(char *output_file_name, 
		option_file_type *allowed_opt,
		int if_comments,
		char **msg) {
	FILE *fin;
	char *buf;
	if (!(fin=fopen(output_file_name,"wb"))) {
		buf = (char *) x_malloc(strlen(_("Not able to open the options file %s for writing."))+strlen(output_file_name)+1, "const char (buf - msg in save_all_options)");
		sprintf(buf,_("Not able to open the options file %s for writing."),output_file_name);
		if (msg)
			(*msg) = buf;
	}
	else {
		print_allowed_options(fin,allowed_opt,if_comments);
//		print_allowed_options(stdout,allowed_opt,if_comments);
		fclose(fin);
	}
}

