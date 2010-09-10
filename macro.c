/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include "keys.h"
#include "help.h"
 
#define Newmacro()	((Short *)malloc(MSIZE))

Short *Mbuff = NULL, *Mptr, *Mend, Mstate = FALSE;
Short *Macro[ NUMMACROS ];


Proc Zstartmacro()
{
	extern int Arg;
	
	if( Mbuff || (Mbuff = Newmacro()) )
	{
		memset( Mbuff, '?', MSIZE );
		Mstate = MCREATE;
		Mptr = Mbuff;
		Mend = Mbuff + (MSIZE >> 1);
		Echo( "Start Macro Def" );
	}
	else
		Error( "Not Enough memory" );
	Arg = 0;
}


Proc Zendmacro()
{
	extern int Arg;
	
	if( Mstate == MCREATE ) Echo( "End Macro Def" );
	Mstate = FALSE;
	Arg = 0;
}


Proc Zdomacro()
{
	Domacro( Mbuff );
}


Proc Znamemacro()
{
	extern struct cnames Cnames[];
	char name[ BUFNAMMAX + 1 ];
	int buff, j, k, len;
	
	if( Mbuff )
	{
		for( buff = 0; buff < NUMMACROS; ++buff )
			if( Macro[buff] == NULL )
			{
				for( j = 0; j < NUMFUNCS; ++j )
					if( Cnames[j].fnum == ZMACRON + buff )
						break;
				strcpy( name, Cnames[j].name );
				do
				{
					if( Getarg("Macro Name: ", name, BUFNAMMAX) ) return;
					len = strlen( name );
					for( k = 0; k < NUMFUNCS; ++k )
						if( Strnicmp(Cnames[k].name, name, len) == 0 )
						{
							if( k != j ) Tbell();
							break;
						}
				}
				while( k != NUMFUNCS && k != j );
				if((Macro[buff] = Newmacro()))
				{
					Addtocnames( j, name );
					memcpy( Macro[buff], Mbuff, MSIZE );
				}
				else
					Error( "Not Enough Memory" );
				return;
			}
		Error( "Out of Macro Slots" );
	}
	else
		Tbell();
}


Proc Zmacron()
{
	extern Byte Keys[], Lfunc;
	extern unsigned Cmd;

	if( Lfunc != ZMETAX ) Cmd = Keys[ Cmd ];
	Domacro( Macro[Cmd - ZMACRON] );
}


Proc Zwritemacro()
{
	Macrofile( NULL, TRUE );
}


Proc Zreadmacro()
{
	Macrofile( NULL, FALSE );
}


/*
Process the macro file. The macro files are only accessed via this routine.
Note: Zedit '?' pads the macros, 'zc' '.' pads it.
Format:

version				(Short - defined in z.h)
[
	macro name		(variable - NL terminated)
	key def			(Short)
	macro			(MSIZE bytes)
] ...

*/
void Macrofile( fname, write )
char *fname;
Boolean write;
{
	extern Byte Keys[];
	extern struct cnames Cnames[];
	FILE *fp;
	char mname[ PATHMAX + 1 ];
	Short i, j, k;

	if( fname )
		strcpy( mname, fname );
	else
	{
		if( !fname && !write )
			strcpy( mname, "z.mac" );
		else
			*mname = '\0';
		if( Getfname("Macro File: ", mname) ) return;
	}
	if( !(fp = fopen(mname, write ? "w" : "r")) )
	{
		if( !fname ) Error( "Unable to open macro file" );
		return;
	}
	if(Verbose) Dbg("%s macro file %s\n", write ? "Write" : "Read", fname);
	if( write )
	{
		/* put version in file */
		i = MACROVERSION;
		fwrite( (char *)&i, sizeof(Short), 1, fp );

		/* process the macros */
		for( i = 0; i < NUMMACROS && Macro[i]; ++i )
			for( j = 0; j < NUMFUNCS; ++j )
				if( Cnames[j].fnum == ZMACRON + i )
				{
					/* store the macro name */
					fprintf( fp, "%s\n", Cnames[j].name );
					/* Find the binding for this macro if any */
					for( k = 0;
						 (unsigned)k < NUMKEYS && Keys[k] != Cnames[j].fnum;
						 ++k ) ;
					fwrite( (char *)&k, sizeof(Short), 1, fp );
					/* save the macro buffer */
					fwrite( (char *)Macro[i], MSIZE, 1, fp );
					break;
				}
	}
	else if(fread((char *)&i, sizeof(Short), 1, fp) != 1)
		Error("Bad macro file.");
	else if(i != MACROVERSION)
	{
		sprintf(PawStr, "Macroversion %x expected %x\n", i, MACROVERSION);
		Error(PawStr);
	}
	else
	{
		for( i = 0; Readstr(mname, fp); ++i )
			for( j = 0; j < NUMFUNCS; ++j )
				if( Cnames[j].fnum == ZMACRON + i )
				{
					/*
						If this macro slot was already defined,
						unbind it from any keys
					*/
					if( Macro[i] )
						for(k = 0; (unsigned)k < NUMKEYS; ++k)
							if( Keys[k] == ZMACRON + i )
								Keys[ k ] = ZNOTIMPL;
					if( Macro[i] || (Macro[i] = Newmacro()) )
					{
						Addtocnames( j, mname );
						if(fread((void *)&k, sizeof(Short), 1, fp) &&
							(unsigned)k < NUMKEYS )
								Keys[ k ] = ZMACRON + i;
						fread( (char *)Macro[i], MSIZE, 1, fp );
						break;
					}
				}
		}
	fclose( fp );
}
						

void Addtomacro( cmd )
int cmd;
{
	if( Mptr < Mend )
		*Mptr++ = cmd;
	else
	{
		Mstate = FALSE;
		Mbuff = NULL;
		Error( "Macro too long" );
	}
}

		
void Domacro( mbuff )
Short *mbuff;
{
	extern int Arg;
	int cnt;
	
	if( mbuff && !Mstate )
	{
		for( cnt = Arg; cnt > 0 && Mstate != MABORT; --cnt )
		{
			Mstate = INMACRO;
			Mptr = mbuff;
			while( Mstate == INMACRO )
				Execute();
		}
		Mstate = FALSE;
	}
	else
		Tbell();
}


void Gomacro()
{
	if(Macro[0]) Domacro(Macro[0]);
}

void Addtocnames( n, name )
int n;
char *name;
{
	extern struct cnames Cnames[];
	int fnum, i;
	
	fnum = Cnames[ n ].fnum;
	for( i = n; i > 0 && Stricmp(Cnames[i - 1].name, name) > 0; --i )
	{
		Cnames[ i ].name = Cnames[ i - 1 ].name;
		Cnames[ i ].fnum = Cnames[ i - 1 ].fnum;
	}
	for( ; i < NUMFUNCS - 1 && Stricmp(Cnames[i + 1].name, name) < 0; ++i )
	{
		Cnames[ i ].name = Cnames[ i + 1 ].name;
		Cnames[ i ].fnum = Cnames[ i + 1 ].fnum;
	}
	Cnames[ i ].name = strdup( name );
	Cnames[ i ].fnum = fnum;
}
