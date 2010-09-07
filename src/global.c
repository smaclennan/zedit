/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

/*
 * This file contains all the globals that must be saved in the save.z file.
 * By putting them in one place, hopefully they will all be consecutive.
 * All strings MUST be on a word boundry! Note that BUFNAMMAX, PATHMAX, and
 *	STRMAX are defined so that adding 1 makes them right.
 *
 * ALL GLOBALS MUST BE AUTO INITIALIZED HERE TO BE SAVED!
 *
 * For the HP Series 700 - all variable must be > 4 words to keep it all
 * in the data section (as apposed to short data).
 */

char G_start[12] = VERSTR;

char Lbufname[BUFNAMMAX + 1] = MAINBUFF;	/* Zswitchto */
char Fname[PATHMAX + 1] = "";				/* Zfindfile, Zfileread */

char Command[STRMAX + 1] = "";				/* Zcmd, Zcmdtobuff */
char mkcmd[STRMAX + 1] = "make";			/* Zmake, Vars */
char grepcmd[STRMAX + 1] = "grep -n";		/* Zgrep, Vars */

char old[STRMAX + 1] = "";					/* Search string */
char new[STRMAX + 1] = "";					/* Replace string */
Boolean searchdir[3] = {0, 0};				/* Current direction for Again.
										 	 * searchdir[2] unused - padding
											 * for hp.
										 	 */

char Calc_str[STRMAX + 1] = "";				/* Zcalc */

char G_end[16] = "Sean MacLennan";
