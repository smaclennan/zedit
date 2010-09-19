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

Boolean Exitflag = TRUE;	/* set to true during initialization */
int ExitStatus;
char *Thispath, *Cwd;
char *ConfigDir;
int Cmask;
unsigned Cmd;
jmp_buf	zenv;
int Verbose;

char **Bnames;				/* array of ptrs to buffer names */
int Numbuffs;			/* number of buffers */

#include <sys/stat.h>
struct passwd *Me;
char *Shell;

int main(int argc, char **argv)
{
	/* A longjmp is called if Bcremrk or Getmemp run out of memory */
	if (setjmp(zenv) != 0) {
		Error("FATAL ERROR: Out of memory");
		Argp = FALSE;	/* so Zexit will not default to save */
		Zexit();
		Tfini();
		exit(1);
	}

#ifdef MEMLOG
	loginit("/tmp/malloc");
#endif

	Setup(argc, argv);
	Edit();
	Tfini();

#ifdef MEMLOG
	logfini();
#endif
	exit(ExitStatus);
}


void Edit()
{
	Exitflag = FALSE;
	while (!Exitflag)
		Execute();
}


void Execute()
{
#if defined(PIPESH) && !defined(XWINDOWS)
	fd_set fds = SelectFDs;

	Refresh();

	if (cpushed)
		Dotty();
	else {
		/* select returns -1 if a child dies (SIGPIPE) -
		 * Sigchild handles it */
		while (select(NumFDs, &fds, 0, 0, 0) == -1) {
#ifdef SYSV2
			Checkpipes(1);
			Refresh();
#endif
			fds = SelectFDs;
		}
		Readpipes(&fds);
		if (FD_ISSET(1, &fds))
			Dotty();
	}
#else
	Refresh();
	Dotty();
#endif
}


/* NOTE: Dotty blocks */
void Dotty()
{
	Cmd = Tgetcmd();
	Arg = 1;
	Argp = FALSE;
	while (Arg > 0) {
		(*Funcs[Keys[Cmd]])();
		--Arg;
	}
	Lfunc = Keys[Cmd];
	First = FALSE;				/* used by Pinsert when InPaw */
}


