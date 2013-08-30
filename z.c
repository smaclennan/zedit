/* z.c - Zedit mainline
 * Copyright (C) 1988-2010 Sean MacLennan
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

Boolean Initializing = TRUE;
char *Cwd;
char *ConfigDir;
int Cmask;
unsigned Cmd;
jmp_buf	zenv;
int Verbose;

char **Bnames;			/* array of ptrs to buffer names */
int Numbuffs;			/* number of buffers */

struct passwd *Me;

static void setup(int, char **);
static void usage(char *prog);

int main(int argc, char **argv)
{
	/* A longjmp is called if bcremrk runs out of memory */
	if (setjmp(zenv) != 0) {
		error("FATAL ERROR: Out of memory");
		Argp = FALSE;	/* so Zexit will not default to save */
		Zexit();
	}

	setup(argc, argv);

	while (1)
		execute();
}

/* NOTE: Dotty blocks */
static void dotty(void)
{
	Cmd = tgetcmd();
	Arg = 1;
	Argp = FALSE;
	while (Arg > 0) {
		CMD(Keys[Cmd]);
		--Arg;
	}
	Lfunc = Keys[Cmd];
	First = FALSE;				/* used by pinsert when InPaw */
}


void execute(void)
{
#ifdef PIPESH
	fd_set fds = SelectFDs;

	refresh();

	if (cpushed)
		dotty();
	else {
		/* select returns -1 if a child dies (SIGPIPE) -
		 * sigchild handles it */
		while (select(NumFDs, &fds, NULL, NULL, NULL) == -1) {
#ifdef SYSV4
			checkpipes(1);
			refresh();
#endif
			fds = SelectFDs;
		}
		readpipes(&fds);
		if (FD_ISSET(1, &fds))
			dotty();
	}
#else
	refresh();
	dotty();
#endif
}

