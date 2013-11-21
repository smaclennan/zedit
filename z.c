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
char *Cwd;
unsigned Cmd;
jmp_buf	zenv;

static char *ConfigDir;
static char dbgfname[PATHMAX];

#ifndef CONFIGDIR
#define CONFIGDIR "/usr/share/zedit"
#endif


static void usage(char *prog)
{
	printf(
		"usage: %s [-ht] [-c config_dir] [-l line] [fname ...]\n"
		"where:\t-h  displays this message.\n"
		"\t-t  default to text mode.\n"
		"\t-c  specifies a config dir.\n"
		"\t-l  goto specified line number. (First file only)\n"
		, prog);

	exit(1);
}

int main(int argc, char **argv)
{
	char path[PATHMAX + 1];
	int arg, files = 0, textMode = 0, exitflag = 0;
	struct buff *tbuff = NULL;

	/* A longjmp is called if bcremrk runs out of memory */
	if (setjmp(zenv) != 0) {
		error("FATAL ERROR: Out of memory");
		Argp = false;	/* so Zexit will not default to save */
		Zexit();
	}

	Home = getenv("HOME");
	if (!Home) {
		puts("You don't exist!");
		exit(1);
	}

	snprintf(dbgfname, sizeof(dbgfname), "%s/z.out", Home);
	unlink(dbgfname);

	Cwd = getcwd(NULL, PATHMAX);
	if (!Cwd) {
		puts("Unable to get CWD");
		exit(1);
	}

	while ((arg = getopt(argc, argv, "c:hl:tE")) != EOF)
		switch (arg) {
		case 'c':
			ConfigDir = optarg;
			break;
		case 'l':
			Arg = atoi(optarg);
			Argp = true;
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

	/* Deal with ConfigDir */
	if (!ConfigDir) {
		ConfigDir = getenv("ZPATH");
		if (!ConfigDir)
			ConfigDir = CONFIGDIR;
	}

	readvfile();		/* Do this BEFORE tinit */

	/* User wants Text mode as default */
	if (textMode)
		VAR(VNORMAL) = 0;

	initscrnmarks(); /* init the screen marks and mark list */

	/* create the needed buffers */
	Killbuff = bcreate();
	Paw = bcreate();
	if (!cmakebuff(MAINBUFF, NULL)) {
		puts("Not enough memory.");
		exit(1);
	}
	Paw->bname = PAWBUFNAME;

	tinit();

	REstart	= bcremrk();
	Sstart	= bcremrk();
	Psstart	= bcremrk();
	Send	= bcremrk();
	Sendp	= false;

	for (; optind < argc; ++optind, ++files)
		if (pathfixup(path, argv[optind]) == 0)
			if (findfile(path) && !tbuff)
				tbuff = Curbuff;

	if (tbuff) {
		bswitchto(tbuff);

		strcpy(Lbufname,
		       Curbuff->prev ? Curbuff->prev->bname : MAINBUFF);

		clrpaw();
	}

	winit();

	reframe();

	if (!Curbuff->mtime && Curbuff->fname)
		putpaw("New File");

	/* Do this after tinit */
	if (findpath(path, ZBFILE))
		bindfile(path, READ_MODE);
	else
		bind();

	Curwdo->modeflags = INVALID;

	if (Argp)
		Zgoto_line();

	Initializing = false;

	if (exitflag)
		Zexit();

	while (1)
		execute();
}

/* Support functions */

void Dbg(char *fmt, ...)
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

static bool isfile(char *path, char *dir, char *fname)
{
	if (!dir || !fname)
		return false;
	strcpy(path, dir);
	if (*(path + strlen(path) - 1) != '/')
		strcat(path, "/");
	strcat(path, fname);
	return access(path, 0) == 0;
}

/* Find the correct path for the config files.
 * We check HOME and then CONFIGDIR.
 */
int findpath(char *p, char *f)
{
	if (isfile(p, Home, f))
		return 2;
	else if (isfile(p, ConfigDir, f))
		return 1;
	else
		return 0;
}

/* ask Yes/No question.
 * Returns YES, NO, BANG, or ABORT
 */
int ask2(char *msg, bool allow_bang)
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
int ask(char *msg)
{
	return ask2(msg, false);
}

bool delayprompt(char *msg)
{
	int rc = delay(500);
	if (rc)
		putpaw(msg);
	return rc;
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

/* Put a string into the PAW. */
void putpaw(const char *fmt, ...)
{
	int trow, tcol;
	char str[STRMAX];
	va_list ap;

	if (InPaw)
		return;

	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	trow = Prow; tcol = Pcol;
	tsetpoint(tmaxrow() - 1, 0);
	tprntstr(str);
	tcleol();
	tsetpoint(trow, tcol);
	tforce();
	tflush();
}

/* echo 'str' to the paw and as the filename for 'buff' */
void message(struct buff *buff, char *str)
{
	struct wdo *wdo;

	if (buff->fname)
		free(buff->fname);
	buff->fname = strdup(str);
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo->wbuff == buff)
			wdo->modeflags = INVALID;
	putpaw("%s", str);
}

/* Get the word at the current buffer point and store in 'word'.
 *  Get at the most 'max' characters.
 * Leaves the point alone.
 */
bool getbword(char word[], int max, int (*valid)())
{
	int i;
	struct mark tmark;

	bmrktopnt(&tmark);
	moveto(bistoken, FORWARD);
	if (bisend())
		moveto(bistoken, BACKWARD);
	movepast(bistoken, BACKWARD);
	for (i = 0; !bisend() && valid() && i < max; ++i, bmove1())
		word[i] = Buff();
	word[i] = '\0';
	bpnttomrk(&tmark);
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

	bmrktopnt(&tmark);
	for (btostart(), i = 0; !bisend() && i < max; bmove1(), ++i)
		txt[i] = Buff();
	txt[i] = '\0';
	bpnttomrk(&tmark);
	return txt;
}

/* Go forward or back past a thingy */
void movepast(int (*pred)(), bool forward)
{
	if (!forward)
		bmove(-1);
	while (!(forward ? bisend() : bisstart()) && (*pred)())
		bmove(forward ? 1 : -1);
	if (!forward && !(*pred)())
		bmove1();
}

/* Go forward or back to a thingy */
void moveto(int (*pred)(), bool forward)
{
	if (!forward)
		bmove(-1);
	while (!(forward ? bisend() : bisstart()) && !(*pred)())
		bmove(forward ? 1 : -1);
	if (!forward && !bisstart())
		bmove1();
}

/* Put in the right number of tabs and spaces */
void tindent(int arg)
{
	if (VAR(VSPACETAB) == 0)
		for (; arg >= Tabsize; arg -= Tabsize)
			binsert('\t');
	while (arg-- > 0)
		binsert(' ');
}

int bisspace(void)
{
	return isspace(Buff());
}

int bisword(void)
{
	return  isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
		Buff() == '$';
}

/* Must be a real function. $ for PL/M */
int bistoken(void)
{
	return isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
		Buff() == '$' || Buff() == '/';
}

int biswhite(void)
{
	return STRIP(Buff()) == ' ' || STRIP(Buff()) == '\t';
}

/* Limit a filename to at most tmaxcol() - 'num' cols */
char *limit(char *fname, int num)
{
	int off;

	off = strlen(fname) - (tmaxcol() - num);
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
