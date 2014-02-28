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

Byte Lfunc;
Byte CRdefault = ZNEWLINE;

Byte Keys[NUMKEYS] = {
	ZSET_MARK,		/* C-@ */
	ZBEGINNING_OF_LINE,	/* C-A */
	ZPREVIOUS_CHAR,		/* C-B */
	ZCOPY_WORD,		/* C-C */
	ZDELETE_CHAR,		/* C-D */
	ZEND_OF_LINE,		/* C-E */
	ZNEXT_CHAR,		/* C-F */
	ZABORT,			/* C-G */
	ZHELP,			/* C-H */
	ZTAB,			/* C-I */
	ZC_INDENT,		/* C-J */
	ZDELETE_TO_EOL,		/* C-K */
	ZREDISPLAY,		/* C-L */
	ZNEWLINE,		/* C-M */
	ZNEXT_LINE,		/* C-N */
	ZOPEN_LINE,		/* C-O */
	ZPREVIOUS_LINE,		/* C-P */
	ZQUOTE,			/* C-Q */
	ZREVERSE_SEARCH,	/* C-R */
	ZINCREMENTAL_SEARCH,	/* C-S */
	ZSWAP_CHARS,		/* C-T */
	ZARG,			/* C-U */
	ZNEXT_PAGE,		/* C-V */
	ZDELETE_REGION,		/* C-W */
	ZCTRL_X,		/* C-X */
	ZYANK,			/* C-Y */
	ZPREVIOUS_PAGE,		/* C-Z */
	ZMETA,			/* ESC */
	ZSET_MARK,		/* C-\ */
	ZINSERT_OVERWRITE,	/* C-] */
	ZDELETE_BLANKS,		/* C-^ */
	ZUNDO,			/* C-_ */

	/* 32 - 126 are ZINSERT */
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,
	ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT, ZINSERT,

	ZDELETE_PREVIOUS_CHAR,	/* 127 Backspace */

	/* Init the CTRL-X functions */

	0,			/* C-X C-@ */
	ZSAVE_ALL_FILES,	/* C-X C-A */
	ZSWITCH_TO_BUFFER,	/* C-X C-B */
	ZEXIT,			/* C-X C-C */
	ZDELETE_BUFFER,		/* C-X C-D */
	0,			/* C-X C-E */
	ZFIND_FILE,		/* C-X C-F */
	ZABORT,			/* C-X C-G */
	0,			/* C-X C-H */
	0,			/* C-X C-I */
	0,			/* C-X C-J */
	ZDELETE_LINE,		/* C-X C-K */
	ZLOWERCASE_REGION,	/* C-X C-L */
	ZMAKE,			/* C-X C-M */
	ZNEXT_ERROR,		/* C-X C-N */
	ZOUT_TO,		/* C-X C-O */
	ZMARK_PARAGRAPH,	/* C-X C-P */
	0,			/* C-X C-Q */
	ZREAD_FILE,		/* C-X C-R */
	ZSAVE_FILE,		/* C-X C-S */
	0,			/* C-X C-T */
	ZUPPERCASE_REGION,	/* C-X C-U */
	ZOTHER_NEXT_PAGE,	/* C-X C-V */
	ZWRITE_FILE,		/* C-X C-W */
	ZSWAP_MARK,		/* C-X C-X */
	0,			/* C-X C-Y */
	ZOTHER_PREVIOUS_PAGE,	/* C-X C-Z */
	0,			/* C-X Esc */
	0,			/* C-X C-\ */
	0,			/* C-X C-] */
	ZGROW_WINDOW,		/* C-X C-^ */
	0,			/* C-X C-_ */

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0,			/* C-X 0 */
	ZONE_WINDOW,		/* C-X 1 */
	ZSPLIT_WINDOW,		/* C-X 2 */
	0,			/* C-X 3 */
	0,			/* C-X 4 */
	0,			/* C-X 5 */
	0,			/* C-X 6 */
	0,			/* C-X 7 */
	0,			/* C-X 8 */
	0,			/* C-X 9 */

	0, 0, 0,
	ZPOSITION,		/* C-X = */
	0, 0, 0,

	/* All lowercase converted to uppercase */
	ZGLOBAL_SEARCH,		/* C-X A */
	ZNEXT_BOOKMARK,		/* C-X B */
	ZCOUNT,			/* C-X C */
	0,			/* C-X D */
	ZRE_REPLACE,		/* C-X E */
	ZNEXT_PARAGRAPH,	/* C-X F */
	0,			/* C-X G */
	ZHELP_FUNCTION,		/* C-X H */
	ZINDENT,		/* C-X I */
	0,			/* C-X J */
	ZDELETE_BUFFER,		/* C-X K */
	ZLIST_BUFFERS,		/* C-X L */
	ZSET_BOOKMARK,		/* C-X M */
	ZNEXT_WINDOW,		/* C-X N */
	ZNEXT_WINDOW,		/* C-X O */
	0,			/* C-X P */
	0,			/* C-X Q */
	0,			/* C-X R */
	ZSAVE_ALL_FILES,	/* C-X S */
	ZTRIM_WHITE_SPACE,	/* C-X T */
	ZUNDENT,		/* C-X U */
	ZSET_VARIABLE,		/* C-X V */
	ZWRITE_FILE,		/* C-X W */
	ZNEXT_BUFFER,		/* C-X X */
	0,			/* C-X Y */
	ZEXIT,			/* C-X Z */
	0, 0, 0,
	ZGROW_WINDOW,		/* C-X ^ */

	/* Init the Meta functions */

};

