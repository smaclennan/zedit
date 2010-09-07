/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

#include <stdarg.h>
#include <signal.h>


char *dbgfname = NULL;
Byte Dbgstr[ 256 ];
int Dbgint = 0;

/*
    NOTE: this version is modified to work on a PC with different memory
          models. DON'T optimize!
*/

/*VARARGS0*/
void Dbg(char *fmt, ...)
{
	FILE *fp;
	va_list arg_ptr;

	fp = dbgfname ? fopen(dbgfname, "a") : NULL;
	va_start(arg_ptr, fmt);

	if(fp)
		vfprintf(fp, fmt, arg_ptr);
	else
		vprintf(fmt, arg_ptr);
	va_end(arg_ptr);
	if(fp) fclose(fp);
}


Boolean Dbgname(name)
char *name;
{
	if(dbgfname) free(dbgfname);
	if((dbgfname = malloc(strlen(name) + 1)))
	{
		strcpy(dbgfname, name);
		unlink(dbgfname);
	}
	return(dbgfname != NULL);
}

void Dbgsig(signum)
int signum;
{
	Dbg("Dbgsig: got a %d\n", signum);
	signal(signum, Dbgsig);
}
