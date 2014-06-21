/* x_alloc.c - Custom functions for managing the heap
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

#include <stdio.h>
#include <string.h>
#include <x_alloc.h>

static gboolean VERBOSE = FALSE;
static gboolean VERY_VERBOSE = FALSE;

static gboolean ABORT_BEHAVIOUR = FALSE;

static gboolean previous_freed = FALSE;

static gint LEFT = -1;
static gint RIGHT = 1;
static gint SEQ_THRESHOLD = 10;

static gint HEAP_INCREMENT = 1000;

static heap_type HEAP;
static gint current_pointer=0;

// Important: HEAP is an ordered list

void x_alloc_init () {
	HEAP.max = HEAP_INCREMENT;
	//  HEAP.size is in base 1
	HEAP.size = 0;
	HEAP.current_node = 0;
	HEAP.nodes = (heap_node *) calloc (HEAP.max, sizeof(heap_node));
	HEAP.no_mem_exit = NULL;
}

void x_alloc_init_with_exit (void (*no_mem_exit) (gchar *)) {
	x_alloc_init ();
	HEAP.no_mem_exit = no_mem_exit;
}

void x_alloc_release () {
	if (HEAP.nodes)
		free (HEAP.nodes);
}

void x_alloc_reinit () {
	HEAP.max += HEAP_INCREMENT;
	HEAP.nodes = (heap_node *) realloc (HEAP.nodes, HEAP.max * sizeof(heap_node));
}

static gint find_node (gpointer ptr) {
	// Remember that HEAP is an ordered array
	// Mixed strategy: 
	// Dichotomic search + sequential under some threshold
	// Returns a base 0 index
	// If index == HEAP.size, then the node is to be added
	gint pivot, step, direction=LEFT, i, j;
	gboolean found = FALSE, seq = FALSE;
	pivot = HEAP.size ;
	step = HEAP.size ;

	// Note: HEAP.size is in base 1, not 0 like C indexes

	if (!HEAP.size)
		return 0; // It's the beginning!

	// We first test if ptr is past the max or before the min
//	printf("Find-Nodes - PTR: %p; HEAP.nodes: %p; HEAP.size: %d; HEAP.max: %d\n", ptr, HEAP.nodes, HEAP.size, HEAP.max);
	if (	((HEAP.nodes + HEAP.size - 1)->ptr < ptr) ||
		(HEAP.nodes->ptr > ptr) ) 
		return (HEAP.size);

	while (!found) {
		if (step <= SEQ_THRESHOLD) {
			seq = TRUE;
//			printf("SEQ==TRUE\n");
			break;
		}

		step = (step+1) >> 1;
		if (direction == LEFT)
			pivot = MAX(0,pivot - step);
		else
			pivot = MIN(pivot + step, HEAP.size-1);

		if ((pivot>=HEAP.max) || (pivot<=0)) {
			printf ("Index error in HEAP processing: %d :: %d\n", HEAP.max, pivot);
			if (ABORT_BEHAVIOUR)
				exit (2);
		}
		if ((HEAP.nodes+pivot)->ptr > ptr) {
//			printf("Direction LEFT: (%d) Heap.nodes+pivot -> ptr (%x) > ptr (%x)\n",pivot, (HEAP.nodes+pivot)->ptr, ptr);
			direction = LEFT;
			continue;
		}
		else {
			if ((HEAP.nodes+pivot)->ptr < ptr) {
//				printf("Direction RIGHT: (%d) Heap.nodes+pivot -> ptr (%x) < ptr (%x)\n",pivot, (HEAP.nodes+pivot)->ptr, ptr);
				direction = RIGHT;
				continue;
			}
			else { // equality
//				printf("FOUND %x\n",ptr);
				found = TRUE;
				break;
			}
		}
	}

	if (found)
		return pivot;
	
	if (seq) { // Sequential search	
		if (direction == LEFT)
			i = -1;
		else
			i = 1;
		for (j=0; j<SEQ_THRESHOLD; j++) {
			pivot += i;
			if ((pivot >= HEAP.size) || (pivot < 0))
				break;
			if ((HEAP.nodes+pivot)->ptr == ptr) {
				found = TRUE;
				break;
			}
		}
	}

	if (found)
		return pivot;
	else
		return (HEAP.size);
}

/*static gint find_node_2 (gpointer ptr) {
	// Sequential search
	gint j;
	for (j=0; j<HEAP.size; j++) {
		if ((HEAP.nodes+j)->ptr == ptr)
			return j;
	}
	return HEAP.size;
	
}*/
static void order_node (gint i) {
	// Order the node at index i by swapping it
	// with its predecessors
	// We always go backward because:
	// 1. Nodes are added at the end, so i==size-1
	// 2. Nodes are never destroyed
	// 3. Nodes pointers (the address field) are never replaced
	// When reusing a pointer, only the "name" field is changed
	// Most of the time, the newly allocated pointers will
	// have an address greater than all the preceding one
	//
	// This function shouldn't be called after a realloc

	gchar *name;
	gpointer ptr;
	gint size;
	gboolean freed;
	if (i>=HEAP.size) // Nothing to do, we're outside limits (not supposed to!)
		return;

	if (i!=HEAP.size-1) {
		if (!previous_freed) {
			if (VERBOSE)
				printf("HEAP inconsistency: trying to add a node not at the end; index: %d; size: %d;\n", i, HEAP.size);
			if (ABORT_BEHAVIOUR)
				exit(5); 
			else
				return;
		}
		else {
			if (VERBOSE && VERY_VERBOSE)
				printf("Overwriting a non-reallocated freed node; index: %d; heap size: %d; %p - %d: %s\n",i, HEAP.size, (HEAP.nodes+i)->ptr, (HEAP.nodes+i)->size, (HEAP.nodes+i)->name);
			return;
		}
	}

	while (i) {
		if ( (HEAP.nodes+i)->ptr >= (HEAP.nodes+i-1)->ptr ) {
			return;
		}
		// Swap i and i-1
		ptr = (HEAP.nodes+i)->ptr;
		name = (HEAP.nodes+i)->name;
		size = (HEAP.nodes+i)->size;
		freed = (HEAP.nodes+i)->freed;
		(HEAP.nodes+i)->ptr = (HEAP.nodes+i-1)->ptr;
		(HEAP.nodes+i)->name = (HEAP.nodes+i-1)->name;
		(HEAP.nodes+i)->size = (HEAP.nodes+i-1)->size;
		(HEAP.nodes+i)->freed = (HEAP.nodes+i-1)->freed;
		(HEAP.nodes+i-1)->ptr = ptr;
		(HEAP.nodes+i-1)->name = name;
		(HEAP.nodes+i-1)->size = size;
		(HEAP.nodes+i-1)->freed = freed;
		i = i-1;
	}
}