void bind_init(void)
{
	Keys[M(7)] = ZABORT;			/* M-C-G */
	Keys[M(14)] = ZSCROLL_DOWN;		/* M-C-N */
	Keys[M(16)] = ZSCROLL_UP;		/* M-C-P */
	Keys[M(19)] = ZINCREMENTAL_SEARCH;	/* M-C-S */
	Keys[M(22)] = ZVIEW_LINE;		/* M-C-V */
	Keys[M(27)] = ZABORT;			/* M-M */
	Keys[M(' ')] = ZSEARCH;
/*	[M('!')] = , */
	Keys[M('@')] = ZCMD_TO_BUFFER;
/*	[M('#')] = , */
	Keys[M('*')] = ZUNMODIFY;
	Keys[M('(')] = ZBEGINNING_OF_BUFFER;
	Keys[M(')')] = ZEND_OF_BUFFER;
/*	[M(',')] = , */
	Keys[M('<')] = ZBEGINNING_OF_BUFFER;
	Keys[M('>')] = ZEND_OF_BUFFER;
	Keys[M('.')] = ZTAG;
/*	[M('/')] = , */
/*	[M('_')] = , */
/*	[M('-')] = , */
	Keys[M('+')] = ZAPPEND_KILL;
	Keys[M('=')] = ZAPPEND_KILL;
	/* All lowercase chars converted to uppercase */
	Keys[M('A')] = ZAGAIN;
	Keys[M('B')] = ZPREVIOUS_WORD;
	Keys[M('C')] = ZCAPITALIZE_WORD;
	Keys[M('D')] = ZDELETE_WORD;
	Keys[M('E')] = ZRE_SEARCH;
	Keys[M('F')] = ZNEXT_WORD;
	Keys[M('G')] = ZGOTO_LINE;
	Keys[M('H')] = ZDELETE_PREVIOUS_WORD;
	Keys[M('I')] = ZTAB;
	Keys[M('J')] = ZJOIN;
	Keys[M('K')] = ZKILL;
	Keys[M('L')] = ZLOWERCASE_WORD;
	Keys[M('M')] = ZFILL_PARAGRAPH;
	Keys[M('N')] = ZNEXT_PARAGRAPH;
	Keys[M('O')] = ZREVERT_FILE;
	Keys[M('P')] = ZPREVIOUS_PARAGRAPH;
	Keys[M('Q')] = ZQUOTE;
	Keys[M('R')] = ZQUERY_REPLACE;
	Keys[M('S')] = ZSEARCH;
	Keys[M('T')] = ZSWAP_WORDS;
	Keys[M('U')] = ZUPPERCASE_WORD;
	Keys[M('V')] = ZPREVIOUS_PAGE;
	Keys[M('W')] = ZCOPY_REGION;
	Keys[M('X')] = ZMETA_X;
	Keys[M('Y')] = ZYANK;
	Keys[M('Z')] = ZSAVE_AND_EXIT;
	Keys[M(127)] = ZDELETE_PREVIOUS_WORD;	/* M-DEL */

	/* Special keys */

	Keys[TC_UP] = ZPREVIOUS_LINE;
	Keys[TC_DOWN] = ZNEXT_LINE;
	Keys[TC_LEFT] = ZPREVIOUS_CHAR;
	Keys[TC_RIGHT] = ZNEXT_CHAR;

	Keys[TC_PGDOWN] = ZNEXT_PAGE;
	Keys[TC_PGUP] = ZPREVIOUS_PAGE;
	Keys[TC_HOME] = ZBEGINNING_OF_LINE;
	Keys[TC_END] = ZEND_OF_LINE;
	Keys[TC_INSERT] = ZINSERT_OVERWRITE;
	Keys[TC_DELETE] = ZDELETE_CHAR;

	Keys[TC_F1] = ZFIND_FILE;
	Keys[TC_F2] = ZSEARCH;
	Keys[TC_F3] = ZAGAIN;
	Keys[TC_F4] = ZNEXT_ERROR;
	Keys[TC_F5] = ZRE_REPLACE;
	Keys[TC_F6] = ZHELP;
	Keys[TC_F7] = ZMAKE;
	Keys[TC_F8] = ZGREP;
	Keys[TC_F9] = ZWORD_SEARCH;
	Keys[TC_F10] = ZTAG_WORD;
	Keys[TC_F11] = ZNEXT_BOOKMARK;
	Keys[TC_F12] = ZREVERT_FILE;

	Keys[TC_C_HOME] = ZBEGINNING_OF_BUFFER;
	Keys[TC_C_END] = ZEND_OF_BUFFER;

	/* HACK. I wanted to keep this in DOS specific code. */
	install_ints();
}

static char *key_label[] = {
	"up", "down", "right", "left",
	"insert", "delete", "page up", "page down", "home", "end",
	"f1", "f2", "f3", "f4", "f5", "f6",
	"f7", "f8", "f9", "f10", "f11", "f12",
	"C-home", "C-end",
};

const char *special_label(int key)
{
	if (key >= SPECIAL_START && key <= SPECIAL_END)
		return key_label[key - SPECIAL_START];
	else
		return "???";
}
