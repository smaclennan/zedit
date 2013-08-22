/* bind.c - Zedit key bindings
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
#include "keys.h"

Byte Keys[NUMKEYS], Lfunc;

/* setup the default bindings for the Keys array */
void bind(void)
{
	memset(Keys, ZNOTIMPL, NUMKEYS);
	memset(Keys, ZINSERT,  128);

	Keys[0]  = ZSETMRK;		/* C-@ */
	Keys[1]  = ZBEGLINE;		/* C-A */
	Keys[2]  = ZPREVCHAR;		/* C-B */
	Keys[3]  = ZGETBWORD;		/* C-C */ /**/
	Keys[4]  = ZDELCHAR;		/* C-D */
	Keys[5]  = ZENDLINE;		/* C-E */
	Keys[6]  = ZNEXTCHAR;		/* C-F */
	Keys[7]  = ZABORT;		/* C-G */
	Keys[8]  = ZRDELCHAR;		/* C-H */
	Keys[9]  = ZTAB;		/* C-I */
	Keys[10] = ZCINDENT;		/* C-J */
	Keys[11] = ZDELEOL;		/* C-K */
	Keys[12] = ZREDISPLAY;		/* C-L */
	Keys[13] = ZNEWLINE;		/* C-M */
	Keys[14] = ZNEXTLINE;		/* C-N */
	Keys[15] = ZOPENLINE;		/* C-O */
	Keys[16] = ZPREVLINE;		/* C-P */
	Keys[17] = ZNOTIMPL;		/* C-Q */ /**/
	Keys[18] = ZRSEARCH;		/* C-R */
	Keys[19] = ZINCSRCH;		/* C-S */
	Keys[20] = ZSWAPCHAR;		/* C-T */
	Keys[21] = ZARG;		/* C-U */
	Keys[22] = ZNEXTPAGE;		/* C-V */
	Keys[23] = ZDELRGN;		/* C-W */
	Keys[24] = ZCTRLX;		/* C-X */
	Keys[25] = ZYANK;		/* C-Y */
	Keys[26] = ZPREVPAGE;		/* C-Z */ /**/
	Keys[27] = ZMETA;		/* ESC */
	Keys[28] = ZSETMRK;		/* C-\ */ /**/
	Keys[29] = ZOVERIN;		/* C-] */ /**/
	Keys[30] = ZDELBLANKS;		/* C-^ */ /**/
	Keys[31] = ZUNDO;		/* C-_ */ /**/
	/* 32 - 126 are ZINSERT */
	Keys[127] = ZRDELCHAR;		/* Backspace */

	/* Init the Meta functions */

	Keys[128 + 2] = ZCMDBIND;	/* M-C-B */
	Keys[128 + 7] = ZABORT;		/* M-C-G */
/* M-C-L */
/*	Keys[128 + 13] = ;		 * M-C-M */
	Keys[128 + 14] = ZSCROLLDOWN;	/* M-C-N */
	Keys[128 + 16] = ZSCROLLUP;	/* M-C-P */
	Keys[128 + 19] = ZINCSRCH;	/* M-C-S */
	Keys[128 + 22] = ZVIEWLINE;	/* M-C-V */
	Keys[128 + 27] = ZABORT;	/* M-M */
	Keys[128 + ' '] = ZSEARCH;
/* SAM	Keys[128 + '!'] = Keys[128 + '1'] = ZCMD; */
	Keys[128 + '@'] = Keys[128 + '2'] = ZCMDTOBUFF;
	Keys[128 + '#'] = Keys[128 + '3'] = ZCALC;
	Keys[128 + '&'] = Keys[128 + '7'] = ZISPACE;
	Keys[128 + '*'] = Keys[128 + '8'] = ZUNMODF;
	Keys[128 + '('] = Keys[128 + '9'] = ZTOSTART;
	Keys[128 + ')'] = Keys[128 + '0'] = ZTOEND;
	Keys[128 + ',']			  = ZBEGWIND;
	Keys[128 + '.']			  = ZFINDTAG;
	Keys[128 + '<']			  = ZTOSTART;
	Keys[128 + '>']			  = ZTOEND;
#if SPELL
	Keys[128 + '/'] = Keys[128 + '?'] = ZSPELL;
#endif
	Keys[128 + '_']			  = ZREF;
	Keys[128 + '-']			  = ZFINDTAG;
	Keys[128 + '+'] = Keys[128 + '='] = ZMAKEDEL;
	Keys[128 + 'A'] = Keys[128 + 'a'] = ZAGAIN;
	Keys[128 + 'B'] = Keys[128 + 'b'] = ZBWORD;
	Keys[128 + 'C'] = Keys[128 + 'c'] = ZCAPWORD;
	Keys[128 + 'D'] = Keys[128 + 'd'] = ZDELWORD;
	Keys[128 + 'E'] = Keys[128 + 'e'] = ZRESRCH;
	Keys[128 + 'F'] = Keys[128 + 'f'] = ZFWORD;
	Keys[128 + 'G'] = Keys[128 + 'g'] = ZLGOTO;
	Keys[128 + 'H'] = Keys[128 + 'h'] = ZRDELWORD;
	Keys[128 + 'I'] = Keys[128 + 'i'] = ZCASE;
	Keys[128 + 'J'] = Keys[128 + 'j'] = ZJOIN;
	Keys[128 + 'K'] = Keys[128 + 'k'] = ZKILL;
	Keys[128 + 'L'] = Keys[128 + 'l'] = ZLOWWORD;
	Keys[128 + 'M'] = Keys[128 + 'm'] = ZFILLPARA;
	Keys[128 + 'N'] = Keys[128 + 'n'] = ZFPARA;
	Keys[128 + 'O'] = Keys[128 + 'o'] = ZREVERTFILE;
	Keys[128 + 'P'] = Keys[128 + 'p'] = ZBPARA;
	Keys[128 + 'Q'] = Keys[128 + 'q'] = ZQUOTE;
	Keys[128 + 'R'] = Keys[128 + 'r'] = ZQUERY;
	Keys[128 + 'S'] = Keys[128 + 's'] = ZSEARCH;
	Keys[128 + 'T'] = Keys[128 + 't'] = ZSWAPWORD;
	Keys[128 + 'U'] = Keys[128 + 'u'] = ZUPWORD;
	Keys[128 + 'V'] = Keys[128 + 'v'] = ZPREVPAGE;
	Keys[128 + 'W'] = Keys[128 + 'w'] = ZCOPYRGN;
	Keys[128 + 'X'] = Keys[128 + 'x'] = ZMETAX;
	Keys[128 + 'Y'] = Keys[128 + 'y'] = ZYANK;
	Keys[128 + 'Z'] = Keys[128 + 'z'] = ZEXIT;
	Keys[128 + DEL] = ZRDELWORD;		/* M-DEL */

	/* Init the CTRL-X functions */

	Keys[256 +  1]  = ZSAVEALL;		/* C-X C-A */
	Keys[256 +  2]  = ZSWITCHTO;		/* C-X C-B */
	Keys[256 +  3]  = ZQUIT;		/* C-X C-C */
	Keys[256 +  4]  = ZKILLBUFF;		/* C-X C-D */
/* C-X C-E */
	Keys[256 +  6]  = ZFINDFILE;		/* C-X C-F */
	Keys[256 +  7]  = ZABORT;		/* C-X C-G */
	Keys[256 +  8]  = ZHEXOUT;		/* C-X C-H */
/* C-X C-I */
/* C-X C-J */
	Keys[256 + 11]  = ZDELLINE;		/* C-X C-K */
	Keys[256 + 12]  = ZLOWREGION;		/* C-X C-L */
	Keys[256 + 13]  = ZMAKE;		/* C-X C-M */
	Keys[256 + 14]  = ZNEXTERR;		/* C-X C-N */
	Keys[256 + 15]  = ZCGOTO;		/* C-X C-O */
	Keys[256 + 16]  = ZMRKPARA;		/* C-X C-P */
/* C-X C-Q */
	Keys[256 + 18]  = ZFILEREAD;		/* C-X C-R */
	Keys[256 + 19]  = ZFILESAVE;		/* C-X C-S */
/* C-X C-T */
	Keys[256 + 21]  = ZUPREGION;		/* C-X C-U */
	Keys[256 + 22]  = ZNXTOTHRWIND;		/* C-X C-V */
	Keys[256 + 23]  = ZFILEWRITE;		/* C-X C-W */
	Keys[256 + 24]  = ZSWAPMRK;		/* C-X C-X */
/* C-X C-Y */
	Keys[256 + 26]  = ZPREVOTHRWIND;	/* C-X C-Z */
	Keys[256 + '='] = ZPRINTPOS;		/* C-X = */
	Keys[256 + '1'] = Z1WIND;		/* C-X 1 */
	Keys[256 + '2'] = Z2WIND;		/* C-X 2 */
/* C-X ( */
/* C-X ) */
	Keys[256 + 'A'] = Keys[256 + 'a'] = ZGSEARCH;		/* C-X A */
	Keys[256 + 'B'] = Keys[256 + 'b'] = ZNXTBOOKMRK;	/* C-X B */
	Keys[256 + 'C'] = Keys[256 + 'c'] = ZCOUNT;		/* C-X C */
	Keys[256 + 'D'] = Keys[256 + 'd'] = ZDATE;		/* C-X D */
	Keys[256 + 'E'] = Keys[256 + 'e'] = ZREREPLACE;		/* C-X E */
/* C-X F */
/* C-X G */
	Keys[256 + 'H'] = Keys[256 + 'h'] = ZHELP;		/* C-X H */
	Keys[256 + 'I'] = Keys[256 + 'i'] = ZINDENT;		/* C-X I */
/* C-X J */
	Keys[256 + 'K'] = Keys[256 + 'k'] = ZKILLBUFF;		/* C-X K */
	Keys[256 + 'L'] = Keys[256 + 'l'] = ZLSTBUFF;		/* C-X L */
	Keys[256 + 'M'] = Keys[256 + 'm'] = ZSETBOOKMRK;	/* C-X M */
	Keys[256 + 'N'] = Keys[256 + 'n'] = ZNEXTWIND;		/* C-X N */
	Keys[256 + 'O'] = Keys[256 + 'o'] - ZNEXTWIND;		/* C-X O */
	Keys[256 + 'P'] = Keys[256 + 'p'] = ZPREVWIND;		/* C-X P */
/* C-X Q */
/* C-X R */
	Keys[256 + 'S'] = Keys[256 + 's'] = ZSAVEALL;		/* C-X S */
	Keys[256 + 'T'] = Keys[256 + 't'] = ZDELWHITE;		/* C-X T */
	Keys[256 + 'U'] = Keys[256 + 'u'] = ZUNDENT;		/* C-X U */
	Keys[256 + 'V'] = Keys[256 + 'v'] = ZSETAVAR;		/* C-X V */
	Keys[256 + 'W'] = Keys[256 + 'w'] = ZFILEWRITE;		/* C-X W */
	Keys[256 + 'X'] = Keys[256 + 'x'] = ZNEXTBUFF;		/* C-X X */
/* C-X Y */
	Keys[256 + 'Z'] = Keys[256 + 'z'] = ZEXIT;		/* C-X Z */
	Keys[256 + '^'] = ZGROWWINDOW;				/* C-X ^ */

	Keys[TC_UP]	= ZPREVLINE;
	Keys[TC_C_UP]	= ZPREVPAGE;

	Keys[TC_DOWN]	= ZNEXTLINE;
	Keys[TC_C_DOWN]	= ZNEXTPAGE;

	Keys[TC_LEFT]	= ZPREVCHAR;
	Keys[TC_C_LEFT]	= ZBWORD;

	Keys[TC_RIGHT]	= ZNEXTCHAR;
	Keys[TC_C_RIGHT] = ZFWORD;

	Keys[TC_HOME]	= ZBEGLINE;
	Keys[TC_C_HOME]	= ZTOSTART;

	Keys[TC_END]	= ZENDLINE;
	Keys[TC_C_END]	= ZTOEND;

	Keys[TC_BACK]	= ZRDELCHAR;
	Keys[TC_NPAGE]	= ZNEXTPAGE;
	Keys[TC_PPAGE]	= ZPREVPAGE;
	Keys[TC_INSERT]	= ZOVERIN;
	Keys[TC_DELETE]	= ZDELCHAR;

	Keys[TC_F1]	= ZFINDFILE;
	Keys[TC_F2]	= ZSEARCH;
	Keys[TC_F3]	= ZAGAIN;
	Keys[TC_F4]	= ZNEXTERR;
	Keys[TC_F5]	= ZREREPLACE;
	/* Keys[TC_F6]	= ; */
	Keys[TC_F7]	= ZMAKE;
	Keys[TC_F8]	= ZGREP;
	Keys[TC_F9]	= ZSETBOOKMRK;
	Keys[TC_F10]	= ZNXTBOOKMRK;
	/* Keys[TC_F11] = ; */
	Keys[TC_F12]	= ZREVERTFILE;
}
