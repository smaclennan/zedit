/* undo.c - Zedit undo commands
 * Copyright (C) 1988-2013 Sean MacLennan
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

/* BUGS:
 *
 * SERIOUS: Currently the changes cannot span page boundaries
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

static bool InUndo;

unsigned long undo_total; /* stats only */

static struct undo *new_undo(struct buff *buff, int action, int size)
{
	struct undo *undo = calloc(1, sizeof(struct undo));
	if (!undo)
		return NULL;

	undo->action = action;
	undo->size = size;
	if (action == ACT_INSERT)
		undo->end = bcremrk();
	else {
		undo->end = calloc(1, sizeof(struct mark));
		if (!undo->end) {
			free(undo);
			return NULL;
		}
		bmrktopnt(undo->end);
	}
	undo->prev = buff->undo_tail;
	buff->undo_tail = undo;

	undo_total += sizeof(struct undo) + sizeof(struct mark);

	return undo;
}

static void free_undo(struct buff *buff)
{
	struct undo *undo = buff->undo_tail;
	if (undo) {
		buff->undo_tail = undo->prev;

		if (undo->action == ACT_INSERT)
			unmark(undo->end);
		else
			free(undo->end);
		if (undo->data)
			free(undo->data);

		undo_total -= sizeof(struct undo) + sizeof(struct mark) +
			undo->size;

		free(undo);

	}
}

static inline int no_undo(struct buff *buff)
{
	return InUndo || buff->bname == NULL || *buff->bname == '*';
}

/* Exports */

void undo_add(int size)
{
	struct undo *undo = Curbuff->undo_tail;

	if (no_undo(Curbuff))
		return;

	if (undo && undo->action == ACT_INSERT && bisatmrk(undo->end)) {
		undo->size += size;
		bmrktopnt(undo->end);
		return;
	}

	/* need a new undo */
	undo = new_undo(Curbuff, ACT_INSERT, size);
	if (undo == NULL)
		return;
}

static void undo_append(struct undo *undo, Byte *data, int size)
{
	Byte *new = realloc(undo->data, undo->size + size);
	if (!new)
		return;

	memcpy(new + undo->size, data, size);

	undo->data = new;
	undo->size += size;

	undo_total += size;
}

static void undo_prepend(struct undo *undo, Byte *data, int size)
{
	Byte *new = realloc(undo->data, undo->size + size);
	if (!new)
		return;

	/* Shift the old */
	memmove(new + size, new, undo->size);
	/* Move in the new */
	memcpy(new, data, size);

	undo->data = new;
	undo->size += size;

	undo_total += size;
}

/* Size is always within the current page. */
void undo_del(int size)
{
	struct undo *undo = Curbuff->undo_tail;

	if (no_undo(Curbuff))
		return;

	if (size == 0) /* this can happen on page boundaries */
		return;

	/* We are not going to deal with page boundaries for now */
	/* We also only merge simple deletes */
	if (undo && undo->action == ACT_DELETE && undo->end->mpage == Curpage) {
		switch (Lfunc) {
		case ZDELETE_CHAR:
			if (Curchar == undo->end->moffset) {
				undo_append(undo, Curcptr, size);
				return;
			}
			break;
		case ZDELETE_PREVIOUS_CHAR:
			if (Curchar == undo->end->moffset - 1) {
				undo_prepend(undo, Curcptr, size);
				--undo->end->moffset;
				return;
			}
			break;
		}
	}

	/* need a new undo */
	undo = new_undo(Curbuff, ACT_DELETE, size);
	if (undo == NULL)
		return;

	undo->data = malloc(size);
	if (!undo->data) {
		free_undo(Curbuff);
		return;
	}

	memcpy(undo->data, Curcptr, size);

	undo_total += size;
}

void undo_clear(struct buff *buff)
{
	while (buff->undo_tail)
		free_undo(buff);
}

void Zundo(void)
{
	struct undo *undo = Curbuff->undo_tail;
	int i;

	if (!Curbuff->undo_tail) {
		tbell();
		return;
	}

	InUndo = true;
	bpnttomrk(undo->end);

	switch (undo->action) {
	case ACT_INSERT:
		bmove(-undo->size - 1);
		bdelete(undo->size);
		break;
	case ACT_DELETE:
		for (i = 0; i < undo->size; ++i)
			binsert(undo->data[i]);
	}
	InUndo = false;

	free_undo(Curbuff);

	if (!Curbuff->undo_tail)
		/* Last undo */
		Curbuff->bmodf = false;
}
#else
void Zundo(void) { tbell(); }
#endif
