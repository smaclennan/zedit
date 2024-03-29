/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"

/** @addtogroup zedit
 * @{
 */

#define PNUMCOLS		3	/* default columns for pout */
#define PCOLSIZE		26	/* default column size */

/* globals for getarg */
int InPaw;
bool First;
int Pawcol, Pawlen, Pshift;
struct zbuff *Paw;
struct zbuff *Buff_save;

/* globals for getplete */
static char **Carray;
static int Csize, Cnum = 0, Cret;
void (*Nextpart)(void);

/* General purpose string argument input routine which recursively calls the
 * editor through the PAW buffer.
 * Returns 0 if ok, ABORT if user aborted, 1 if empty string.
 * Prompt the user for an argument, with an optional default in arg.
 * Only allow max chars.
 * Arg is NOT overwritten if the user aborts, or returns a null string.
 */
bool _getarg(const char *prompt, char *arg, int max, bool tostart)
{
	char *ptr;
	int argp_save, arg_save, rc;
	int tcol, trow;

	tcol = Pcol; trow = Prow;
	t_goto(Rowmax - 1, 0); /* display the prompt */
	tprntstr(prompt);
	Pawcol = Pcol = strlen(prompt); /* prompts are always simple ascii */

	argp_save = Argp;
	arg_save = Arg;
	Buff_save = Curbuff;
	Paw->bmode = Curbuff->bmode;
	InPaw = true;
	Curcmds = 1;
	Pshift = 0;
	Pawlen = max;
	makepaw(arg, false);
	if (tostart) {
		btostart(Bbuff);
		First = true;
	}
	while (InPaw == true)
		execute();
	if (InPaw != ABORT) {
		/* get the argument */
		btostart(Bbuff);
		for (ptr = arg; !bisend(Bbuff) && Buff() != '\0'; bmove1(Bbuff))
			*ptr++ = Buff();
		*ptr = '\0';
		rc = ptr == arg;	/* set to 1 if string empty */
	} else
		rc = ABORT;
	InPaw = false;
	Insearch = false; /* used by Zcase when in search command */
	Argp = argp_save;
	Arg = arg_save;
	zswitchto(Buff_save);		/* go back */
	Curbuff->bmode = Paw->bmode;	/* mainly for EXACT mode */
	t_goto(trow, tcol);
	Curwdo->modeflags = INVALID;
	Curcmds = 0;
	clrpaw();
	return rc;
}

bool getarg(const char *prompt, char *arg, int max)
{
	return _getarg(prompt, arg, max, true);
}

/* General purpose ask for an argument with completion given a struct
 * or char * array.
 * Returns offset of entry in array if found, else -1.
 */
int getplete(const char *prompt, const char *def, char **array,
		 int size, int num)
{
	char cmdstr[STRMAX + 1];

	Carray = array;
	Csize = size / sizeof(char *);
	Cnum = num;
	if (def)
		strcpy(cmdstr, def);
	else
		*cmdstr = '\0';
	if (getarg(prompt, cmdstr, STRMAX))
		Cret = -1;
	Cnum = 0;
	return Cret;
}

static int p_row, p_col;

static void pclear(void)
{
	int i;
	struct wdo *wdo;

	/* We can't use redisplay here */
	foreachwdo(wdo)
		wdo->modeflags = INVALID;

	for (i = 0; i < Rowmax - 2; ++i) {
		t_goto(i, 0);
		tcleol();
	}
	invalidate_scrnmarks(0, Rowmax - 2);
	p_row = p_col = 0;
}

static void pout(char *str)
{
	t_goto(p_row, p_col * PCOLSIZE);
	invalidate_scrnmarks(p_row, p_row + 1);
	if (p_row < Rowmax - 2) {
		tprntstr(str);
		tcleol();
	}
	if (++p_col >= PNUMCOLS) {
		++p_row;
		p_col = 0;
	}
}

static void pcmdplete(bool show)
{
	char cmd[STRMAX + 1], **cca, *mstr = NULL;
	int i = 0, len, len1 = 0, rc;

	Cret = -1;
	getbtxt(cmd, STRMAX);
	len = strlen(cmd);
	cca = Carray;
	if (show && !len)
		for (; i < Cnum; ++i, cca += Csize)
			pout(*cca);
	else
		for (; i < Cnum && (rc = strncasecmp(*cca, cmd, len)) <= 0;
				++i, cca += Csize)
			if (rc == 0) {
				if (show)
					pout(*cca);
				else if (mstr) {
					Cret = -1;
					len1 = nmatch(mstr, *cca);
				} else {
					Cret = i;
					len1 = strlen(mstr = *cca);
				}
			}

	if (show)
		return;

	if (mstr) {
		if (len == len1) {
			/* Help!!!!*/
			pclear();
			pcmdplete(true);
			return;
		}
		strncpy(cmd, mstr, len1);
		cmd[len1] = '\0';
		makepaw(cmd, false);
	} else
		tbell();
}

/* Use instead of Zinsert when in PAW */
void pinsert(void)
{
	struct mark tmark;
	int width;

	if (Cmd == '\t' && Cnum) {
		pcmdplete(false);
		return;
	}

	if (First) {
		bempty(Bbuff);
		First = false;
		invalidate_scrnmarks(Rowmax - 1, Rowmax);
	}

	width = chwidth(Cmd, Pcol, false);
	if (bgetcol(false, 0) + width <= Pawlen) {
		Zinsert();

		bmrktopnt(Bbuff, &tmark);
		btoend(Bbuff);
		if (bgetcol(false, 0) > Pawlen) {
			/* Insert in middle pushed text past end */
			bmove(Bbuff, -width);
			bdelete(Bbuff, width);
		}
		bpnttomrk(Bbuff, &tmark);
	} else
		tbell();
}

/* Use instead of Znewline when in PAW */
void pnewline(void)
{
	if (Cnum) {
		char cmdstr[STRMAX + 1], **ptr;
		int i, found = 0, len;

		getbtxt(cmdstr, STRMAX);
		len = strlen(cmdstr);
		for (i = 0, ptr = Carray; i < Cnum; ++i, ptr += Csize)
			if (strncasecmp(cmdstr, *ptr, len) == 0) {
				Cret = i;
				if (strcasecmp(cmdstr, *ptr) == 0) {
					/* Exact match */
					InPaw = false;
					return;
				}
				++found;
			}
		if (found != 1) {
			tbell();
			return;
		}
	}
	InPaw = false;
}

void Zpart(void)
{
	if (Nextpart)
		Nextpart();
	else
		tbell();
}
/* @} */
