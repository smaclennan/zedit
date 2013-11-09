/* delete.c - Zedit delete commands
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


struct buff *Killbuff;

static void copytomrk(struct mark *tmark)
{
	struct buff *save = Curbuff;
	bswitchto(Killbuff);
	if (delcmd())
		btoend();
	else
		bempty();
	bswitchto(save);
	bcopyrgn(tmark, Killbuff);
}

void killtomrk(struct mark *tmark)
{
	copytomrk(tmark);
	bdeltomrk(tmark);
}

/***
 * Sets the delete flag. The next delete command will append to the kill
 * buffer. A Universal Argument is ignored.
 */
void Zappend_kill(void) {}

/***
 * Deletes the character at the Point and leaves the Point on the next
 * character in the buffer. The character is not put in the Kill Buffer. A
 * Universal Argument causes the command to repeat.
 */
void Zdelete_char(void)
{
	bdelete(Arg);
	Arg = 0;
}

/***
 * Deletes the character before the Point and leaves the Point in the same
 * place. The character is not put in the Kill Buffer. A Universal Argument
 * causes the command to repeat.
 */
void Zdelete_previous_char(void)
{
	bmove(-Arg);
	bdelete(Arg);
	Arg = 0;
}

/***
 * Deletes the characters from the Point to the end of the line. If the
 * Point is at the end of a line, the Newline character is deleted and the
 * next line is joined to the end of the current line. The characters
 * deleted are put in the Kill Buffer. A Universal Argument causes the
 * command to repeat.
 */
void Zdelete_to_eol(void)
{
	struct mark *tmark = bcremrk();

	if (!bisend() && Buff() == NL)
		bmove1();
	else if (VAR(VKILLLINE)) {
		bool atstart;

		tobegline();
		atstart = bisatmrk(tmark);
		toendline();
		if (atstart)
			bmove1(); /* delete the NL */
	} else
		toendline();
	killtomrk(tmark);
	unmark(tmark);
}

/***
 * Deletes the entire line, including the Newline, no matter where the
 * Point is in the line. The Point is left at the start of the next line.
 * The deleted line is put in the Kill Buffer. A Universal Argument causes
 * the command to repeat.
 */
void Zdelete_line(void)
{
	struct mark *tmark;

	tobegline();
	tmark = bcremrk();
	bcsearch(NL);
	killtomrk(tmark);
	unmark(tmark);
}

/***
 * Deletes the characters in the region. The deleted characters are put in
 * the Kill Buffer based on the delete flag. A Universal Argument is
 * ignored.
 */
void Zdelete_region(void)
{
	killtomrk(Curbuff->mark);
}

/***
 * Copies the region to the kill buffer. The kill buffer is overwritten
 * unless the the delete flag is set. See Append Kill command.
 */
void Zcopy_region(void)
{
	copytomrk(Curbuff->mark);
}


/***
 * Inserts the characters from the Kill Buffer before the Point. The
 * characters are inserted, even in overwrite mode. A Universal Argument
 * causes the command to repeat.
 */
void Zyank(void)
{
	struct buff *tbuff;
	int yanked;
	struct mark *tmark, save;	/* save must NOT be a pointer */

	if (InPaw && First) {
		bdelete(Curplen);
		First = false;
	}

	mrktomrk(&save, Send);
	tbuff = Curbuff;
	bmrktopnt(Curbuff->mark);
	bswitchto(Killbuff);
	btoend();
	tmark = bcremrk();
	btostart();
	yanked = bcopyrgn(tmark, tbuff);
	unmark(tmark);
	bswitchto(tbuff);
	undo_add(yanked);
	if (bisaftermrk(&save))
		reframe();
}

/***
 * Deletes the word to the right of and including the Point. The word is
 * put in the Kill Buffer. A Universal Argument causes the command to
 * repeat.
 */
void Zdelete_word(void)
{
	struct mark *tmark;

	tmark = bcremrk();
	moveto(bisword, FORWARD);
	movepast(bisword, FORWARD);
	killtomrk(tmark);
	unmark(tmark);
}

/***
 * Deletes the word to the left of the Point. The character the Point is on
 * is not deleted. The word is put in the Kill Buffer. A Universal Argument
 * causes the command to repeat.
 */
void Zdelete_previous_word(void)
{
	struct mark *tmark;

	tmark = bcremrk();
	Zprevious_word();
	killtomrk(tmark);
	unmark(tmark);
}

/***
 * Copies the word the Point is on to the kill buffer. The kill buffer is
 * overwritten unless the the delete flag is set.In the PAW, the Copy Word
 * command takes the word the Point was on in the previously active window
 * and inserts it into the PAW.
 */
void Zcopy_word(void)
{
	char word[STRMAX], *ptr;
	struct mark *tmark, *start;

	if (InPaw) {
		bswitchto(Buff_save);
		getbword(word, STRMAX, bistoken);
		bswitchto(Paw);
		for (ptr = word; *ptr; ++ptr) {
			Cmd = *ptr;
			pinsert();
		}
	} else {
		tmark = bcremrk();	/* save current Point */
		moveto(bistoken, FORWARD); /* find start of word */
		movepast(bistoken, BACKWARD);
		start = bcremrk();
		movepast(bistoken, FORWARD); /* move Point to end of word */
		copytomrk(start); /* copy to Kill buffer */
		bpnttomrk(tmark); /* move Point back */
		unmark(tmark);
		unmark(start);
	}
	Arg = 0;
}

/***
 * Delete all the blank lines around the Point. The lines are not put in
 * the Kill Buffer. A Universal Argument causes the command to repeat,
 * which accomplishes nothing.
 */
void Zdelete_blanks(void)
{
	struct mark *tmark, *pmark;

	pmark = bcremrk();
	if (bcrsearch(NL)) {
		bmove1();
		tmark = bcremrk();
		movepast(bisspace, BACKWARD);
		if (!bisstart())
			bcsearch(NL);
		if (bisbeforemrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	if (bcsearch(NL)) {
		tmark = bcremrk();
		movepast(bisspace, FORWARD);
		if (bcrsearch(NL))
			bmove1();
		if (bisaftermrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	bpnttomrk(pmark);
	unmark(pmark);
}

/***
 * Joins two lines. Performs the following Zedit commands:
 * End of Line, Delete Newline, Trim Whitespace, Insert space.
 */
void Zjoin(void)
{
	toendline();
	bdelete(1);
	Ztrim_white_space();
	binsert(' ');
}

/***
 * Deletes the entire contents of the current buffer. A Universal Arguments
 * is ignored.
 */
void Zempty_buffer(void)
{
	if (ask("Empty buffer? ") != YES)
		return;
	bempty();
	Curbuff->bmodf = true;
}