void Setup(int argc, char **argv)
{
	char path[PATHMAX + 1];
	char *progname;
	int col = 0, arg, files = 0, textMode = 0;
	struct buff *tbuff, *other = NULL;
#ifdef XWINDOWS
	Boolean Spawn = TRUE;

	/* This MUST be called before any file IO */
	Xinit("zedit", &argc, argv);
#endif
	Me = dup_pwent(getpwuid(geteuid()));
	if (!Me) {
		Me = dup_pwent(getpwuid(getuid()));
		if (!Me) {
			puts("You don't exist!");
			exit(1);
		}
	}

	Shell = getenv("SHELL");
	if (!Shell)
		Shell = "/bin/sh";
	/* SAM - can't get tcsh to work...use csh */
	if (strcmp(Lastpart(Shell), "tcsh") == 0)
		Shell = "/bin/csh";

#if DBG
	Dbgname(AddHome(path, ZDBGFILE));
#endif

	Cmask = umask(0);		/* get the current umask */
	umask(Cmask);			/* set it back */
	Cmask = ~Cmask & 0666;	/* make it usable */

	srand(time(0));

	/* see if ZPATH set */
	Thispath = getenv("ZPATH");
	if (Thispath)
		progname = Lastpart(argv[0]);
	/* convert argv[0] to directory name */
	else {
		Thispath = argv[0];
		progname = Lastpart(Thispath);
		if (progname != Thispath)
			*(progname - 1) = '\0';
		/* use current dir */
		else
			Thispath = ".";
	}

	Cwd = getcwd(NULL, PATHMAX);
	if (!Cwd) {
		puts("Unable to get CWD");
		exit(1);
	}
	/* default for Zfindfile */
	strcpy(Fname, Cwd);
	strcat(Fname, "/");

	Colmax = EOF;
	Tstart = 0;

	/* Note: for X we cannot use -m */
	opterr = 0;
	while ((arg = getopt(argc, argv, "c:hl:o:tv")) != EOF)
		switch (arg) {
		case 'c':
			ConfigDir = optarg;
			break;
		case 'h':
			Usage(progname);
		case 'l':
			Argp = Arg = atoi(optarg);
			break;
#ifdef XWINDOWS
		case 'n':
			Spawn = FALSE;
			break;
#endif
		case 'o':
			col = atoi(optarg);
			break;
		case 't':
			textMode = 1;
			break;
		case 'v':
			++Verbose;
			break;
		}

	ReadVfile();		/* Do this BEFORE Tinit */

	/* User wants Text mode as default */
	if (textMode)
		VAR(VNORMAL) = 0;

	initScrnmarks(); /* init the screen marks and mark list */

	/* create the needed buffers */
	Killbuff = Bcreate();
	Paw = Bcreate();
	if (!Cmakebuff(MAINBUFF, NULL)) {
		puts("Not enough memory.");
		exit(1);
	}
	Paw->bname = PAWBUFNAME;
	InPaw = FALSE;

#ifdef XWINDOWS
	XAddBuffer(MAINBUFF);
	Tinit(argc, argv);
#else
	Tinit();
#endif

	REstart	= Bcremrk();
	Sstart	= Bcremrk();
	Psstart	= Bcremrk();
	Send	= Bcremrk();
	Sendp	= FALSE;

	for ( ; optind < argc; ++optind)
		if (argv[optind][0] == '-') {
			if (argv[optind][1] == 'l') {
				Argp = TRUE;
				Arg = atoi(&argv[optind][2]);
			} else if (argv[optind][1] == 'o')
				col = atoi(&argv[optind][2]);
			else if (argv[optind][1] == '\0')
				++files;
		} else {
			++files;
			if (Pathfixup(path, argv[optind]) == 0)
				if (!Findfile(path, TRUE))
					break;
		}

	if (files) {
		/* find the first buffer read or Main */
		for (tbuff = Bufflist; tbuff->next; tbuff = tbuff->next)
			;
		Bswitchto(tbuff->prev ? tbuff->prev : tbuff);

		/* if VUSEOTHER try to load two buffers */
		if (VAR(VUSEOTHER) && Curbuff->prev) {
			other = Curbuff->prev;
			strcpy(Lbufname,
			       other->prev ? other->prev->bname : MAINBUFF);
		} else
			strcpy(Lbufname,
			       Curbuff->prev ? Curbuff->prev->bname : MAINBUFF);
		Clrecho();
	} else
		Loadsaved();

	Winit();
	if (other) {
		Z2wind();
		Cswitchto(other);
		Reframe();
		Znextwind();
	}

	Reframe();
	Setmodes(Curbuff);		/* start it off right! */

	if (!Curbuff->mtime && Curbuff->fname)
		Echo("New File");

	Bind();
	Loadbind();		/* Do this after Tinit */

	Curwdo->modeflags = INVALID;

	if (Argp)
		Zlgoto();
	if (col > 1)
		Bmakecol(col - 1, FALSE);

#ifdef XWINDOWS
	if (Spawn) {
		if (fork() == 0)		/* fork off editor */
			return;
		exit(0);	/* kill parent */
	}
#elif defined(PIPESH)
	/* For xwindows this is set in initSockets */
	FD_ZERO(&SelectFDs);
	FD_SET(1, &SelectFDs);
	NumFDs = 2;
#endif
}


/* Read one file, creating the buffer is necessary.
 * Returns FALSE if unable to create buffer only.
 * FALSE means that there is no use continuing.
 */
