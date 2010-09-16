/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include <setjmp.h>

Boolean Exitflag = TRUE;	/* set to true during initialization */
int ExitStatus = 0;
char *Thispath, *Cwd;
char *ConfigDir = 0;
int Cmask;
unsigned Cmd;
jmp_buf	zenv;
int Verbose = 0;

char **Bnames;				/* array of ptrs to buffer names */
int Numbuffs = 0;			/* number of buffers */

#include <sys/stat.h>
struct passwd *Me;
char *Shell;

int main(int argc, char **argv)
{
	/* A longjmp is called if Bcremrk or Getmemp run out of memory */
	if( setjmp(zenv) != 0 )
	{
		Error( "FATAL ERROR: Out of memory" );
		Argp = FALSE;	/* so Zexit will not default to save */
		Zexit();
		Tfini();
		exit( 1 );
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
	while(!Exitflag)
		Execute();
}


void Execute()
{
#if PIPESH && !XWINDOWS
	extern int cpushed;
	extern fd_set SelectFDs;
	extern int NumFDs;
	fd_set fds = SelectFDs;

	Refresh();

	if(cpushed)
		Dotty();
	else
	{
		/* select returns -1 if a child dies (SIGPIPE) - Sigchild handles it */
		while(select(NumFDs, &fds, 0, 0, 0) == -1)
		{
#if SYSV2
			Checkpipes(1);
			Refresh();
#endif
			fds = SelectFDs;
		}
		Readpipes(&fds);
/* SAM 		Checkpipes(1); */
		if(FD_ISSET(1, &fds)) Dotty();
	}
#else
	Refresh();
	Dotty();
#endif
}


/* NOTE: Dotty blocks */
Proc Dotty()
{
	extern Proc (**Funcs)();
	extern Byte Keys[], Lfunc;
	extern Boolean First;

	Cmd = Tgetcmd();
	Arg = 1;
	Argp = FALSE;
	while( Arg > 0 )
	{
		(*Funcs[Keys[Cmd]])();
		--Arg;
	}
	Lfunc = Keys[ Cmd ];
	First = FALSE;				/* used by Pinsert when InPaw */
}


void Setup(argc, argv)
int argc;
char **argv;
{
	extern char *getcwd(), *getenv();
	extern struct passwd *getpwuid();
	extern char *optarg;
	extern int optind, opterr;
	extern Boolean Sendp;
	extern Buffer *Killbuff;
	extern char Lbufname[];
	extern struct avar Vars[];
	extern Buffer *Bufflist, *Paw;
	extern Mark *Sstart, *Psstart, *Send, *REstart;
#ifdef PIPESH
	extern fd_set SelectFDs;
	extern int NumFDs;
#endif
	char path[ PATHMAX + 1 ];
	char *progname;
	int col = 0, arg, files = 0, textMode = 0;
	Buffer *tbuff, *other = NULL;
	Boolean xor = FALSE;
#if XWINDOWS
	Boolean Spawn = TRUE;

	/* This MUST be called before any file IO */
	Xinit("zedit", &argc, argv);
#endif

	if((Me = dup_pwent(getpwuid(geteuid()))) == NULL &&
	   (Me = dup_pwent(getpwuid(getuid ()))) == NULL)
	{
		puts("You don't exist!");
		exit(1);
	}

	if((Shell = getenv("SHELL")) == NULL) Shell = "/bin/sh";
	/* SAM - can't get tcsh to work...use csh */
	if(strcmp(Lastpart(Shell), "tcsh") == 0) Shell = "/bin/csh";

#if DBG
	Dbgname(AddHome(path, ZDBGFILE));
#endif

	Cmask = umask(0);		/* get the current umask */
	umask(Cmask);			/* set it back */
	Cmask = ~Cmask & 0666;	/* make it usable */

	srand(time(0));

	/* see if ZPATH set */
	if((Thispath = getenv("ZPATH")) != NULL)
		progname = Lastpart(argv[0]);
	/* convert argv[0] to directory name */
	else if((progname = Lastpart(Thispath = argv[0])) != Thispath)
		*progname++ = '\0';
	/* use current dir */
	else
		Thispath = ".";

	if((Cwd = getcwd(NULL, PATHMAX)) == NULL)
	{
		puts("Unable to get CWD");
		exit(1);
	}

	Colmax = EOF;
	Tstart = 0;

	/* Note: for X we cannot use -m */
	opterr = 0;
	while((arg = getopt(argc, argv, "c:hl:o:tvx")) != EOF)
		switch(arg)
		{
			case 'c': ConfigDir = optarg;			break;
			case 'h': Usage(progname);
			case 'l': Argp = Arg = atoi(optarg);	break;
#if XWINDOWS
			case 'n': Spawn = FALSE;				break;
#endif
			case 'o': col = atoi(optarg);			break;
			case 't': textMode = 1;					break;
			case 'v': ++Verbose;					break;
			case 'x': xor  = TRUE;					break;
		}
	ReadVfile();		/* Do this BEFORE Tinit */

	/* User wants Text mode as default */
	if(textMode) Vars[VNORMAL].val = 0;

	/* Command line overrides config.z */
	if(xor) Vars[VXORCURSOR].val = 1;

	Binit();

	/* create the needed buffers */
	Killbuff = Bcreate();
	Paw = Bcreate();				/* create the Prompt Argument Window */
	if(!Cmakebuff(MAINBUFF, NULL))
	{
		puts( "Not enough memory." );
		exit( 1 );
	}
	Paw->bname = PAWBUFNAME;
	InPaw = FALSE;

#if XWINDOWS
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

	for( ; optind < argc; ++optind)
		if(argv[optind][0] == '-')
		{
			if(argv[optind][1] == 'l')
			{
				Argp = TRUE;
				Arg = atoi( &argv[optind][2]);
			}
			else if(argv[optind][1] == 'o')
				col = atoi( &argv[optind][2]);
			else if(argv[optind][1] == '\0')
				++files;
		}
		else
		{
			++files;
			if(Pathfixup(path, argv[optind]) == 0)
#if 0
			if(!ReadFile(Lastpart(argv[optind]), path))
				break;
#else
			if(!Findfile(path, TRUE))
				break;
#endif
		}

	if(files)
	{	/* find the first buffer read or Main */
		for(tbuff = Bufflist; tbuff->next; tbuff = tbuff->next) ;
		Bswitchto(tbuff->prev ? tbuff->prev : tbuff);

		/* if VUSEOTHER try to load two buffers */
		if(Vars[VUSEOTHER].val && Curbuff->prev)
		{
			other = Curbuff->prev;
			strcpy(Lbufname, other->prev ? other->prev->bname : MAINBUFF);
		}
		else
			strcpy(Lbufname, Curbuff->prev ? Curbuff->prev->bname : MAINBUFF);
		Clrecho();
	}
	else
		Loadsaved();

	Winit();
	if(other)
	{
		Z2wind();
		Cswitchto(other);
		Reframe();
		Znextwind();
	}

	Reframe();
	Setmodes(Curbuff);		/* start it off right! */

	if( !Curbuff->mtime && Curbuff->fname ) Echo( "New File" );

	Bind();
	Loadbind();		/* Do this after Tinit */

/*	Setmodes( TRUE ); */
	Curwdo->modeflags = INVALID;

	if(Argp)	Zlgoto();
	if(col > 1) Bmakecol(col - 1, FALSE);

#if XWINDOWS
	if(Spawn)
	{
		if(fork() == 0)		/* fork off editor */
		{
			return;
		}
		exit(0);	/* kill parent */
	}
#else
	/* For xwindows this is set in initSockets */
	FD_ZERO(&SelectFDs);
	FD_SET(1, &SelectFDs);
	NumFDs = 2;
#endif
}


#ifdef SAM_OBSOLETE_USE_FINDFILE
/* Called at startup to read a file. */
Boolean ReadFile(bname, path)
char *bname, *path;
{
	Buffer *b;
	char tbname[BUFNAMMAX + 1], *p;
	int i;

	/* limit name to BUFNAMMAX */
	strncpy(tbname, bname, BUFNAMMAX);
	tbname[BUFNAMMAX] = '\0';

	/* Check if buffer exists.
	 * If one exists with same file name, return, else create unique name.
	 */
	if((b = Cfindbuff(tbname)) != NULL)
	{	/* buffer exists */
		if(strcmp(path, b->fname) == 0) return TRUE;

		/* find a unique name */
		i = strlen(tbname);
		p = &tbname[i < BUFNAMMAX - 3 ? i : BUFNAMMAX - 3];
		i = 0;
		do
			sprintf(p, ".%d", ++i);
		while(Cfindbuff(tbname));
	}

	return Readone(tbname, path);
}
#endif


/*
Read one file, creating the buffer is necessary.
Returns FALSE if unable to create buffer only.
FALSE means that there is no use continuing.
*/
Boolean Readone( bname, path )
char *bname, *path;
{
	extern char Lbufname[];

	int rc;
	Buffer *was;

	was = Curbuff;
	if(Cfindbuff(bname)) return TRUE;
	if(Cmakebuff(bname, path))
	{
		if((rc = Breadfile(path)) >= 0)
		{
			Toggle_mode(0);
			if(rc > 0)
				Echo("New File");
			else if(access(path, R_OK|W_OK) == EOF)
				Curbuff->bmode |= VIEW;
			strcpy(Lbufname, was->bname);
#if XWINDOWS
			XAddBuffer(bname);
#endif
		}
		else
		{	/* error */
			Delbname(Curbuff->bname);
			Bdelbuff(Curbuff);
			Bswitchto(was);
		}
		return TRUE;
	}
	return FALSE;
}


Buffer *Cmakebuff( bname, fname )
char *bname, *fname;
/* Create a buffer. */
{
	extern Buffer *Bufflist;
	Buffer *bptr, *save;

	save = Curbuff;
	if((bptr = Cfindbuff(bname)) != NULL)
		Bswitchto(bptr);
	else if((bptr = Bcreate()) != NULL)
		if((bptr->bname = Addbname(bname)) != NULL)
		{
			Bswitchto(bptr);
			if(fname) bptr->fname = strdup(fname);
			/* add the buffer to the head of the list */
			if(Bufflist) Bufflist->prev = bptr;
			bptr->next = Bufflist;
			Bufflist = bptr;
		}
		else
		{
			Error("Out of buffers");
			Bdelbuff(bptr);
			Bswitchto(save);
			bptr = NULL;
		}
	else
		Error("Unable to create buffer");
	return bptr;
}


/*
Add the new bname to the Bname array.
If we hit maxbuffs, try to enlarge the Bnames array by 10.
Note that the compare MUST be insensitive for the Getplete!
*/
char *Addbname( bname )
char *bname;
{
	static int maxbuffs = 0;		/* max buffers Bnames can hold */
	char **ptr;
	register int i;

	if( Numbuffs == maxbuffs )
	{
		if((ptr = (char **)malloc((maxbuffs + 10) * sizeof(char *))))
		{
			if( maxbuffs )
			{
				memcpy( ptr, Bnames, maxbuffs * sizeof(char *) );
				free( (char *)Bnames );
			}
			Bnames = ptr;
			maxbuffs += 10;
		}
		else
			return( NULL );
	}
	for( i = Numbuffs; i > 0 && Stricmp(bname, Bnames[i - 1]) < 0; --i )
		Bnames[ i ] = Bnames[ i - 1 ];
	Bnames[ i ] = strdup( bname );
	if( strlen(Bnames[i]) > BUFNAMMAX ) Bnames[ i ][ BUFNAMMAX ] = '\0';
	++Numbuffs;
	return( Bnames[i] );
}


Boolean Delbname( bname )
char *bname;
{
	int i, rc;

	if( Numbuffs == 0 ) return( FALSE );
	for( i = rc = 0; i <= Numbuffs && (rc = strcmp(bname, Bnames[i])); ++i ) ;
	if( rc == 0 )
		for( --Numbuffs; i <= Numbuffs; ++i )
			Bnames[ i ] = Bnames[ i + 1 ];
	return( TRUE );
}


Buffer *Cfindbuff( bname )
char *bname;
/* Locate a given buffer */
{
	extern Buffer *Bufflist;
	Buffer *tbuff;

	for( tbuff = Bufflist; tbuff; tbuff = tbuff->next )
		if( Strnicmp(tbuff->bname, bname, BUFNAMMAX) == 0 )
			return( tbuff );
	return( NULL );
}


char *Lastpart( fname )
char *fname;
/* Return a pointer to the start of the last part of fname */
{
	register char *sp;

	for( sp = fname + strlen(fname) - 1; sp >= fname && *sp != '/'; --sp ) ;
	return( ++sp );
}


void Usage(prog)
char *prog;
{
	printf(
"usage: %s [-hntx] [-c config_dir] [fname ... [-l#] [-o#]]\n",
		prog);
	puts("where:\t-h  displays this message.");
#if XWINDOWS
	puts("\t-n  do not spawn window.");
#else
	puts("\t-n  unused in this version.");
#endif
	puts("\t-t  default to text mode.");
#if XWINDOWS
	puts("\t-x  unused in this version.");
#else
	puts("\t-x  cursor cancels mark.");
#endif
	puts("\t-c  specifies a config dir.");
	puts("\t-l  goto specified line number.");
	puts("\t    this only applies to the first file.");
	puts("\t-o  goto specified offset in line.");
	puts("\t    this only applies to the first file.");
#if XWINDOWSy
	xusage();
#endif
	exit(1);
}


Proc Zcwd()
{
	char path[PATHMAX], *p;

	strcpy(path, Cwd);
	if(Getdname("CWD: ", path) == 0) {
		if((p = strdup(path)) == NULL)
			Error("Not enough memory");
		else if(chdir(p) == 0)
		{
			Cwd = p;
#if XWINDOWS
			if(Vars[VSHOWCWD].val) Newtitle(Cwd);
#endif
		}
		else
			Error("chdir failed.");
	}
}


/* Create a file relative to home.
 * On non-unix we just make it current.
 */
char *AddHome(path, fname)
char *path, *fname;
{
	sprintf(path, "%s/%s", Me->pw_dir, fname);
	return path;
}


struct passwd *dup_pwent(pw)
struct passwd *pw;
{
	struct passwd *new;

	if(pw == NULL) return NULL;
	if((new = (struct passwd *)malloc(sizeof(struct passwd))) != NULL)
	{
		memset(new, '\0', sizeof(struct passwd));
		if((new->pw_name	= strdup(pw->pw_name))    != NULL &&
	       (new->pw_passwd	= strdup(pw->pw_passwd))  != NULL &&
#if !LINUX
		   (new->pw_age		= strdup(pw->pw_age))     != NULL &&
		   (new->pw_comment	= strdup(pw->pw_comment)) != NULL &&
#endif
		   (new->pw_gecos	= strdup(pw->pw_gecos))   != NULL &&
		   (new->pw_dir		= strdup(pw->pw_dir))     != NULL &&
		   (new->pw_shell	= strdup(pw->pw_shell))   != NULL)
		{
			new->pw_uid = pw->pw_uid;
		    new->pw_gid = pw->pw_gid;
			return new;
		}
		if(new->pw_name)	free(new->pw_name);
		if(new->pw_passwd)	free(new->pw_passwd);
#if !LINUX
		if(new->pw_age)		free(new->pw_age);
		if(new->pw_comment)	free(new->pw_comment);
#endif
		if(new->pw_gecos)	free(new->pw_gecos);
		if(new->pw_dir)		free(new->pw_dir);
		if(new->pw_shell)	free(new->pw_shell);
		free(new);
	}
	return NULL;
}

void free_pwent(pw)
struct passwd *pw;
{
	free(pw->pw_name);
	free(pw->pw_passwd);
#if !LINUX
	free(pw->pw_age);
	free(pw->pw_comment);
#endif
	free(pw->pw_gecos);
	free(pw->pw_dir);
	free(pw->pw_shell);
	free(pw);
}
