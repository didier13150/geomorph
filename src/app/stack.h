/* stack.h - headers for managing the undo / redo stack
 *
 * Copyright (C) 2010 Patrice St-Gelais
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _STACK
#define _STACK 1

#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#include "../utils/menus_n_tools.h"
#include "globals.h"

//	Changes confirmation behavior
#ifndef COMMIT_DEFAULT
#define COMMIT_DEFAULT 0
#endif

#ifndef RESET_DEFAULT
#define RESET_DEFAULT 1
#endif

#ifndef COMMIT_RESET_ASK
#define COMMIT_RESET_ASK 2
#endif

#ifndef UNDO
#define UNDO 10001
#endif

#ifndef REDO
#define REDO 10002
#endif

#ifndef NONE
#define NONE 10003
#endif

// There is one document stack for each document type, 
typedef struct {
	gpointer (*commit_fn) (gpointer commit_data); // Function to execute when swapping context (or the next)
	gpointer (*reset_fn) (gpointer commit_data);
	gpointer commit_data;
	gboolean commit_flag; // FALSE when a transaction is pending
	void (*generic_commit_or_reset) (gpointer stack_struct);
	void (*specific_commit_or_reset) (gpointer stack_struct);
} stack_struct;

/**********************************************************************/

stack_struct *stack_struct_new (gpointer (*specific_commit_or_reset) (gpointer stack_struct));

void stack_free (stack_struct *);

void generic_commit_or_reset (stack_struct *) ;

void commit_or_reset (stack_struct *) ;

void set_commit_reset_fns (stack_struct *stack,
	gpointer commit_fn,
	gpointer reset_fn,
	gpointer data);

void set_stack_specific_fn (stack_struct *stack, gpointer specific_fn);

void set_commit_data (stack_struct *stack, gpointer data);

void doc_undo (gpointer doc_swap_struct_ptr);

void doc_redo (gpointer doc_swap_struct_ptr);

gboolean doc_register_in_history (gpointer dw_ptr) ;

#endif // _STACK



