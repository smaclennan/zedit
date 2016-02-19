/* undo.c - undo functions
 * Copyright (C) 1988-2016 Sean MacLennan
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

#include "buff.h"

#if defined(UNDO) && UNDO
#include <stdlib.h>
#include <string.h>

/* We cannot use marks here since pages may come and go and the marks
 * become useless. Use an offset instead.
 */
struct undo {
	struct undo *prev;
	Byte *data;
	unsigned long offset;
	int size;
};

#define is_insert(u) ((u)->data == NULL)

unsigned long undo_total; /* stats only */

static struct undo *new_undo(struct buff *buff, void **tail, bool insert, int size)
{
	struct undo *undo = (struct undo *)calloc(1, sizeof(struct undo));
	if (!undo)
		return NULL;

	undo->size = size;
	undo->offset = blocation(buff);
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

/* Exports */

void undo_add(struct buff *buff, int size, bool clumped)
{
	if (buff->in_undo)
		return;

	struct undo *undo = (struct undo *)buff->undo_tail;

	if (undo && is_insert(undo) && (clumped || undo_add_clumped(buff, size))) {
		/* clump with last undo */
		undo->size += size;
		undo->offset += size;
	} else
		/* need a new undo */
		undo = new_undo(buff, &buff->undo_tail, true, size);
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
void undo_del(struct buff *buff, int size)
{
	if (buff->in_undo)
		return;

	if (size == 0) /* this can happen on page boundaries */
		return;

	struct undo *undo = (struct undo *)buff->undo_tail;

	/* We only merge simple deletes */
	if (undo && !is_insert(undo))
		switch (undo_del_clumped(buff, size)) {
		case 1: /* delete forward */
			undo_append(undo, buff->curcptr);
			return;
		case -1: /* delete previous */
			undo_prepend(undo, buff->curcptr);
			undo->offset--;
			return;
		}

	/* need a new undo */
	undo = new_undo(buff, &buff->undo_tail, false, size);
	if (undo == NULL)
		return;

	memcpy(undo->data, buff->curcptr, size);
}

void undo_clear(struct buff *buff)
{
	while (buff->undo_tail)
		free_undo(&buff->undo_tail);
}

int do_undo(struct buff *buff)
{
	struct undo *undo;
	int i;

	if (!buff->undo_tail)
		return 1;

	undo = (struct undo *)buff->undo_tail;
	buff->in_undo = true;
	boffset(buff, undo->offset);

	if (is_insert(undo)) {
		bmove(buff, -undo->size);
		bdelete(buff, undo->size);
		free_undo(&buff->undo_tail);
	} else {
		unsigned long offset = undo->offset;
		struct mark *tmark = bcremark(buff);
		if (!tmark)
			return 1;
		do {
			for (i = 0; i < undo->size; ++i)
				binsert(buff, undo->data[i]);
			free_undo(&buff->undo_tail);
			undo = (struct undo *)buff->undo_tail;
		} while (undo && !is_insert(undo) && undo->offset == offset);
		bpnttomrk(buff, tmark);
		bdelmark(tmark);
	}

	buff->in_undo = false;
	return 0;
}
#endif
