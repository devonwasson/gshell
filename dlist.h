/*
 * Copyright (c) 2012 Bucknell University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Devon Wasson
 */

#ifndef _DLIST_H_
#define _DLIST_H_

#include <stdint.h>
#include <stdbool.h>
#include "dnode.h"

/**
 * Doubly-linked list.
 */
struct dlist {
	struct dnode *front;
	struct dnode *back;
	struct dnode *iter;
	uint32_t counter;
};

/**
 * Allocates new dlist dynamically.
 *
 * @return pointer to the list.
 */
struct dlist *dlist_create();

/**
 * Inserts new node in dlist before the first node.
 *
 * @param l pointer to the list (non_NULL)
 * @param ptr pointer to generic data to store in new list node
 */
void dlist_add_front(struct dlist *l, char* str);

/**
 * Initialize iterator for traversing a dlist forwards.
 */
void dlist_iter_begin(struct dlist *l);

/**
 * Returns current item and advances iterator to next item in list.
 */
struct dnode *dlist_iter_next(struct dlist *l);

/**
 * Check if iterator has reached the end of the list.
 */
bool dlist_iter_has_next(struct dlist *l);

/**
 * Remove the first node and return pointer to the data. It is the
 * responsibility of the caller to free the memory pointed by the return
 * value.
 * 
 * @param l pointer to the list (non-NULL)
 * @return pointer to data stored in front node
 */
void dlist_remove_front(struct dlist *l);

#endif

