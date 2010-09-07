/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
/* EMM routines */
int  EMMinit ARGS((int min));
void EMMfree ARGS((void));
void EMMread  ARGS((int lpage, Byte *data));
void EMMwrite ARGS((int lpage, Byte *data));

/* type defines for EMMtype */
#define USESWAP		0
#define EXTENDED	1
#define EXPANDED	2

extern int EMMtype;
