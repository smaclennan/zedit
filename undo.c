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

static struct undo *new_undo(void **tail, bool insert, int size)
{
	struct undo *undo = (struct undo *)calloc(1, sizeof(struct undo));
	if (!undo)
		return NULL;

	undo->size = size;
	undo->offset = blocation(Curbuff);
	if (!insert) {
		undo->data = (Byte *)malloc(size);
		if (!undo->data) {
			free(undo);
			return NULL;
		}
		undo_total += size;
	}
	undo->prev = *tail;
	*tail = undo;

	undo_total += sizeof(struct undo);

	return undo;
}

static void free_undo(void **tail)
{
	struct undo *undo = (struct undo *)*tail;
	if (undo) {
		*tail = undo->prev;

		undo_total -= sizeof(struct undo) + undo->size;

		if (undo->data)
			free(undo->data);
		free(undo);
	}
}

static inline int no_undo(struct buff *buff)
{
	return !buff->app || VAR(VUNDO) == 0 || InUndo || *zapp(buff)->bname == '*';
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
	if (no_undo(Curbuff))
		return;

	struct zapp *app = Curbuff->app;
	struct undo *undo = (struct undo *)app->undo_tail;

	if (undo && is_insert(undo) && clump()) {
		/* clump with last undo */
		undo->size += size;
		undo->offset += size;
	} else
		/* need a new undo */
		undo = new_undo(&app->undo_tail, true, size);
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
	if (no_undo(Curbuff))
		return;

	struct zapp *app = Curbuff->app;
	struct undo *undo = (struct undo *)app->undo_tail;

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
	undo = new_undo(&app->undo_tail, false, size);
	if (undo == NULL)
		return;

	memcpy(undo->data, Curcptr, size);
}

void undo_clear(struct buff *buff)
{
	struct zapp *app = buff->app;
	if (app)
		while (app->undo_tail)
			free_undo(&app->undo_tail);
}

void Zundo(void)
{
	struct zapp *app = Curbuff->app;
	struct undo *undo;
	int i;

	if (!app || !app->undo_tail) {
		tbell();
		return;
	}

	undo = (struct undo *)app->undo_tail;
	InUndo = true;
	boffset(Curbuff, undo->offset);

	if (is_insert(undo)) {
		bmove(Curbuff, -undo->size);
		bdelete(Curbuff, undo->size);
		free_undo(&app->undo_tail);
	} else {
		unsigned long offset = undo->offset;
		do {
			struct mark *tmark = zcreatemrk();
			for (i = 0; i < undo->size; ++i)
				binsert(Curbuff, undo->data[i]);
			bpnttomrk(Curbuff, tmark);
			unmark(tmark);
			free_undo(&app->undo_tail);
			undo = (struct undo *)app->undo_tail;
		} while (undo && !is_insert(undo) && undo->offset == offset);
	}

	InUndo = false;

	if (!app->undo_tail)
		/* Last undo */
		Curbuff->bmodf = false;
}
#else
void Zundo(void) { tbell(); }
#endif
