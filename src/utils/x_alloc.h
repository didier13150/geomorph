/* x_alloc.h - Headers for custom functions for managing the heap
 *
 * Copyright (C) 2008 Patrice St-Gelais
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
#include <glib.h>

#ifndef _XALLOC
#define _XALLOC 1

typedef struct {
	gpointer ptr;
	gint size;
	gchar *name;
	gboolean freed;
} heap_node;

// Heap type -> nodes is an ordered array
typedef struct {
	// max and size are in base 1, they are not C indexes 
	// (10 elements, from 0 to 9, ==> size = 10)
	gint max;
	gint size;
	gint current_node;
	heap_node *nodes;
	void (*no_mem_exit) (gchar *struct_name);
} heap_type;

void x_alloc_init ();
void x_alloc_init_with_exit (void (*no_mem_exit) (gchar *struct_name));
gpointer x_malloc (size_t size, gchar *name);
gpointer x_realloc (gpointer ptr, size_t size, gchar *name);
gpointer x_calloc (size_t num, size_t size, gchar *name);
void x_free (gpointer ptr);
void x_alloc_release ();
void x_alloc_print_all ();
void x_alloc_print_select_by_ptr (gpointer ptr);
void x_alloc_print_select_by_flag (gboolean freed_flag);
void x_alloc_print_select_by_name (gchar *name);
void x_alloc_print_node (gint node_index);
gboolean x_alloc_test_boundaries (gint pointer_index, gint atomic_size, gint index);
gboolean x_alloc_set_current_pointer (gpointer ptr);
gint x_alloc_get_current_pointer () ;

#endif // _XALLOC



