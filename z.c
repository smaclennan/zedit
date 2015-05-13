/* z.c - Zedit mainline
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
#include <stdarg.h>

bool Initializing = true;
char *Home;
jmp_buf	zenv;

unsigned Cmd;
int Cmdpushed = -1; /* Search pushed a key */

static char dbgfname[PATHMAX];

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
		Cmd = tgetcmd();
	else {
		Cmd = Cmdpushed;
		Cmdpushed = -1;
	}
	if (Cmd == TC_MOUSE) return;

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

#if DOPIPES
#include <poll.h>

#define MAX_FDS 3
#define PIPEFD 2

static struct pollfd fds[MAX_FDS];

static void fd_init(void)
{
	int i;
	for (i = 0; i < MAX_FDS; ++i) {
		fds[i].fd = -1;
		fds[i].events = POLLIN;
	}

	fds[0].fd = 0; /* stdin */
#if GPM_MOUSE
	extern int gpm_fd;
	fds[1].fd = gpm_fd;
#endif
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
	
#if DOPIPES
#if GPM_MOUSE
	handle_mouse_cursor();
#endif

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
#if GPM_MOUSE
		if (fds[1].revents)
			handle_gpm_mouse();
#endif
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
	int arg, files = 0, textMode = 0, exitflag = 0, line = 0;
	struct zbuff *tbuff = NULL;

	/* A longjmp is called if zcreatemrk runs out of memory */
	if (setjmp(zenv) != 0) {
		error("FATAL ERROR: Out of memory");
		Argp = false;	/* so Zexit will not default to save */
		Zexit();
	}

	Home = gethomedir();
	if (!Home) {
		puts("You don't exist!");
		exit(1);
	}

	snprintf(dbgfname, sizeof(dbgfname), "%s/z.out", Home);
	unlink(dbgfname);

	while ((arg = getopt(argc, argv, "hl:rtE")) != EOF)
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
		case 'h':
		default:
			usage(argv[0]);
		}

	/* Do this BEFORE tinit */
	findpath(path, ZCFILE, readvfile);

	/* User wants Text mode as default */
	if (textMode)
		VAR(VNORMAL) = 0;

	display_init();

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

	tinit();
	fd_init();

	for (; optind < argc; ++optind, ++files)
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

void Dbg(const char *fmt, ...)
{
	FILE *fp = fopen(dbgfname, "a");
	if (fp) {
		va_list arg_ptr;

		va_start(arg_ptr, fmt);
		vfprintf(fp, fmt, arg_ptr);
		va_end(arg_ptr);
		fclose(fp);
	}
}

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
		switch (cmd = tgetcmd()) {
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
	cmd = tgetcmd();
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

void _putpaw(const char *str)
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
int getbword(char word[], int max, int (*valid)())
{
	int i;
	struct mark tmark;

	bmrktopnt(Bbuff, &tmark);
	if (!bistoken())
		moveto(bistoken, BACKWARD);
	movepast(bistoken, BACKWARD);
	for (i = 0; !bisend(Bbuff) && valid() && i < max; ++i, bmove1(Bbuff))
		word[i] = Buff();
	word[i] = '\0';
	bpnttomrk(Bbuff, &tmark);
	return i;
}

/* Kinda like the emacs looking-at command. Does not move the point. */
bool looking_at(const char *match)
{
	struct mark tmark;

	bmrktopnt(Bbuff, &tmark);
	while (*match && *Curcptr == *match) {
		bmove1(Bbuff);
		++match;
	}
	bpnttomrk(Bbuff, &tmark);
	return *match == '\0';
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

/* Go forward or back past a thingy */
void movepast(int (*pred)(), bool forward)
{
	if (!forward)
		bmove(Bbuff, -1);
	while (!(forward ? bisend(Bbuff) : bisstart(Bbuff)) && (*pred)())
		bmove(Bbuff, forward ? 1 : -1);
	if (!forward && !(*pred)())
		bmove1(Bbuff);
}

/* Go forward or back to a thingy */
void moveto(int (*pred)(), bool forward)
{
	if (!forward)
		bmove(Bbuff, -1);
	while (!(forward ? bisend(Bbuff) : bisstart(Bbuff)) && !(*pred)())
		bmove(Bbuff, forward ? 1 : -1);
	if (!forward && !bisstart(Bbuff))
		bmove1(Bbuff);
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

int bisspace(void)
{
	return isspace(Buff());
}

int bisword(void)
{
	return  isalnum(Buff()) || Buff() == '_' || Buff() == '.';
}

/* Must be a real function. */
int bistoken(void)
{
	return bisword() || Buff() == '/';
}

int biswhite(void)
{
	return Buff() == ' ' || Buff() == '\t';
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

/* Keep around one mark */
struct mark *freemark;

struct mark *zcreatemrk(void)
{
	struct mark *mrk = freemark;

	if (mrk) {
		freemark = NULL;
		bmrktopnt(Bbuff, mrk);
	} else if (!(mrk = bcremrk(Bbuff)))
		longjmp(zenv, -1);	/* ABORT */

	return mrk;
}

void unmark(struct mark *mrk)
{
	if (freemark)
		bdelmark(mrk);
	else
		freemark = mrk;
}
