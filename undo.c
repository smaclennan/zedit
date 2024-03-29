/* Copyright (C) 1988-2018 Sean MacLennan */

#include "buff.h"

/** @addtogroup zedit
 * @{
 */

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
	struct mark mrk;
	int size;
};

#define is_insert(u) ((u)->data == NULL)

static struct undo *new_undo(struct buff *buff, void **tail,
				 int insert, int size)
{
	struct undo *undo = (struct undo *)calloc(1, sizeof(struct undo));

	if (!undo)
		return NULL;

	undo->size = size;
	undo->offset = blocation(buff);
	bmrktopnt(buff, &undo->mrk);
	if (!insert) {
		undo->data = (Byte *)malloc(size);
		if (!undo->data) {
			free(undo);
			return NULL;
		}
	}
	undo->prev = *tail;
	*tail = undo;

	return undo;
}

static void free_undo(void **tail)
{
	struct undo *undo = (struct undo *)*tail;

	if (undo) {
		*tail = undo->prev;

		free(undo->data);
		free(undo);
	}
}

static int add_clumped(struct buff *buff, struct undo *undo, int size)
{
	int left;
	struct mark *mrk = &undo->mrk;

	/* move the undo mark */
	left = mrk->mpage->plen - mrk->moffset;
	if (left >= size)
		mrk->moffset += size;
	else {
		size -= left;
		while (size > 0) {
			mrk->mpage = mrk->mpage->nextp;
			mrk->moffset = 0;
			if (!mrk->mpage)
				return 0;
			if (mrk->mpage->plen >= (unsigned int)size) {
				mrk->moffset = size;
				size = 0;
			} else
				size -= mrk->mpage->plen;
		}
	}

	return bisatmrk(buff, mrk);
}

/* Always within current buffer page... may not be current mrk page */
static int del_clumped(struct buff *buff, struct undo *undo, int size)
{
	struct mark *mrk = &undo->mrk;

	if (bisatmrk(buff, mrk))
		return 1;

	/* Move mark back */
	if (mrk->moffset >= (unsigned int)size) {
		mrk->moffset -= size;
	} else if (mrk->mpage->prevp) { /* paranoia */
		mrk->mpage = mrk->mpage->prevp;
		mrk->moffset = mrk->mpage->plen - size;
	}

	if (bisatmrk(buff, mrk))
		return -1;

	return 0;
}

/* Exports */

void undo_add(struct buff *buff, int size)
{
	struct undo *undo = (struct undo *)buff->undo_tail;

	if (buff->in_undo)
		return;

	if (undo && is_insert(undo) && add_clumped(buff, undo, size)) {
		/* clump with last undo */
		undo->size += size;
		undo->offset += size;
	} else
		/* need a new undo */
		undo = new_undo(buff, &buff->undo_tail, 1, size);

	bmrktopnt(buff, &undo->mrk);
}

static void undo_append(struct undo *undo, Byte *data)
{
	Byte *buf = (Byte *)realloc(undo->data, undo->size + 1);

	if (!buf)
		return;

	buf[undo->size++] = *data;

	undo->data = buf;
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
}

/* Size is always within the current page. */
void undo_del(struct buff *buff, int size)
{
	struct undo *undo = (struct undo *)buff->undo_tail;

	if (buff->in_undo)
		return;

	if (size == 0) /* this can happen on page boundaries */
		return;

	if (undo && !is_insert(undo))
		switch (del_clumped(buff, undo, size)) {
		case 1: /* delete forward */
			undo_append(undo, buff->curcptr);
			return;
		case -1: /* delete previous */
			undo_prepend(undo, buff->curcptr);
			undo->offset--;
			return;
		}

	/* need a new undo */
	undo = new_undo(buff, &buff->undo_tail, 0, size);
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
	buff->in_undo = 1;
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

	buff->in_undo = 0;
	return 0;
}
#endif
/* @} */
