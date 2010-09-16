/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
 
#if XWINDOWS
#include "xkeys.h"
#else

/*
	The keys defined by TERMCAP
	Don't change these without changing kbd.c
		- TCkey array
		- assumes numbers contiguous and start at TC_UP
	Max 32 keys (415)
*/
#define TC_UP			384 /*  0 */
#define TC_DOWN			385 /*  1 */
#define TC_RIGHT		386 /*  2 */
#define TC_LEFT			387 /*  3 */
#define TC_HOME			388 /*  4 */
#define TC_BACK			389 /*  5 */
#define TC_F1			390 /*  6 */
#define TC_F2			391 /*  7 */
#define TC_F3			392 /*  8 */
#define TC_F4			393 /*  9 */
#define TC_F5			394 /* 10 */
#define TC_F6			395 /* 11 */
#define TC_F7			396 /* 12 */
#define TC_F8			397 /* 13 */
#define TC_F9			398 /* 14 */
#define TC_F10			399 /* 15 */
#define TC_F11			400 /* 16 */
#define TC_F12			401 /* 17 */
#define TC_END			402 /* 18 */
#define TC_NPAGE		403 /* 19 */
#define TC_PPAGE		404 /* 20 */
#define TC_INSERT		405 /* 21 */
#define TC_DELETE		406 /* 22 */
#define TC_C_UP			407
#define TC_C_DOWN		408
#define TC_C_RIGHT		409
#define TC_C_LEFT		410
#define TC_C_HOME		411
#define TC_C_END		412
#define NUMKEYS			413

struct key_array
{
	char *key;
	char *label;
};

extern struct key_array Tkeys[];

extern unsigned int Key_mask;

extern char *Term;

#endif

#define K_NODEF			0xffff
#define SPECIAL_START		384
