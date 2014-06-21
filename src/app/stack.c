/* stack.c - undo / redo stack related functions - to be revised
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include "globals.h"
#include "doc.h"
#include "stack.h"

gint COMMIT_BEHAVIOR = COMMIT_DEFAULT;

stack_struct *stack_struct_new (gpointer (*specific_commit_or_reset) (gpointer stack_struct)) {
	stack_struct *stack = x_malloc(sizeof(stack_struct), "struct stack in stack_struct_new");
	stack->commit_fn = NULL;
	stack->reset_fn = NULL;
	stack->commit_flag = FALSE;
	stack->generic_commit_or_reset = (gpointer) generic_commit_or_reset;
	stack->specific_commit_or_reset = (gpointer) specific_commit_or_reset;
	return stack;
}

void stack_free (stack_struct *stack) {
	if (stack)
		x_free (stack);
}

void set_commit_reset_fns (stack_struct *stack,
	gpointer commit_fn,
	gpointer reset_fn,
	gpointer data) {

	// Set the functions and data to execute when we change context
	stack->commit_fn = commit_fn;
	stack->reset_fn = reset_fn;
	stack->commit_data = data;
	stack->commit_flag = FALSE;
}

void set_commit_data (stack_struct *stack, gpointer data) {
	if (stack)
		stack->commit_data = data;
}

void set_stack_specific_fn (stack_struct *stack, gpointer specific_fn) {
	if (stack)
		stack->specific_commit_or_reset = specific_fn;
}

void commit_or_reset (stack_struct *stack) {
	if (stack->generic_commit_or_reset) {
//		printf("Executing generic_commit_or_reset: %p on %p\n",stack->generic_commit_or_reset, stack);
		(*stack->generic_commit_or_reset) (stack);
	}
	if (stack->specific_commit_or_reset) {
//		printf("Executing specific_commit_or_reset: %p on %p\n",stack->generic_commit_or_reset, stack);
		(*stack->specific_commit_or_reset) (stack);
	}
}

void generic_commit_or_reset (stack_struct *ss) {

//	Generic function for committing or resetting the changes in the current dialog
//	Should be executed once - managed with commit_flag

	if (!ss)
		return;

	if (!ss->commit_data)
		return;
		
	if (ss->commit_flag)
		return;

	switch (COMMIT_BEHAVIOR) {
		case COMMIT_DEFAULT:
			if (ss->commit_fn) {
				(*ss->commit_fn) (ss->commit_data);
//				printf("*COMMIT_FN: %p\n",ss->commit_fn);
				ss->commit_flag = TRUE;
			}
			break;
		case RESET_DEFAULT:
			if (ss->reset_fn) {
				(*ss->reset_fn) (ss->commit_data);
//				printf("*RESET_FN: %d\n",ss->reset_fn);
				ss->commit_flag = TRUE;
			}
			break;
		case COMMIT_RESET_ASK:
			if ( yes_no ( _("COMMIT_CHANGES"), TRUE ) ) {
				if (ss->commit_fn) {
					(*ss->commit_fn) (ss->commit_data);
//					printf("*COMMIT_FN: %d\n",ss->commit_fn);
					ss->commit_flag = TRUE;
				}
			}
			else {
				if (ss->reset_fn) {
					(*ss->reset_fn) (ss->commit_data);
//					printf("*RESET_FN: %d\n",ss->reset_fn);
					ss->commit_flag = TRUE;
				}
			}

	}
	
}

void doc_undo(gpointer dsw_ptr) {
	GList *node;
	doc_wrapper *dw;
	doc_swap_struct *dsw = (doc_swap_struct *) dsw_ptr;
// printf("\nDOC_UNDO\n\n");
	dw = dsw->current_doc;
	if (dw->type->stack->commit_flag)
		// We assume that the current transaction would be pending again
		// (we take no chance... to be revisited in the future)
		dw->type->stack->commit_flag = FALSE;
//	Try the custom undo function, if it doesn't exist apply the default one	
	if (dw->type->undo) {
		(*dw->type->undo) (dw->data);
		return;
	}
//	If no history, return (should be tested in the calling function, but we take no risk)
	if (!g_list_length(dw->history))
		return;
//	If no copy function, we cannot swap the current data and the data to show
	if (!dw->type->copy_struct)
		return;
//	Standard processing:
	//	In all cases, we register the current data in history if required
	//	Find the current document data_to_show in the history
	//		if not found or NULL:  it's the first undo in the chain
	//	Find the document data previous to the current document
	//		if current document not found:  take the last document data in history
	//		if no data before the current document, nothing to undo, return
	//	If it's the first undo in the chain,
	//		add the current document data at the end of history
	//			drop the beginning if stack too long (done by the add function)
	//	Replace the current document data with the found data
	//	Display the current data
	if (dw->to_register_in_history)
		doc_register_in_history((gpointer) dw);
	node = NULL;
	if (dw->data_to_show) {
		node = g_list_first(dw->history);
		while (node) {
			if (dw->data_to_show == node->data)
				break;
			node = node->next;
		}
	}
	if (!node) {
// printf("NODE NOT FOUND\n");
		// Not found (not supposed, since all undoable actions should be recorded)
		// We take the last document data
		// If an action was not recorded, it is not redoable
		dw->data_to_show = g_list_last(dw->history)->data; 
	}
	else {
// printf("NODE FOUND!\n");
		if (!node->prev) {
// printf("DÉBUT DE L'HISTORIQUE\n");
			return;				// no data to undo!
		}
		dw->data_to_show = node->prev->data;
	}
// printf("UNDOING: DW->data: %d;  data_to_show: %d\n", dw->data, dw->data_to_show);
	if (dw->type->stack)
		commit_or_reset (dw->type->stack);
	//	Replace the current document data with the found data
	(*dw->type->copy_struct) (dw->data_to_show, dw->data, UNDO);
//	NOTE: we undo even if there is no display function... this FN is almost mandatory!
	if (dw->type->display)
		(*dw->type->display) (dw->data);
}

void doc_redo(gpointer dsw_ptr) {
	GList *node;
	doc_wrapper *dw;
	doc_swap_struct *dsw = (doc_swap_struct *) dsw_ptr;
	dw = dsw->current_doc;
	if (dw->type->stack)
		// We assume that the current transaction would be pending again
		// (we take no chance... to be revisited in the future)
		dw->type->stack->commit_flag = FALSE;
//	Try custom redo function, if it doesn't exist apply the default one
	if (dw->type->redo) {
		(*dw->type->redo) (dw);
		return;
	}
//	If no history, return (should be tested in the calling function, but we take no risk)
	if (!g_list_length(dw->history))
		return;
//	If no copy function, we cannot swap the current data and the data to show
	if (!dw->type->copy_struct)
		return;
//	Standard processing:
	//	Find the current document data in history
	//		if not found:  nothing to redo, return!
	//	Go to the next data
	//		if no next data, data was updated after the last undo, return
	//	Let the next data become the current data
	//	Display the current data	
	node = NULL;
	if (dw->data_to_show) {
		node = g_list_first(dw->history);
		while (node) {
			if (dw->data_to_show == node->data)
				break;
			node = node->next;
		}
	}
	if (!node) {
// printf("NODE NOT FOUND\n");
		return;
	}
	else {
// printf("NODE FOUND!\n");
		if (!node->next) {
// printf("HISTORY START\n");
			return;				// no data to redo!
		}
		dw->data_to_show = node->next->data;
	}
// printf("REDOING: DW->data: %d;  data_to_show: %d\n", dw->data, dw->data_to_show);
	if (dw->type->stack)
		commit_or_reset (dw->type->stack);
	(*dw->type->copy_struct) (dw->data_to_show, dw->data, REDO);
//	NOTE: we redo even if there is no display function... this FN is almost mandatory!
	if (dw->type->display)
		(*dw->type->display) (dw->data);
}

gboolean doc_register_in_history (gpointer dw_ptr) {
//	Put a copy of the current doc data in the history list
//	If the copy function for the current document type is undefined, do nothing
//	If the document pointer is already in the history, we are modifying an undoed instance
//	... so we must free all the instances next to the current one (this is the Gimp behavior)
	GList *node, *current, *node_to_remove;
	doc_wrapper *dw = (doc_wrapper *) dw_ptr;
	if (!dw->type->copy_struct)
		return FALSE;
//	printf("DOC_REGISTER_IN_HISTORY:  DW: %d == %x; DATA (HFW): %d = %x \n",dw, dw, dw->data, dw->data);
	current = g_list_find(dw->history,dw->data_to_show);
	if (current) {
//		Free all the document instances next to the current instance
		node = current->next;
		while (node) {
//		printf("DOC_REGISTER_IN_HISTORY:  DW_FREE- NODE->DATA: %d == %x\n",node->data, node->data);
			(*dw->type->free) (node->data, FALSE);
			node_to_remove = node;
			node = node->next;
			dw->history = g_list_remove(dw->history,node_to_remove->data);
		}
	}
	if (dw->to_register_in_history)
		dw->history = g_list_append(dw->history,
			(*dw->type->copy_struct) (dw->data, NULL, FALSE));
// printf("DOC_REGISTER_IN_HISTORY longueur de dw->history: %d\n",g_list_length(dw->history));
//	Test for length
	if (g_list_length(dw->history)>dw->max_history) {
		node = g_list_first(dw->history);
		if (dw->type->free)
			(*dw->type->free) (node->data, FALSE);
		dw->history = g_list_remove(dw->history,node->data);
	}
// printf("Longueur de dw->history à la sortie de doc_register_in_history: %d\n",g_list_length(dw->history));
// print_list(dw->history);
//	The "data to show" is always the last instance recorded
	dw->data_to_show = g_list_last(dw->history)->data;
//	printf("DOC_REGISTER_IN_HISTORY, data to show (HFW):  %d == %x; \n",dw->data_to_show, dw->data_to_show);
	dw->to_register_in_history = FALSE;
	return TRUE;
}


