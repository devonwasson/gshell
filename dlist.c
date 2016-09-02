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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dnode.h"
#include "dlist.h"


struct dlist *
dlist_create() {
	return calloc(1, sizeof(struct dlist));
}

void
dlist_add_front(struct dlist *l, char *str) {
	if (str == NULL){return;}
	if ((strcmp(str, "last") == 0) || (strcmp(str, "executeLast") == 0)) {
		return;
	}
	struct dnode *n = dnode_create();
	dnode_setdata(n, str);

	if (0 == l->counter) {
		l->front = l->back = n;
		l->counter = 1;
	} else {
		n->next = l->front;
		l->front->prev = n;
		l->front = n;
		(l->counter)++;
	}

dlist_iter_begin(l);

#ifdef DEBUG
	printf("counter= %d, %s\n", l->counter, (char *) ptr);
	printf("front= %s\n", (char *) l->front->data);
	printf("back= %s\n\n", (char *) l->back->data);
#endif /* DEBUG */
}

void
dlist_iter_begin(struct dlist *l) {
	l->iter = l->front;
}

struct dnode *
dlist_iter_next(struct dlist *l) {
	struct dnode *ret_val = NULL;

	if (l->iter != NULL) {
		ret_val = l->iter;
		l->iter = l->iter->next;
	}

	return ret_val;
}

bool
dlist_iter_has_next(struct dlist *l) {
	bool ret_val = false;

	if (l->iter != NULL) {
		ret_val = (l->iter->next != NULL);
	}

#ifdef DEBUG
	if (ret_val) {
		printf("dlist_has_next: current is %s\n", (char *) l->iter->data);
		printf("dlist_has_next: returning %d\n\n", ret_val);
	}
#endif /* DEBUG */

	return ret_val;
}


void
dlist_remove_front(struct dlist *l) {

	if(1 == l->counter){
                l->front = l->back = NULL;
        }
	else{
             	l->front = l->front->next;
                l->front->prev = NULL;
        }

	(l->counter)--;
        return;

}
