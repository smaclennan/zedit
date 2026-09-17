/* Copyright (C) 1988-2017 Sean MacLennan */

#include "z.h"

/** @addtogroup zedit
 * @{
 */


static struct buff *Killbuff;

void delinit(void)
{
	Killbuff = bcreate();
}

void delfini(void)
{
	bdelbuff(Killbuff);
}

unsigned int delpages(void)
{
	unsigned int npage = 0;
	for (struct page *page = Killbuff->firstp; page; page = page->nextp)
		++npage;
	return npage;
}

/* Was the last command a delete to kill buffer command? */
bool delcmd(void)
{
	switch (Lfunc) {
	case ZDELETE_TO_EOL:
	case ZDELETE_LINE:
	case ZDELETE_REGION:
	case ZDELETE_WORD:
	case ZDELETE_PREVIOUS_WORD:
	case ZCOPY_REGION:
	case ZCOPY_WORD:
	case ZAPPEND_KILL:
		return true;
	default:
		return false;
	}
}

static void copytomrk(struct mark *tmark)
{
	if (delcmd())
		btoend(Killbuff);
	else
		bempty(Killbuff);
	bcopyrgn(tmark, Killbuff);
}

void killtomrk(struct mark *tmark)
{
	copytomrk(tmark);
	bdeltomrk(tmark);
}

void Zappend_kill(void) {}

void Zdelete_char(void)
{
	bdelete(Bbuff, Arg);
	Arg = 0;
}

void Zdelete_previous_char(void)
{
	bmove(Bbuff, -Arg);
	bdelete(Bbuff, Arg);
	Arg = 0;
}

void Zdelete_to_eol(void)
{
	if (!bisend(Bbuff) && Buff() == NL)
		bdelete(Bbuff, 1);
	else {
		bool atstart = bpeek(Bbuff) == NL;
		struct mark *tmark = bcremark(Bbuff);
		if (!tmark) {
			tbell();
			return;
		}
		toendline(Bbuff);
		if (atstart)
			bmove1(Bbuff); /* delete the NL */
		killtomrk(tmark);
		bdelmark(tmark);
	}
}

void Zdelete_line(void)
{
	struct mark *tmark = bcremark(Bbuff);
	if (!tmark) {
		tbell();
		return;
	}

	tobegline(Bbuff);
	bmrktopnt(Bbuff, tmark);
	bcsearch(Bbuff, NL);
	killtomrk(tmark);
	bdelmark(tmark);
}

void Zdelete_region(void)
{
	NEED_UMARK;
	killtomrk(UMARK);
	CLEAR_UMARK;
}

void Zcopy_region(void)
{
	NEED_UMARK;
	copytomrk(UMARK);
	CLEAR_UMARK;
}


void Zyank(void)
{
	struct mark *tmark;	/* save must NOT be a pointer */

	if (InPaw && First) {
		bempty(Bbuff);
		First = false;
	}

#if !UNDO
	/* This leaves the mark at the start of the yank and
	 * the point at the end.
	 */
	set_umark(NULL);
#endif
	btoend(Killbuff);
	tmark = bcremark(Killbuff);
	if (!tmark) {
		error("out of memory");
		return;
	}
	btostart(Killbuff);
#if UNDO
	undo_add(Bbuff, bcopyrgn(tmark, Bbuff));
#else
	bcopyrgn(tmark, Bbuff);
#endif
	bdelmark(tmark);
}

void Zdelete_word(void)
{
	struct mark *tmark = bcremark(Bbuff);
	if (!tmark) {
		tbell();
		return;
	}
	bmoveto(Bbuff, bisword, FORWARD);
	bmovepast(Bbuff, bisword, FORWARD);
	killtomrk(tmark);
	bdelmark(tmark);
}

void Zdelete_previous_word(void)
{
	struct mark *tmark = bcremark(Bbuff);
	if (!tmark) {
		tbell();
		return;
	}
	Zprevious_word();
	killtomrk(tmark);
	bdelmark(tmark);
}

void Zcopy_word(void)
{
	char word[STRMAX], *ptr;

	if (InPaw) {
		zswitchto(Buff_save);
		getbword(word, STRMAX, bistoken);
		zswitchto(Paw);
		for (ptr = word; *ptr; ++ptr) {
			Cmd = *ptr;
			pinsert();
		}
	} else {
		struct mark *tmark = bcremark(Bbuff); /* save current Point */
		struct mark *start = bcremark(Bbuff);
		if (tmark && start) {
			 /* find start of word */
			bmoveto(Bbuff, bistoken, FORWARD);
			bmovepast(Bbuff, bistoken, BACKWARD);
			bmrktopnt(Bbuff, start);
			/* move Point to end of word */
			bmovepast(Bbuff, bistoken, FORWARD);
			/* copy to Kill buffer */
			copytomrk(start);
			/* move Point back */
			bpnttomrk(Bbuff, tmark);
		} else
			tbell();
		bdelmark(tmark);
		bdelmark(start);
	}
	Arg = 0;
}

void Zdelete_blanks(void)
{
	struct mark *pmark = bcremark(Bbuff);
	struct mark *tmark = bcremark(Bbuff);
	if (!pmark || !tmark) {
		tbell();
		goto done;
	}

	if (Argp) {
		struct regexp re;
		Arg = 0;

		if (re_compile(&re, "^[ \t]*\r?$", REG_EXTENDED)) {
			error("Internal re error.");
			goto done;
		}

		btostart(Bbuff);
		while (re_step(Bbuff, &re, tmark)) {
			bmove1(Bbuff); /* skip over the NL */
			bdeltomrk(tmark);
		}

		re_free(&re);
		goto done;
	}

	if (bcrsearch(Bbuff, NL)) {
		bmove1(Bbuff);
		bmrktopnt(Bbuff, tmark);
		bmovepast(Bbuff, isspace, BACKWARD);
		if (!bisstart(Bbuff))
			bcsearch(Bbuff, NL);
		if (bisbeforemrk(Bbuff, tmark))
			bdeltomrk(tmark);
	}
	if (bcsearch(Bbuff, NL)) {
		bmrktopnt(Bbuff, tmark);
		bmovepast(Bbuff, isspace, FORWARD);
		if (bcrsearch(Bbuff, NL))
			bmove1(Bbuff);
		if (bisaftermrk(Bbuff, tmark))
			bdeltomrk(tmark);
	}

done:
	if (pmark)
		bpnttomrk(Bbuff, pmark);
	bdelmark(pmark);
	bdelmark(tmark);
}

void Zjoin(void)
{
	toendline(Bbuff);
	bdelete(Bbuff, 1);
	Ztrim_white_space();
	binsert(Bbuff, ' ');
}

void Zempty_buffer(void)
{
	if (ask("Empty buffer? ") != YES)
		return;
	bempty(Bbuff);
	Bbuff->bmodf = true;
}
/* @} */
