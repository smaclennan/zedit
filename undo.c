#include "z.h"

#if UNDO

#define ACT_INSERT 1
#define ACT_DELETE 2

struct undo {
	int action;
	Mark *end;
	Byte *data;
	int size;
	struct undo *prev, *next;
};

static struct undo *freelist;

static struct undo *new_undo(Buffer *buff)
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

	undo->end = Bcremrk();
	undo->prev = buff->undo_tail;
	buff->undo_tail = undo;

	return undo;
}

static void recycle_undo(struct undo *undo)
{
	if (undo->data)
		free(undo->data);

	undo->prev = freelist;
	freelist = undo;
}

/* Exports */

void undo_add(int size)
{
	struct undo *undo = Curbuff->undo_tail;

	if (undo && undo->action == ACT_INSERT && Bisatmrk(undo->end)) {
		Dbg("undo_add: add to current p %p.%d u %p.%d\n",
		    Curpage, Curchar, undo->end->mpage, undo->end->moffset); // SAM DBG
		undo->size += size;
		Bmrktopnt(undo->end);
		return;
	}

	Dbg("undo_add: new p %p.%d\n", Curpage, Curchar); // SAM DBG

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

	if (size == 0) /* this can happen on page boundaries */
		return;

	/* SAM merge deletes! */

	/* need a new undo */
	undo = new_undo(Curbuff);
	if (undo == NULL)
		return;

	undo->data = malloc(size);
	if (!undo->data) {
		recycle_undo(undo);
		return;
	}

	memcpy(undo->data, Curcptr, size);

	undo->action = ACT_DELETE;
	undo->size = size;
}


Proc Zundo(void)
{
	struct undo *undo = Curbuff->undo_tail;
	int i;

	if (!undo) {
		Tbell();
		return;
	}

	switch(undo->action) {
	case ACT_INSERT:
		Bpnttomrk(undo->end);
		Bmove(-undo->size - 1);
		Bdelete(undo->size);
		break;
	case ACT_DELETE:
		Bpnttomrk(undo->end);
		Bmove(1);
		for (i = 0; i < undo->size; ++i)
			Binsert(undo->data[i]);
	}

	Curbuff->undo_tail = undo->prev;
	recycle_undo(undo);
}
#else
Proc Zundo(void) { Tbell(); }
#endif
