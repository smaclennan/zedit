/* z.c - Zedit mainline
 * Copyright (C) 1988-2018 Sean MacLennan
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

char PawStr[PAWSTRLEN];

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
	zbuff_t *tbuff = NULL;

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

	os_init();

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

		set_last_bufname(Curbuff->prev ? Curbuff->prev : Curbuff);
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

void Zstats(void)
{
	zbuff_t *buff;
	struct mark *mark;
	struct page *page;
	unsigned nbuff = 2; /* paw + kill */
	unsigned npage = 1 + delpages(); /* paw page + kill */
	unsigned nmarks = 0;

	foreachbuff(buff) {
		++nbuff;
		for (page = buff->buff->firstp; page; page = page->nextp)
			++npage;
		foreach_buffmark(buff->buff, mark)
			++nmarks;
	}

	snprintf(PawStr, Colmax, "Buffers: %u  Pages: %u  Marks: %u",
			 nbuff, npage, nmarks);
	_putpaw(PawStr);
}
