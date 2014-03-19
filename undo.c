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

#include "z.h"

#if UNDO

#define is_insert(u) ((u)->data == NULL)

/* We cannot use marks here since pages may come and go and the marks
 * become useless. Use an offset instead.
 */
struct undo {
	struct undo *prev;
	Byte *data;
	unsigned long offset;
	int size;
};

static bool InUndo;

unsigned long undo_total; /* stats only */

static struct undo *new_undo(struct buff *buff, bool insert, int size)
{
	struct undo *undo = (struct undo *)calloc(1, sizeof(struct undo));
	if (!undo)
		return NULL;

	undo->size = size;
	undo->offset = blocation(NULL);
	if (!insert) {
		undo->data = (Byte *)malloc(size);
		if (!undo->data) {
			free(undo);
			return NULL;
		}
		undo_total += size;
	}
	undo->prev = (struct undo *)buff->undo_tail;
	buff->undo_tail = undo;

	undo_total += sizeof(struct undo);

	return undo;
}

static void free_undo(struct buff *buff)
{
	struct undo *undo = (struct undo *)buff->undo_tail;
	if (undo) {
		buff->undo_tail = undo->prev;

		undo_total -= sizeof(struct undo) + undo->size;

		if (undo->data)
			free(undo->data);
		free(undo);
	}
}

static inline int no_undo(struct buff *buff)
{
	return InUndo || buff->bname == NULL || *buff->bname == '*';
}

static inline bool clump(void)
{	/* commands we clump together */
	switch (Lfunc) {
	case ZINSERT:
	case ZNEWLINE:
	case ZTAB:
	case ZC_INSERT:
	case ZC_INDENT:
		return true;
	default:
		return false;
	}
}

/* Exports */

void undo_add(int size)
{
	struct undo *undo = (struct undo *)Curbuff->undo_tail;

	if (no_undo(Curbuff))
		return;

	if (undo && is_insert(undo) && clump()) {
		/* clump with last undo */
		undo->size += size;
		undo->offset += size;
	} else
		/* need a new undo */
		undo = new_undo(Curbuff, true, size);
}

static void undo_append(struct undo *undo, Byte *data)
{
	Byte *buf = (Byte *)realloc(undo->data, undo->size + 1);
	if (!buf)
		return;

	buf[undo->size++] = *data;

	undo->data = buf;

	undo_total++;
}

static void undo_prepend(struct undo *undo, Byte *data)
{
	Byte *buf = (Byte *)realloc(undo->data, undo->size + 1);
	if (!buf)
		return;

	/* Shift the old */
	memmove(buf + 1, buf, undo->size);
	/* Move in the new */
	*buf = *data;

	undo->data = buf;
	undo->size++;

	undo_total++;
}

/* Size is always within the current page. */
void undo_del(int size)
{
	struct undo *undo = (struct undo *)Curbuff->undo_tail;

	if (no_undo(Curbuff))
		return;

	if (size == 0) /* this can happen on page boundaries */
		return;

	/* We only merge simple deletes */
	if (undo && !is_insert(undo) && size == 1) {
		switch (Lfunc) {
		case ZDELETE_CHAR:
			undo_append(undo, Curcptr);
			return;
		case ZDELETE_PREVIOUS_CHAR:
			undo_prepend(undo, Curcptr);
			undo->offset--;
			return;
		}
	}

	/* need a new undo */
	undo = new_undo(Curbuff, false, size);
	if (undo == NULL)
		return;

	memcpy(undo->data, Curcptr, size);
}

void undo_clear(struct buff *buff)
{
	while (buff->undo_tail)
		free_undo(buff);
}

void Zundo(void)
{
	struct undo *undo = (struct undo *)Curbuff->undo_tail;
	int i;

	if (!Curbuff->undo_tail) {
		tbell();
		return;
	}

	InUndo = true;
	boffset(undo->offset);

	if (is_insert(undo)) {
		bmove(-undo->size);
		bdelete(undo->size);
		free_undo(Curbuff);
	} else {
		unsigned long offset = undo->offset;
		do {
			struct mark *tmark = bcremrk();
			for (i = 0; i < undo->size; ++i)
				binsert(undo->data[i]);
			bpnttomrk(tmark);
			unmark(tmark);
			free_undo(Curbuff);
			undo = (struct undo *)Curbuff->undo_tail;
		} while (undo && !is_insert(undo) && undo->offset == offset);
	}

	InUndo = false;

	if (!Curbuff->undo_tail)
		/* Last undo */
		Curbuff->bmodf = false;
}
#else
void Zundo(void) { tbell(); }
#endif