static void add_node (gpointer ptr, gint size, gchar *name) {
	// Add a node or replace a freed node
	// i is the node index, in base 0
	gint i;
	i = find_node (ptr);
	HEAP.current_node = i;
//	printf("Adding %x == %s\n",ptr, name);
	if (i >= HEAP.max)
		x_alloc_reinit();
	if (i == HEAP.size) // new node 
		// (usually the case, except if already freed)
		HEAP.size ++;
	else {// We replace an existing node (ex. after realloc)
		if (i > HEAP.size) { // abnormal
			printf("HEAP inconsistency: node index (%d) > size (%d)\n", i, HEAP.size);
			if (ABORT_BEHAVIOUR)
				exit(3);
		}
		if ((HEAP.nodes+i)->ptr != ptr) {
			printf ("HEAP inconsistency: trying to overwrite a different pointer: (%d) %p over %p\n", i, ptr, (HEAP.nodes+i)->ptr);
			if (ABORT_BEHAVIOUR)
				exit(4);
		}		
	}
	(HEAP.nodes+i)->ptr = ptr;
	(HEAP.nodes+i)->name = name;
	previous_freed = (HEAP.nodes+i)->freed ;
	(HEAP.nodes+i)->freed = FALSE;
	(HEAP.nodes+i)->size = size;
	if (VERBOSE && VERY_VERBOSE) {
		printf ("Adding #%d: %p, %d, %s\n", i, ptr, size, name);
	}
	order_node (i);
}


static void check_ptr (gpointer ptr, gchar *name) {
	// Check memory
	if (!ptr) {
		if (!HEAP.no_mem_exit) {
			printf ("*** No more memory available when allocating %s - stopping ***\n", name);
			exit(99);
		}
		else
			(*HEAP.no_mem_exit) (name);
	}
}

gpointer x_malloc (size_t size, gchar *name) {
	// Try to allocate the pointer
	// Find if node exists using the address
	// If so, replace name, flag it as not freed
	// If not, check if extending the array is needed
	// Add the node
	// Return the added address
	gpointer ptr;
	ptr = malloc (size);
//	printf("x_malloc de %s: %p\n",name,ptr);
	check_ptr (ptr, name);
	add_node (ptr, size, name);
	return ptr;
}

