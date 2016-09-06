/* z.c - Zedit mainline
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

#include "z.h"
#include <stdarg.h>

int verbose;
bool Initializing = true;
char *Home;

unsigned Cmd;
int Cmdpushed = -1; /* Search pushed a key */

#ifndef CONFIGDIR
#define CONFIGDIR "/usr/share/zedit"
#endif

static void usage(char *prog)
{
	printf(
		"usage: %s [-hrt] [-l line] [fname ...]\n"
		"where:\t-h  displays this message.\n"
		"\t-r  do not do CR/LF conversion.\n"
		"\t-t  default to text mode.\n"
		"\t-l  goto specified line number. (First file only)\n"
		, prog);

	exit(1);
}

/* Find the correct path for the config files. */
static void findpath(char *path, const char *f, void (*action)(const char *))
{
	snprintf(path, PATHMAX, "%s/%s", Home, f);
	if (access(path, F_OK) == 0)
		action(path);
	snprintf(path, PATHMAX, "%s/%s", CONFIGDIR, f);
	if (access(path, F_OK) == 0)
		action(path);
	if (access(f, F_OK) == 0)
		action(f);
}

/* NOTE: Dotty blocks */
static void dotty(void)
{
	if (Cmdpushed == -1)
		Cmd = tgetkb();
	else {
		Cmd = Cmdpushed;
		Cmdpushed = -1;
	}

	if (ring_bell) {
		ring_bell = 0;
		Curwdo->modeflags = INVALID;
	}

	Arg = 1;
	Argp = false;
	while (Arg > 0) {
		CMD(Keys[Cmd]);
		--Arg;
		Lfunc = Keys[Cmd]; /* for undo + delcmd */
	}
	First = false; /* used by pinsert when InPaw */
}

#ifdef DOPIPES
#include <poll.h>

#define MAX_FDS 2
#define PIPEFD 1

static struct pollfd fds[MAX_FDS];

static void fd_init(void)
{
	int i;
	for (i = 0; i < MAX_FDS; ++i) {
		fds[i].fd = -1;
		fds[i].events = POLLIN;
	}

	fds[0].fd = 0; /* stdin */
}

bool fd_add(int fd)
{
	if (fds[PIPEFD].fd == -1) {
		fds[PIPEFD].fd = fd;
		return true;
	}
	return false;
}

void fd_remove(int fd)
{
	if (fds[PIPEFD].fd == fd) {
		fds[PIPEFD].fd = -1;
	}
}
#else
static void fd_init(void) {}
#endif

void execute(void)
{
	zrefresh();

#ifdef DOPIPES
	if (tkbrdy() || Cmdpushed != -1)
		dotty();
	else {
		/* select returns -1 if a child dies (SIGPIPE) -
		 * sigchild handles it
		 */
		while (poll(fds, MAX_FDS, -1) == -1) {
			checkpipes(1);
			zrefresh();
		}

		if (fds[0].revents)
			dotty();
		if (fds[PIPEFD].revents)
			readapipe();
	}
#else
	dotty();
#endif
}

int main(int argc, char **argv)
{
	char path[PATHMAX + 1];
	int arg, textMode = 0, exitflag = 0, line = 0;
	struct mark tmark;
	struct zbuff *tbuff = NULL;

	Home = gethomedir();
	if (!Home) {
		puts("You don't exist!");
		exit(1);
	}

	snprintf(path, sizeof(path), "%s/z.out", Home);
	unlink(path);
	Dbgfname(path);

	while ((arg = getopt(argc, argv, "hl:rtvE")) != EOF)
		switch (arg) {
		case 'l':
			line = atoi(optarg);
			break;
		case 'r':
			raw_mode = 1;
			break;
		case 't':
			textMode = 1;
			break;
		case 'E':
			exitflag = true;
			break;
		case 'v':
			++verbose;
			break;
		case 'h':
		default:
			usage(argv[0]);
		}

	/* Do this BEFORE tinit */
	findpath(path, ZCFILE, readvfile);

	/* User wants Text mode as default */
	if (textMode)
		VAR(VNORMAL) = 0;

	delinit();

	/* PAW must not be on the Bufflist */
	if (!(Paw = cmakebuff("*paw*", NULL)))
		exit(1);
	Paw->prev = Paw->next = NULL;
	Bufflist = NULL;
	Curbuff = NULL;

	if (!cmakebuff(MAINBUFF, NULL)) {
		puts("Not enough memory.");
		exit(1);
	}

	bmrktopnt(Bbuff, &tmark);
	display_init(&tmark);

	tinit();
	tainit();
	fd_init();

	for (; optind < argc; ++optind)
		if (pathfixup(path, argv[optind]) == 0)
			if (findfile(path) && !tbuff)
				tbuff = Curbuff;

	if (tbuff) {
		zswitchto(tbuff);

		strcpy(Lbufname,
			   Curbuff->prev ? Curbuff->prev->bname : MAINBUFF);
	}

	winit();

	reframe();

	if (!Curbuff->mtime && Curbuff->fname)
		putpaw("New File");

	if (line) {
		Argp = true;
		Arg = line;
		Zgoto_line();
	}

	Initializing = false;

	if (exitflag)
		Zexit();

	while (1)
		execute();
}

