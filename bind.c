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

	[CX(1)]  = ZSAVE_ALL_FILES,		/* C-X C-A */
	[CX(2)]  = ZSWITCH_TO_BUFFER,	/* C-X C-B */
	[CX(3)]  = ZEXIT,			/* C-X C-C */
	[CX(4)]  = ZDELETE_BUFFER,		/* C-X C-D */
	[CX(6)]  = ZFIND_FILE,		/* C-X C-F */
	[CX(7)]  = ZABORT,			/* C-X C-G */
	[CX(11)]  = ZDELETE_LINE,		/* C-X C-K */
	[CX(12)]  = ZLOWERCASE_REGION,	/* C-X C-L */
	[CX(13)]  = ZMAKE,			/* C-X C-M */
	[CX(14)]  = ZNEXT_ERROR,		/* C-X C-N */
	[CX(15)]  = ZOUT_TO,			/* C-X C-O */
	[CX(16)]  = ZMARK_PARAGRAPH,		/* C-X C-P */
	[CX(18)]  = ZREAD_FILE,		/* C-X C-R */
	[CX(19)]  = ZSAVE_FILE,		/* C-X C-S */
	[CX(21)]  = ZUPPERCASE_REGION,	/* C-X C-U */
	[CX(22)]  = ZOTHER_NEXT_PAGE,		/* C-X C-V */
	[CX(23)]  = ZWRITE_FILE,		/* C-X C-W */
	[CX(24)]  = ZSWAP_MARK,		/* C-X C-X */
	[CX(26)]  = ZOTHER_PREVIOUS_PAGE,	/* C-X C-Z */
	[CX('=')] = ZPOSITION,
	[CX('1')] = ZONE_WINDOW,
	[CX('2')] = ZSPLIT_WINDOW,
	/* All lowercase converted to uppercase */
	[CX('A')] = ZGLOBAL_SEARCH,
	[CX('B')] = ZNEXT_BOOKMARK,
	[CX('C')] = ZCOUNT,
/*	[CX('D')] = , */
	[CX('E')] = ZRE_REPLACE,
	[CX('F')] = ZNEXT_PARAGRAPH,
/*	[CX('G')] = , */
	[CX('H')] = ZHELP_FUNCTION,
	[CX('I')] = ZINDENT,
/*	[CX('J')] = , */
	[CX('K')] = ZDELETE_BUFFER,
	[CX('L')] = ZLIST_BUFFERS,
	[CX('M')] = ZSET_BOOKMARK,
	[CX('N')] = ZNEXT_WINDOW,
	[CX('O')] = ZNEXT_WINDOW,
/*	[CX('P')] = , */
/*	[CX('Q')] = , */
/*	[CX('R')] = , */
	[CX('S')] = ZSAVE_ALL_FILES,
	[CX('T')] = ZTRIM_WHITE_SPACE,
	[CX('U')] = ZUNDENT,
	[CX('V')] = ZSET_VARIABLE,
	[CX('W')] = ZWRITE_FILE,
	[CX('X')] = ZNEXT_BUFFER,
/*	[CX('Y')] = , */
	[CX('Z')] = ZEXIT,
	[CX('^')] = ZGROW_WINDOW,

	/* Init the Meta functions */

	[M(7)] = ZABORT,			/* M-C-G */
	[M(14)] = ZSCROLL_DOWN,		/* M-C-N */
	[M(16)] = ZSCROLL_UP,		/* M-C-P */
	[M(19)] = ZINCREMENTAL_SEARCH,	/* M-C-S */
	[M(22)] = ZVIEW_LINE,		/* M-C-V */
	[M(27)] = ZABORT,			/* M-M */
	[M(' ')] = ZSEARCH,
/*	[M('!')] = , */
	[M('@')] = ZCMD_TO_BUFFER,
/*	[M('#')] = , */
	[M('*')] = ZUNMODIFY,
	[M('(')] = ZBEGINNING_OF_BUFFER,
	[M(')')] = ZEND_OF_BUFFER,
/*	[M(',')] = , */
	[M('<')] = ZBEGINNING_OF_BUFFER,
	[M('>')] = ZEND_OF_BUFFER,
	[M('.')] = ZTAG,
/*	[M('/')] = , */
/*	[M('_')] = , */
/*	[M('-')] = , */
	[M('+')] = ZAPPEND_KILL,
	[M('=')] = ZAPPEND_KILL,
	/* All lowercase chars converted to uppercase */
	[M('A')] = ZAGAIN,
	[M('B')] = ZPREVIOUS_WORD,
	[M('C')] = ZCAPITALIZE_WORD,
	[M('D')] = ZDELETE_WORD,
	[M('E')] = ZRE_SEARCH,
	[M('F')] = ZNEXT_WORD,
	[M('G')] = ZGOTO_LINE,
	[M('H')] = ZDELETE_PREVIOUS_WORD,
	[M('I')] = ZTAB,
	[M('J')] = ZJOIN,
	[M('K')] = ZKILL,
	[M('L')] = ZLOWERCASE_WORD,
	[M('M')] = ZFILL_PARAGRAPH,
	[M('N')] = ZNEXT_PARAGRAPH,
	[M('O')] = ZREVERT_FILE,
	[M('P')] = ZPREVIOUS_PARAGRAPH,
	[M('Q')] = ZQUOTE,
	[M('R')] = ZQUERY_REPLACE,
	[M('S')] = ZSEARCH,
	[M('T')] = ZSWAP_WORDS,
	[M('U')] = ZUPPERCASE_WORD,
	[M('V')] = ZPREVIOUS_PAGE,
	[M('W')] = ZCOPY_REGION,
	[M('X')] = ZMETA_X,
	[M('Y')] = ZYANK,
	[M('Z')] = ZSAVE_AND_EXIT,
	[M(127)] = ZDELETE_PREVIOUS_WORD,	/* M-DEL */

	/* Special keys */

	[TC_UP] = ZPREVIOUS_LINE,
	[TC_DOWN] = ZNEXT_LINE,
	[TC_LEFT] = ZPREVIOUS_CHAR,
	[TC_RIGHT] = ZNEXT_CHAR,

	[TC_PGDOWN] = ZNEXT_PAGE,
	[TC_PGUP] = ZPREVIOUS_PAGE,
	[TC_HOME] = ZBEGINNING_OF_LINE,
	[TC_END] = ZEND_OF_LINE,
	[TC_INSERT] = ZINSERT_OVERWRITE,
	[TC_DELETE] = ZDELETE_CHAR,

	[TC_F1] = ZFIND_FILE,
	[TC_F2] = ZSEARCH,
	[TC_F3] = ZAGAIN,
	[TC_F4] = ZNEXT_ERROR,
	[TC_F5] = ZRE_REPLACE,
	[TC_F6] = ZHELP,
	[TC_F7] = ZMAKE,
	[TC_F8] = ZGREP,
	[TC_F9] = ZWORD_SEARCH,
	[TC_F10] = ZTAG_WORD,
	[TC_F11] = ZNEXT_BOOKMARK,
	[TC_F12] = ZREVERT_FILE,

	[TC_C_HOME] = ZBEGINNING_OF_BUFFER,
	[TC_C_END] = ZEND_OF_BUFFER,
};

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
