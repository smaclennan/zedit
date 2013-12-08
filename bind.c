/* bind.c - Zedit key bindings
 * Copyright (C) 1988-2013 Sean MacLennan
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
Byte CRdefault = ZNEWLINE;

/* setup the default bindings for the Keys array */
void zbind(void)
{
	memset(Keys, ZNOTIMPL, NUMKEYS);

	Keys[0]  = ZSET_MARK;			/* C-@ */
	Keys[1]  = ZBEGINNING_OF_LINE;		/* C-A */
	Keys[2]  = ZPREVIOUS_CHAR;		/* C-B */
	Keys[3]  = ZCOPY_WORD;			/* C-C */
	Keys[4]  = ZDELETE_CHAR;		/* C-D */
	Keys[5]  = ZEND_OF_LINE;		/* C-E */
	Keys[6]  = ZNEXT_CHAR;			/* C-F */
	Keys[7]  = ZABORT;			/* C-G */
	Keys[8]  = ZHELP;			/* C-H */
	Keys[9]  = ZTAB;			/* C-I */
	Keys[10] = ZC_INDENT;			/* C-J */
	Keys[11] = ZDELETE_TO_EOL;		/* C-K */
	Keys[12] = ZREDISPLAY;			/* C-L */
	Keys[13] = ZNEWLINE;			/* C-M */
	Keys[14] = ZNEXT_LINE;			/* C-N */
	Keys[15] = ZOPEN_LINE;			/* C-O */
	Keys[16] = ZPREVIOUS_LINE;		/* C-P */
	Keys[17] = ZQUOTE;			/* C-Q */
	Keys[18] = ZREVERSE_SEARCH;		/* C-R */
	Keys[19] = ZINCREMENTAL_SEARCH;		/* C-S */
	Keys[20] = ZSWAP_CHARS;			/* C-T */
	Keys[21] = ZARG;			/* C-U */
	Keys[22] = ZNEXT_PAGE;			/* C-V */
	Keys[23] = ZDELETE_REGION;		/* C-W */
	Keys[24] = ZCTRL_X;			/* C-X */
	Keys[25] = ZYANK;			/* C-Y */
	Keys[26] = ZPREVIOUS_PAGE;		/* C-Z */
	Keys[27] = ZMETA;			/* ESC */
	Keys[28] = ZSET_MARK;			/* C-\ */
	Keys[29] = ZINSERT_OVERWRITE;		/* C-] */
	Keys[30] = ZDELETE_BLANKS;		/* C-^ */
	Keys[31] = ZUNDO;			/* C-_ */
	/* 32 - 126 are ZINSERT */
	memset(Keys + 32, ZINSERT,  127 - 32);
	Keys[127] = ZDELETE_PREVIOUS_CHAR;	/* Backspace */

	/* Init the Meta functions */

/*	Keys[128 + 2] = ;			* M-C-B */
	Keys[128 + 7] = ZABORT;			/* M-C-G */
	Keys[128 + 14] = ZSCROLL_DOWN;		/* M-C-N */
	Keys[128 + 16] = ZSCROLL_UP;		/* M-C-P */
	Keys[128 + 19] = ZINCREMENTAL_SEARCH;	/* M-C-S */
	Keys[128 + 22] = ZVIEW_LINE;		/* M-C-V */
	Keys[128 + 27] = ZABORT;		/* M-M */
	Keys[128 + ' '] = ZSEARCH;
/*	Keys[128 + '!'] = Keys[128 + '1'] = ; */
	Keys[128 + '@'] = Keys[128 + '2'] = ZCMD_TO_BUFFER;
	Keys[128 + '#'] = Keys[128 + '3'] = ZCALC;
	Keys[128 + '$'] = Keys[128 + '4'] = ZSPELL_WORD;
/*	Keys[128 + '&'] = Keys[128 + '7'] = ; */
	Keys[128 + '*'] = Keys[128 + '8'] = ZUNMODIFY;
	Keys[128 + '('] = Keys[128 + '9'] = ZBEGINNING_OF_BUFFER;
	Keys[128 + ')'] = Keys[128 + '0'] = ZEND_OF_BUFFER;
/*	Keys[128 + ',']			  = ; */
	Keys[128 + '<']			  = ZBEGINNING_OF_BUFFER;
	Keys[128 + '>']			  = ZEND_OF_BUFFER;
	Keys[128 + '.']			  = ZTAG;
/*	Keys[128 + '/'] = Keys[128 + '?'] = ; */
/*	Keys[128 + '_']			  = ; */
/*	Keys[128 + '-']			  = ; */
	Keys[128 + '+'] = Keys[128 + '='] = ZAPPEND_KILL;
	/* All lowercase chars converted to uppercase */
	Keys[128 + 'A'] = ZAGAIN;
	Keys[128 + 'B'] = ZPREVIOUS_WORD;
	Keys[128 + 'C'] = ZCAPITALIZE_WORD;
	Keys[128 + 'D'] = ZDELETE_WORD;
	Keys[128 + 'E'] = ZRE_SEARCH;
	Keys[128 + 'F'] = ZNEXT_WORD;
	Keys[128 + 'G'] = ZGOTO_LINE;
	Keys[128 + 'H'] = ZDELETE_PREVIOUS_WORD;
	Keys[128 + 'I'] = ZTAB;
	Keys[128 + 'J'] = ZJOIN;
	Keys[128 + 'K'] = ZKILL;
	Keys[128 + 'L'] = ZLOWERCASE_WORD;
	Keys[128 + 'M'] = ZFILL_PARAGRAPH;
	Keys[128 + 'N'] = ZNEXT_PARAGRAPH;
	Keys[128 + 'O'] = ZREVERT_FILE;
	Keys[128 + 'P'] = ZPREVIOUS_PARAGRAPH;
	Keys[128 + 'Q'] = ZQUOTE;
	Keys[128 + 'R'] = ZQUERY_REPLACE;
	Keys[128 + 'S'] = ZSEARCH;
	Keys[128 + 'T'] = ZSWAP_WORDS;
	Keys[128 + 'U'] = ZUPPERCASE_WORD;
	Keys[128 + 'V'] = ZPREVIOUS_PAGE;
	Keys[128 + 'W'] = ZCOPY_REGION;
	Keys[128 + 'X'] = ZMETA_X;
	Keys[128 + 'Y'] = ZYANK;
	Keys[128 + 'Z'] = ZSAVE_AND_EXIT;
	Keys[128 + 127] = ZDELETE_PREVIOUS_WORD;	/* M-DEL */

	/* Init the CTRL-X functions */

	Keys[256 +  1]  = ZSAVE_ALL_FILES;	/* C-X C-A */
	Keys[256 +  2]  = ZSWITCH_TO_BUFFER;	/* C-X C-B */
	Keys[256 +  3]  = ZEXIT;		/* C-X C-C */
	Keys[256 +  4]  = ZDELETE_BUFFER;	/* C-X C-D */
/* C-X C-E */
	Keys[256 +  6]  = ZFIND_FILE;		/* C-X C-F */
	Keys[256 +  7]  = ZABORT;		/* C-X C-G */
/* C-X C-H */
/* C-X C-I */
/* C-X C-J */
	Keys[256 + 11]  = ZDELETE_LINE;		/* C-X C-K */
	Keys[256 + 12]  = ZLOWERCASE_REGION;	/* C-X C-L */
	Keys[256 + 13]  = ZMAKE;		/* C-X C-M */
	Keys[256 + 14]  = ZNEXT_ERROR;		/* C-X C-N */
	Keys[256 + 15]  = ZOUT_TO;		/* C-X C-O */
	Keys[256 + 16]  = ZMARK_PARAGRAPH;	/* C-X C-P */
/* C-X C-Q */
	Keys[256 + 18]  = ZREAD_FILE;		/* C-X C-R */
	Keys[256 + 19]  = ZSAVE_FILE;		/* C-X C-S */
/* C-X C-T */
	Keys[256 + 21]  = ZUPPERCASE_REGION;	/* C-X C-U */
	Keys[256 + 22]  = ZOTHER_NEXT_PAGE;	/* C-X C-V */
	Keys[256 + 23]  = ZWRITE_FILE;		/* C-X C-W */
	Keys[256 + 24]  = ZSWAP_MARK;		/* C-X C-X */
/* C-X C-Y */
	Keys[256 + 26]  = ZOTHER_PREVIOUS_PAGE;	/* C-X C-Z */
	Keys[256 + '='] = ZPOSITION;		/* C-X = */
	Keys[256 + '1'] = ZONE_WINDOW;		/* C-X 1 */
	Keys[256 + '2'] = ZSPLIT_WINDOW;	/* C-X 2 */
/* C-X ( */
/* C-X ) */
	/* All lowercase converted to uppercase */
	Keys[256 + 'A'] = ZGLOBAL_SEARCH;	/* C-X A */
	Keys[256 + 'B'] = ZNEXT_BOOKMARK;	/* C-X B */
	Keys[256 + 'C'] = ZCOUNT;		/* C-X C */
/* C-X D */
	Keys[256 + 'E'] = ZRE_REPLACE;		/* C-X E */
	Keys[256 + 'F'] = ZNEXT_PARAGRAPH;	/* C-X F */
/* C-X G */
	Keys[256 + 'H'] = ZHELP_FUNCTION;	/* C-X H */
	Keys[256 + 'I'] = ZINDENT;		/* C-X I */
/* C-X J */
	Keys[256 + 'K'] = ZDELETE_BUFFER;	/* C-X K */
	Keys[256 + 'L'] = ZLIST_BUFFERS;	/* C-X L */
	Keys[256 + 'M'] = ZSET_BOOKMARK;	/* C-X M */
	Keys[256 + 'N'] = ZNEXT_WINDOW;	/* C-X N */
	Keys[256 + 'O'] = ZNEXT_WINDOW;	/* C-X O */
/* C-X P */
/* C-X Q */
/* C-X R */
	Keys[256 + 'S'] = ZSAVE_ALL_FILES;	/* C-X S */
	Keys[256 + 'T'] = ZTRIM_WHITE_SPACE;	/* C-X T */
	Keys[256 + 'U'] = ZUNDENT;		/* C-X U */
	Keys[256 + 'V'] = ZSET_VARIABLE;	/* C-X V */
	Keys[256 + 'W'] = ZWRITE_FILE;	/* C-X W */
	Keys[256 + 'X'] = ZNEXT_BUFFER;	/* C-X X */
/* C-X Y */
	Keys[256 + 'Z'] = ZEXIT;		/* C-X Z */
	Keys[256 + '^'] = ZGROW_WINDOW;				/* C-X ^ */

	/* Special keys */

	Keys[TC_UP]	= ZPREVIOUS_LINE;
	Keys[TC_DOWN]	= ZNEXT_LINE;
	Keys[TC_LEFT]	= ZPREVIOUS_CHAR;
	Keys[TC_RIGHT]	= ZNEXT_CHAR;

	Keys[TC_NPAGE]	= ZNEXT_PAGE;
	Keys[TC_PPAGE]	= ZPREVIOUS_PAGE;
	Keys[TC_HOME]	= ZBEGINNING_OF_LINE;
	Keys[TC_END]	= ZEND_OF_LINE;
	Keys[TC_INSERT]	= ZINSERT_OVERWRITE;
	Keys[TC_DELETE]	= ZDELETE_CHAR;

	Keys[TC_F1]	= ZFIND_FILE;
	Keys[TC_F2]	= ZSEARCH;
	Keys[TC_F3]	= ZAGAIN;
	Keys[TC_F4]	= ZNEXT_ERROR;
	Keys[TC_F5]	= ZRE_REPLACE;
	/* Keys[TC_F6]	= ; */
	Keys[TC_F7]	= ZMAKE;
	Keys[TC_F8]	= ZGREP;
	Keys[TC_F9]	= ZWORD_SEARCH;
	Keys[TC_F10]	= ZTAG_WORD;
	Keys[TC_F11]	= ZNEXT_BOOKMARK;
	Keys[TC_F12]	= ZREVERT_FILE;
}