static void setup(int argc, char **argv)
{
	char path[PATHMAX + 1];
	int arg, files = 0, textMode = 0, exitflag = 0;
	struct buff *tbuff = NULL;

	Me = dup_pwent(getpwuid(geteuid()));
	if (!Me) {
		Me = dup_pwent(getpwuid(getuid()));
		if (!Me) {
			puts("You don't exist!");
			exit(1);
		}
	}
	Dbgname();

	Cmask = umask(0);	/* get the current umask */
	umask(Cmask);		/* set it back */
	Cmask = ~Cmask & 0666;	/* make it usable */

	Cwd = getcwd(NULL, PATHMAX);
	if (!Cwd) {
		puts("Unable to get CWD");
		exit(1);
	}

	Colmax = EOF;
	Tstart = 0;

	while ((arg = getopt(argc, argv, "c:hl:tvE")) != EOF)
		switch (arg) {
		case 'c':
			ConfigDir = optarg;
			break;
		case 'h':
			usage(argv[0]);
		case 'l':
			Argp = Arg = atoi(optarg);
			break;
		case 't':
			textMode = 1;
			break;
		case 'v':
			++Verbose;
			break;
		case 'E':
			exitflag = TRUE;
			break;
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
	InPaw = FALSE;

	tinit();

	REstart	= bcremrk();
	Sstart	= bcremrk();
	Psstart	= bcremrk();
	Send	= bcremrk();
	Sendp	= FALSE;

	for ( ; optind < argc; ++optind, ++files)
		if (pathfixup(path, argv[optind]) == 0)
			if (findfile(path) && !tbuff)
				tbuff = Curbuff;

	if (tbuff) {
		bswitchto(tbuff);

		strcpy(Lbufname,
		       Curbuff->prev ? Curbuff->prev->bname : MAINBUFF);

		clrecho();
	}

	winit();

	reframe();
	setmodes(Curbuff);		/* start it off right! */

	if (!Curbuff->mtime && Curbuff->fname)
		echo("New File");

	bind();
	loadbind();		/* Do this after tinit */

	Curwdo->modeflags = INVALID;

	if (Argp)
		Zlgoto();

#ifdef PIPESH
	FD_ZERO(&SelectFDs);
	FD_SET(1, &SelectFDs);
	NumFDs = 2;
#endif

	Initializing = FALSE;

	if (exitflag)
		Zexit();
}

/* Add the new bname to the Bname array.
 * If we hit maxbuffs, try to enlarge the Bnames array.
 * Note that the compare MUST be insensitive for the getplete!
 */
static char *addbname(char *bname)
{
	static int maxbuffs; /* max buffers Bnames can hold */
	int i;

	if (Numbuffs == maxbuffs) {
		/* increase Bnames array */
		char **ptr = realloc(Bnames, (maxbuffs + 10) * sizeof(char *));
		if (!ptr)
			return NULL;

		Bnames = ptr;
		maxbuffs += 10;
	}

	for (i = Numbuffs; i > 0 && strcasecmp(bname, Bnames[i - 1]) < 0; --i)
		Bnames[i] = Bnames[i - 1];
	Bnames[i] = strdup(bname);
	if (strlen(Bnames[i]) > BUFNAMMAX)
		Bnames[i][BUFNAMMAX] = '\0';
	++Numbuffs;

	return Bnames[i];
}

/* Create a buffer. */
struct buff *cmakebuff(char *bname, char *fname)
{
	struct buff *bptr, *save = Curbuff;

	bptr = cfindbuff(bname);
	if (bptr) {
		bswitchto(bptr);
		return bptr;
	}

	bptr = bcreate();
	if (!bptr) {
		error("Unable to create buffer");
		return NULL;
	}

	bptr->bname = addbname(bname);
	if (!bptr->bname) {
		error("Out of buffers");
		bdelbuff(bptr);
		bswitchto(save);
		return NULL;
	}

	if (*bname == '*')
		bptr->bmode |= SYSBUFF;

	bswitchto(bptr);
	if (fname)
		bptr->fname = strdup(fname);
	/* add the buffer to the head of the list */
	if (Bufflist)
		Bufflist->prev = bptr;
	bptr->next = Bufflist;
	Bufflist = bptr;

	return bptr;
}

Boolean delbname(char *bname)
{
	int i, rc;

	if (Numbuffs == 0)
		return FALSE;
	for (i = rc = 0; i <= Numbuffs && (rc = strcmp(bname, Bnames[i])); ++i)
		;
	if (rc == 0)
		for (--Numbuffs; i <= Numbuffs; ++i)
			Bnames[i] = Bnames[i + 1];

	return TRUE;
}

/* Locate a given buffer */
struct buff *cfindbuff(char *bname)
{
	struct buff *tbuff;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (strncasecmp(tbuff->bname, bname, BUFNAMMAX) == 0)
			return tbuff;
	return NULL;
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

void Zcwd(void)
{
	char path[PATHMAX], *p;

	strcpy(path, Cwd);
	if (getdname("CWD: ", path) == 0) {
		p = strdup(path);
		if (!p)
			error("Not enough memory");
		else if (chdir(p) == 0) {
			free(Cwd);
			Cwd = p;
		} else
			error("chdir failed.");
	}
}

/* Dup a passwd entry. Only saves: pw_name, pw_dir, pw_uid, pw_gid */
struct passwd *dup_pwent(struct passwd *pw)
{
	struct passwd *new;

	if (pw == NULL)
		return NULL;

	new = malloc(sizeof(struct passwd));
	if (new) {
		memset(new, '\0', sizeof(struct passwd));
		new->pw_name = strdup(pw->pw_name);
		new->pw_dir = strdup(pw->pw_dir);
		new->pw_uid = pw->pw_uid;
		new->pw_gid = pw->pw_gid;

		if (!new->pw_name || !new->pw_dir) {
			if (new->pw_name)
				free(new->pw_name);
			free(new);
			new = NULL;
		}
	}

	return new;
}

void free_pwent(struct passwd *pw)
{
	free(pw->pw_name);
	free(pw->pw_dir);
	free(pw);
}
