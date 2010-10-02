/* undo.c - Zedit undo commands
 * Copyright (C) 1988-2010 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "z.h"

#if UNDO

#define ACT_INSERT 1
#define ACT_DELETE 2

struct undo {
	int action;
	struct mark *end;
	Byte *data;
	int size;
	struct undo *prev, *next;
};

static struct undo *freelist;

static struct undo *new_undo(struct buff *buff)
{
	struct undo *undo;

	if (freelist) {
		undo = freelist;
		freelist = freelist->prev;
	} else {
		undo = malloc(sizeof(struct undo));

		if (!undo)
			return NULL;
	}

	/* reset everything */
	memset(undo, 0, sizeof(struct undo));

	undo->end = bcremrk();
	undo->prev = buff->undo_tail;
	buff->undo_tail = undo;

	return undo;
}

static void recycle_undo(struct buff *buff)
{
	struct undo *undo = buff->undo_tail;

	buff->undo_tail = undo->next;

	if (undo->data)
		free(undo->data);

	undo->prev = freelist;
	freelist = undo;
}

static inline int no_undo(struct buff *buff)
{
	if (buff == Paw || buff == Killbuff)
		return TRUE;

	return FALSE;
}

/* Exports */

void undo_add(int size)
{
	struct undo *undo = Curbuff->undo_tail;

	if (no_undo(Curbuff))
		return;

	if (undo && undo->action == ACT_INSERT && Bisatmrk(undo->end)) {
		undo->size += size;
		bmrktopnt(undo->end);
		return;
	}

	/* need a new undo */
	undo = new_undo(Curbuff);
	if (undo == NULL)
		return;

	undo->action = ACT_INSERT;
	undo->size = size;
}

/* Size is always within the current page. */
void undo_del(int size)
{
	struct undo *undo = Curbuff->undo_tail;

	if (no_undo(Curbuff))
		return;

	if (size == 0) /* this can happen on page boundaries */
		return;

	/* SAM merge deletes! */

	/* need a new undo */
	undo = new_undo(Curbuff);
	if (undo == NULL)
		return;

	undo->data = malloc(size);
	if (!undo->data) {
		recycle_undo(Curbuff);
		return;
	}

	memcpy(undo->data, Curcptr, size);

	undo->action = ACT_DELETE;
	undo->size = size;
}

void undo_clear(struct buff *buff)
{
	while (buff->undo_tail)
		recycle_undo(buff);
}

void Zundo(void)
{
	struct undo *undo = Curbuff->undo_tail;
	int i;

	if (!undo) {
		Tbell();
		return;
	}

	switch (undo->action) {
	case ACT_INSERT:
		bpnttomrk(undo->end);
		bmove(-undo->size - 1);
		bdelete(undo->size);
		break;
	case ACT_DELETE:
		bpnttomrk(undo->end);
		bmove(1);
		for (i = 0; i < undo->size; ++i)
			binsert(undo->data[i]);
	}

	recycle_undo(Curbuff);

	if (!Curbuff->undo_tail)
		/* Last undo */
		Curbuff->bmodf = FALSE;
}
#else
void Zundo(void) { Tbell(); }
void undo_add(int size) {}
void undo_del(int size) {}
void undo_clear(struct buff *buff) {}
#endif