/* Support functions */

/* For debugging */
const char *func2name(Byte func)
{
	int i;

	for (i = 0; i < NUMFUNCS; ++i)
		if (Cnames[i].fnum == func)
			return Cnames[i].name;
	return "???";
}

/* ask Yes/No question.
 * Returns YES, NO, BANG, or ABORT
 */
int ask2(const char *msg, bool allow_bang)
{
	int rc = BADCHAR;
	unsigned cmd;

	putpaw("%s", msg);
	while (rc == BADCHAR)
		switch (cmd = tgetkb()) {
		case 'y':
		case 'Y':
			rc = YES;
			break;
		case 'N':
		case 'n':
			rc = NO;
			break;
		case '!':
			if (allow_bang)
				rc = BANG;
			else
				tbell();
			break;
		default:
			tbell();
			if (Keys[cmd] == ZABORT)
				rc = ABORT;
		}
	clrpaw();
	return rc;
}

/* ask Yes/No question.
 * Returns YES, NO, or ABORT
 */
int ask(const char *msg)
{
	return ask2(msg, false);
}

/* Delay before displaying a prompt and wait for a cmd */
int delayprompt(const char *msg)
{
	int cmd, rc = tdelay(600);
	if (rc)
		putpaw(msg);
	cmd = tgetkb();
	if (rc)
		clrpaw();
	return cmd;
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

char PawStr[COLMAX + 10];

static void _putpaw(const char *str)
{
	int trow = Prow, tcol = Pcol;

	if (InPaw)
		return;

	tsetpoint(Rowmax - 1, 0);
	tprntstr(str);
	tcleol();
	tsetpoint(trow, tcol);
	tforce();
	tflush();
}

/* Put a string into the PAW. */
void putpaw(const char *fmt, ...)
{
	char str[STRMAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	_putpaw(str);
}

void error(const char *fmt, ...)
{
	char str[STRMAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	tbell();
	_putpaw(str);
}

/* Get the word at the current buffer point and store in 'word'.
 * Get at the most 'max' characters.
 * Leaves the point alone.
 */
int getbword(char word[], int max, int (*valid)(int))
{
	int i;
	struct mark tmark;

	bmrktopnt(Bbuff, &tmark);
	if (!bistoken(Buff()))
		moveto(bistoken, BACKWARD);
	movepast(bistoken, BACKWARD);
	for (i = 0; !bisend(Bbuff) && valid(Buff()) && i < max; ++i, bmove1(Bbuff))
		word[i] = Buff();
	word[i] = '\0';
	bpnttomrk(Bbuff, &tmark);
	return i;
}

/* Get the current buffer text and store in 'txt'.
 * Get at the most 'max' characters.
 * Leaves the point alone.
 */
char *getbtxt(char txt[], int max)
{
	int i;
	struct mark tmark;

	bmrktopnt(Bbuff, &tmark);
	for (btostart(Bbuff), i = 0; !bisend(Bbuff) && i < max; bmove1(Bbuff), ++i)
		txt[i] = Buff();
	txt[i] = '\0';
	bpnttomrk(Bbuff, &tmark);
	return txt;
}

int bisword(int c)
{
	return  isalnum(c) || c == '_' || c == '.' || c == '-';
}

int bistoken(int c)
{
	return bisword(c) || c == '/';
}

int biswhite(int c)
{
	return c == ' ' || c == '\t';
}

/* Put in the right number of tabs and spaces */
void tindent(int arg)
{
	if (VAR(VSPACETAB) == 0)
		for (; arg >= Tabsize; arg -= Tabsize)
			binsert(Bbuff, '\t');
	while (arg-- > 0)
		binsert(Bbuff, ' ');
}

/* Limit a filename to at most Colmax - 'num' cols */
char *limit(char *fname, int num)
{
	int off = strlen(fname) - (Colmax - num);
	return off > 0 ? fname + off : fname;
}

/* Return a pointer to the start of the last part of fname */
char *lastpart(char *fname)
{
	char *p = strrchr(fname, '/');
	if (p)
		return p + 1;
	else
		return fname;
}

void Zstats(void)
{
	int n = snprintf(PawStr, Colmax, "Buffers: %u  Pages: %u  Marks: %u",
					 NumBuffs, NumPages, NumMarks);
#if UNDO
	n += snprintf(PawStr + n, Colmax - n, "  Undos: %lu%c",
			  (undo_total + 521) / 1024, undo_total ? 'K' : ' ');
#endif
	_putpaw(PawStr);
}
