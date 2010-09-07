#include "z.h"
#ifdef MSDOS
#include <dos.h>

void saveasfile();


int DosLoad(arg, argc, argv)
int arg, argc;
char *argv[];
{
	char path[ PATHMAX + 1 ], *ptr;
	char fname[ STRMAX + 1 ];
	int files = 0;
#ifdef __TURBOC__
	struct ffblk find;
#else
	struct find_t find;
#endif

	for( ; arg < argc; ++arg )
		if( argv[arg][0] == '-' )
			switch( argv[arg][1] )
			{
				case 'l':
					Argp = TRUE;
					Arg = (int)strtol( &argv[arg][2], NULL, 0 );
					break;
			}
		/* try to expand possible wildcards */
		else if(FINDFIRST(argv[arg], &find, FF_FILES) == 0 )
		{
			strcpy( fname, argv[arg] );
			ptr = Lastpart( fname );
			do
			{
				++files;
				strlwr( FF_NAME(&find) );
				strcpy( ptr, FF_NAME(&find) );
				if( Pathfixup(path, fname) == 0 )
					if( !Readone(FF_NAME(&find), path) )	
					{
						arg = argc + 1;	/* abort main loop */
						break;			/* abort this loop */
					}
			}
			while( FINDNEXT(&find) == 0 );
		}
		else
		{
			++files;
			if( Pathfixup(path, argv[arg]) == 0 )
			if( !Readone(Lastpart(argv[arg]), path) )
				break;
		}
	return files;
}
#endif



/* send the current buffer to the printer */
Proc Zprint()
{
	char cmd[STRMAX + 20];
#if MSDOS
	char fname[20];
	FILE *fp;
#else
	int rc = 0;
#endif
	
	Echo("Printing...");
#if MSDOS
	saveasfile(fname);

	/*  Use the print command. We must send port as needed. */
	if((fp = fopen("print.in", "w")) != NULL)
	{
		fputs((char *)Vars[VPRINT].val, fp);
		fclose(fp);
	}
	sprintf(cmd, "print %s <print.in", fname);
	Dopipe("print.out", cmd);
	unlink("print.in");
	unlink("print.out");
#else
	strcpy(cmd, Vars[VPRINT].val);	/* note that BuffToPipe updates cmd */
	PrintExit(BuffToPipe(Curbuff, cmd));
#endif
}


#if MSDOS
void saveasfile(fname)
char *fname;
{
	int bmodf;

	mktemp(strcpy(fname, MAILTMP));

	if(Argp)
		Write_rgn(fname);
	else
	{
		bmodf = Curbuff->bmodf;
		Bwritefile(fname);
		Curbuff->bmodf = bmodf;
	}
}
#endif


void HandleDrive(from, to)
char **from, **to;
{
		unsigned drive;
#ifdef __TURBOC__
		unsigned junk;

		drive = getdisk();
		drive += 'A';
#else
		_dos_getdrive(&drive);
		drive += '@';
#endif
		if( *(*from + 1) == ':' )
		{
			if((**to = Toupper(**from)) != drive)
			{
#ifdef __TURBOC__
				setdisk(**to - 'A');
#else
				_dos_setdrive(**to - '@', &junk);
#endif
			}
			*++*to = ':';
			++*to;
			++*from;
		}
		else if(Vars[VEXPAND].val)
		{
			*(*to)++ = drive;
			*(*to)++ = ':';
		}
		**to = '\0';
}