gpointer x_realloc (gpointer ptr, size_t size, gchar *name) {
	// If the new pointer is not equal to the old one,
	// the old one is marked as freed,
	// the new one is added (see xmalloc for more)
	// If they're equal, only the size and the name are replaced
	gpointer new_ptr;
	gint i;
	new_ptr = realloc (ptr, size);
//	printf("realloc de %s: new_ptr == ptr in x_realloc: %p == %p\n",name, new_ptr, ptr);
	check_ptr (new_ptr, name);
	i = find_node (ptr);
	if (ptr != new_ptr) {
		// Mark old node as freed
		// Add new node
		if (i < HEAP.size) { // Exists
			(HEAP.nodes+i)->freed = TRUE;
		}
		add_node (new_ptr, size, name);
	}
	else {
		(HEAP.nodes+i)->size = size;
		(HEAP.nodes+i)->name = name;
	}
	return new_ptr;
}

gpointer x_calloc (size_t num, size_t size, gchar *name) {
	// See description in xmalloc
	gpointer ptr;
	ptr = calloc (num, size);
//	printf("x_calloc de %s: %p\n",name,ptr);
	check_ptr (ptr, name);
	add_node (ptr, num*size, name);
	return ptr;
}

void x_free (gpointer ptr) {
	// Find if pointer exists
	// If it does not exist, try to free it anyway, print a message
	// If it exists, check the "freed" flag
	// If flag is TRUE, print a message, exits
	// If flag is FALSE, free the pointer, set flag to TRUE, exits
	gint i;
	if (!ptr)
		return;

//	printf("x_free: %x\n",ptr);
	i = find_node (ptr);
	if (i < HEAP.size) { // Exists
		if ((HEAP.nodes+i)->freed) { // oops!
			if (VERBOSE)
				printf("# %d = %s: %p (%p) already freed! Skipped.\n", i, (HEAP.nodes+i)->name, (HEAP.nodes+i)->ptr, ptr);
			return;
		}
		else {
			free (ptr);
			(HEAP.nodes+i)->freed = TRUE;
			if (VERBOSE && VERY_VERBOSE) {
				printf ("Freeing ... ");
				x_alloc_print_node (i);
			}
			return;
		}
	}
	else { // Not exists ??
		if (VERBOSE)
			printf("%p not found in HEAP; trying to free it anyway\n",ptr);
		free (ptr);
		return;
	}
}

void x_alloc_print_all () {
	gint i;
	for (i=0; i<HEAP.size; i++)
		x_alloc_print_node (i);
}

void x_alloc_print_select_by_ptr (gpointer ptr) {
	gint i;
	for (i=0; i<HEAP.size; i++) {
		if ((HEAP.nodes+i)->ptr == ptr)
			x_alloc_print_node (i);
	}
}

void x_alloc_print_select_by_flag (gboolean freed_flag) {
	gint i;
	for (i=0; i<HEAP.size; i++) {
		if ((HEAP.nodes+i)->freed == freed_flag)
			x_alloc_print_node (i);
	}
}

void x_alloc_print_select_by_name (gchar *name) {
	gint i;
	for (i=0; i<HEAP.size; i++) {
		if (!strcmp((HEAP.nodes+i)->name,name))
			x_alloc_print_node (i);
	}
}

void x_alloc_print_node (gint node_index) {
	if (node_index >= HEAP.size) {
		printf ("Heap node index (%d) outside boundaries\n", node_index);
		return;
	}
	printf ("# %4d : %p ; Size: %6d; Freed: %5s; Structure: %s\n", node_index, (HEAP.nodes+node_index)->ptr, (HEAP.nodes+node_index)->size, ((HEAP.nodes+node_index)->freed?"TRUE":"FALSE"), (HEAP.nodes+node_index)->name);
}

// Functions for testing if arrays indices are out of boudaries

gboolean x_alloc_set_current_pointer (gpointer ptr) {
	current_pointer = find_node (ptr);
	if (current_pointer < HEAP.size)
		return TRUE;
	else
		return FALSE;
}

gint x_alloc_get_current_pointer () {
	return current_pointer;
}

gboolean x_alloc_test_boundaries ( gint pointer_index, gint atomic_size, gint index) {
	// Returns FALSE if index out of boudaries
	if (((atomic_size*index) > (HEAP.nodes+pointer_index)->size) || 
		(index<0) ) {
		printf ("%d: Index %d out of boundaries of %p (%s)\n", pointer_index, index, (HEAP.nodes+pointer_index)->ptr, (HEAP.nodes+pointer_index)->name);
		return FALSE;
	}
	return TRUE;
}