Boolean Readone(char *bname, char *path)
{
	struct buff *was = Curbuff;

	if (Cfindbuff(bname))
		return TRUE;

	if (Cmakebuff(bname, path)) {
		int rc = Breadfile(path);
		if (rc >= 0) {
			Toggle_mode(0);
			if (rc > 0)
				Echo("New File");
			else if (access(path, R_OK|W_OK) == EOF)
				Curbuff->bmode |= VIEW;
			strcpy(Lbufname, was->bname);
#ifdef XWINDOWS
			XAddBuffer(bname);
#endif
		} else { /* error */
			Delbname(Curbuff->bname);
			Bdelbuff(Curbuff);
			Bswitchto(was);
		}
		return TRUE;
	}
	return FALSE;
}

/* Create a buffer. */
struct buff *Cmakebuff(char *bname, char *fname)
{
	struct buff *bptr, *save = Curbuff;

	bptr = Cfindbuff(bname);
	if (bptr) {
		Bswitchto(bptr);
		return bptr;
	}

	bptr = Bcreate();
	if (!bptr) {
		Error("Unable to create buffer");
		return NULL;
	}

	bptr->bname = Addbname(bname);
	if (!bptr->bname) {
		Error("Out of buffers");
		Bdelbuff(bptr);
		Bswitchto(save);
		return NULL;
	}

	if (*bname == '*')
		bptr->bmode |= SYSBUFF;

	Bswitchto(bptr);
	if (fname)
		bptr->fname = strdup(fname);
	/* add the buffer to the head of the list */
	if (Bufflist)
		Bufflist->prev = bptr;
	bptr->next = Bufflist;
	Bufflist = bptr;

	return bptr;
}


/* Add the new bname to the Bname array.
 * If we hit maxbuffs, try to enlarge the Bnames array.
 * Note that the compare MUST be insensitive for the Getplete!
 */
char *Addbname(char *bname)
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

	for (i = Numbuffs; i > 0 && Stricmp(bname, Bnames[i - 1]) < 0; --i)
		Bnames[i] = Bnames[i - 1];
	Bnames[i] = strdup(bname);
	if (strlen(Bnames[i]) > BUFNAMMAX)
		Bnames[i][BUFNAMMAX] = '\0';
	++Numbuffs;

	return Bnames[i];
}

Boolean Delbname(char *bname)
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
struct buff *Cfindbuff(char *bname)
{
	struct buff *tbuff;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (Strnicmp(tbuff->bname, bname, BUFNAMMAX) == 0)
			return tbuff;
	return NULL;
}

/* Return a pointer to the start of the last part of fname */
char *Lastpart(char *fname)
{
	char *sp;

	for (sp = fname + strlen(fname) - 1; sp >= fname && *sp != '/'; --sp)
		;
	return ++sp;
}

void Usage(prog)
char *prog;
{
	printf(
#ifdef XWINDOWS
		"usage: %s [-hnt] [-c config_dir] [fname ... [-l#] [-o#]]\n"
#else
		"usage: %s [-ht] [-c config_dir] [fname ... [-l#] [-o#]]\n"
#endif
		"where:\t-h  displays this message.\n"
#ifdef XWINDOWS
		"\t-n  do not spawn window.\n"
#endif
		"\t-t  default to text mode.\n"
		"\t-c  specifies a config dir.\n"
		"\t-l  goto specified line number.\n"
		"\t    this only applies to the first file.\n"
		"\t-o  goto specified offset in line.\n"
		"\t    this only applies to the first file.\n",
		prog);

	exit(1);
}

void Zcwd()
{
	char path[PATHMAX], *p;

	strcpy(path, Cwd);
	if (Getdname("CWD: ", path) == 0) {
		p = strdup(path);
		if (!p)
			Error("Not enough memory");
		else if (chdir(p) == 0) {
			Cwd = p;
#ifdef XWINDOWS
			if (VAR(VSHOWCWD))
				Newtitle(Cwd);
#endif
		} else
			Error("chdir failed.");
	}
}

/* Create a file relative to home.
 * On non-unix we just make it current.
 */
char *AddHome(char *path, char *fname)
{
	sprintf(path, "%s/%s", Me->pw_dir, fname);
	return path;
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
