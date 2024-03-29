/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"
#include <stdarg.h>

/** @addtogroup zedit
 * @{
 */

int verbose;
bool Initializing = true;
char *Home;
int Homelen;
int text_mode;

unsigned int Cmd;
int Cmdpushed = -1; /* Search pushed a key */

static void _usage(void)
{
	terror("usage: zedit [-hrt] [-c config] [-l line] [fname ...]\n"
		   "where:\t-h  displays this message.\n"
		   "\t-r  do not do CR/LF conversion.\n"
		   "\t-t  default to text mode.\n"
		   "\t-l  goto specified line number. (First file only)\n");
	exit(1);
}

char PawStr[PAWSTRLEN];

static void _putpaw(const char *str)
{
	int trow = Prow, tcol = Pcol;

	if (InPaw)
		return;

	t_goto(Rowmax - 1, 0);
	tprntstr(str);
	tcleol();
	t_goto(trow, tcol);
	tflush();
}

/* Put a string into the PAW. */
void putpaw(const char *fmt, ...)
{
	char str[STRMAX];
	va_list ap;

	va_start(ap, fmt);
	strfmt_ap(str, sizeof(str), fmt, ap);
	va_end(ap);

	_putpaw(str);
}

void error(const char *fmt, ...)
{
	char str[STRMAX];
	va_list ap;

	va_start(ap, fmt);
	strfmt_ap(str, sizeof(str), fmt, ap);
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

static struct pollfd fds[] = {
	{ .fd =  0, .events = POLLIN },
	{ .fd = -1, .events = POLLIN }
};
#define MAX_FDS (sizeof(fds) / sizeof(struct pollfd))

void set_pipefd(int fd)
{
	fds[1].fd = fd;
}
#endif

void execute(void)
{
	zrefresh();

#if HUGE_FILES && HUGE_THREADED
	extern void check_events(void);
	check_events();
#endif

#ifdef DOPIPES
	if (fds[1].fd == -1 || cpushed || Cmdpushed != -1)
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
		if (fds[1].revents)
			readapipe();
	}
#else
	dotty();
#endif
}

int main(int argc, char **argv)
{
	char path[PATHMAX + 1];
	int arg, line = 0;
	struct mark tmark;
	struct zbuff *tbuff = NULL;

	Home = getenv("HOME");
	if (!Home)
		Home = "/";
	Homelen = strlen(Home);

	strconcat(path, sizeof(path), Home, "/z.out", NULL);
	unlink(path);
	Dbgfname(path);

	while ((arg = getopt(argc, argv, "c:hl:rtv")) != EOF)
		switch (arg) {
		case 'c':
			readvfile(optarg);
			break;
		case 'l':
			line = atoi(optarg);
			break;
		case 'r':
			raw_mode = 1;
			break;
		case 't':
			text_mode = 1;
			break;
		case 'v':
			++verbose;
			break;
		case 'h':
		default:
			_usage();
		}

	binit();

	/* PAW must not be on the Bufflist */
	Paw = cmakebuff("*paw*", NULL);
	if (!Paw)
		exit(1);
	Paw->prev = Paw->next = NULL;
	Bufflist = NULL;
	Curbuff = NULL;

	if (!cmakebuff(MAINBUFF, NULL)) {
		terror("Not enough memory.\n");
		exit(1);
	}

	bmrktopnt(Bbuff, &tmark);
	display_init(&tmark);

	tinit();
	tainit();

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

	while (1)
		execute();
}

void Zstats(void)
{
	struct zbuff *buff;
	struct mark *mark;
	struct page *page;
	unsigned int nbuff = 2; /* paw + kill */
	unsigned int npage = 1 + delpages(); /* paw page + kill */
	unsigned int nmarks = 0;

	foreachbuff(buff) {
		++nbuff;
		for (page = buff->buff->firstp; page; page = page->nextp)
			++npage;
		foreach_buffmark(buff->buff, mark)
			++nmarks;
	}

	putpaw("Buffers: %u  Pages: %u  Marks: %u", nbuff, npage, nmarks);
}
/* @} */
