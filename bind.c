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

Byte Lfunc;

#ifdef __cplusplus
#include "win32/dosbind.c"
#else
Byte Keys[] = {
	ZSET_MARK,				/* C-@ */
	ZBEGINNING_OF_LINE,		/* C-A */
	ZPREVIOUS_CHAR,			/* C-B */
	ZCOPY_WORD,				/* C-C */
	ZDELETE_CHAR,			/* C-D */
	ZEND_OF_LINE,			/* C-E */
	ZNEXT_CHAR,				/* C-F */
	ZABORT,					/* C-G */
	ZDELETE_PREVIOUS_CHAR,	/* C-H */
	ZTAB,					/* C-I */
	ZC_INDENT,				/* C-J */
	ZDELETE_TO_EOL,			/* C-K */
	ZREDISPLAY,				/* C-L */
	ZNEWLINE,				/* C-M */
	ZNEXT_LINE,				/* C-N */
	ZOPEN_LINE,				/* C-O */
	ZPREVIOUS_LINE,			/* C-P */
	ZQUOTE,					/* C-Q */
	ZREVERSE_SEARCH,		/* C-R */
	ZINCREMENTAL_SEARCH,	/* C-S */
	ZSWAP_CHARS,			/* C-T */
	ZARG,					/* C-U */
	ZNEXT_PAGE,				/* C-V */
	ZDELETE_REGION,			/* C-W */
	ZCTRL_X,				/* C-X */
	ZYANK,					/* C-Y */
	ZPREVIOUS_PAGE,			/* C-Z */
	ZMETA,					/* ESC */
	ZSET_MARK,				/* C-\ */
	ZINSERT_OVERWRITE,		/* C-] */
	ZDELETE_BLANKS,			/* C-^ */
	ZUNDO,					/* C-_ */

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

	0,						/* C-X C-@ */
	ZSAVE_ALL_FILES,		/* C-X C-A */
	ZSWITCH_TO_BUFFER,		/* C-X C-B */
	ZEXIT,					/* C-X C-C */
	ZDELETE_BUFFER,			/* C-X C-D */
	0,						/* C-X C-E */
	ZFIND_FILE,				/* C-X C-F */
	ZABORT,					/* C-X C-G */
	ZHELP_KEY,				/* C-X C-H */
	0,						/* C-X C-I */
	0,						/* C-X C-J */
	ZDELETE_LINE,			/* C-X C-K */
	ZLOWERCASE_REGION,		/* C-X C-L */
	ZMAKE,					/* C-X C-M */
	ZNEXT_ERROR,			/* C-X C-N */
	ZOUT_TO,				/* C-X C-O */
	ZMARK_PARAGRAPH,		/* C-X C-P */
	0,						/* C-X C-Q */
	ZREAD_FILE,				/* C-X C-R */
	ZSAVE_FILE,				/* C-X C-S */
	0,						/* C-X C-T */
	ZUPPERCASE_REGION,		/* C-X C-U */
	ZOTHER_NEXT_PAGE,		/* C-X C-V */
	ZWRITE_FILE,			/* C-X C-W */
	ZSWAP_MARK,				/* C-X C-X */
	0,						/* C-X C-Y */
	ZOTHER_PREVIOUS_PAGE,	/* C-X C-Z */
	0,						/* C-X Esc */
	0,						/* C-X C-\ */
	0,						/* C-X C-] */
	ZGROW_WINDOW,			/* C-X C-^ */
	0,						/* C-X C-_ */
	/* makedosbind assumes this ends before C-X space */

	[CX('+')] = ZSTATS,
	[CX('/')] = ZUNDO,
	[CX('1')] = ZONE_WINDOW,
	[CX('2')] = ZSPLIT_WINDOW,
	[CX('=')] = ZPOSITION,

	/* All lowercase converted to uppercase */
	[CX('A')] = 0,
	[CX('B')] = ZNEXT_BOOKMARK,
	[CX('C')] = ZCOUNT,
	[CX('D')] = 0,
	[CX('E')] = ZRE_REPLACE,
	[CX('F')] = ZNEXT_PARAGRAPH,
	[CX('G')] = 0,
	[CX('H')] = ZHELP,
	[CX('I')] = ZINDENT,
	[CX('J')] = 0,
	[CX('K')] = ZDELETE_BUFFER,
	[CX('L')] = ZLIST_BUFFERS,
	[CX('M')] = ZSET_BOOKMARK,
	[CX('N')] = ZNEXT_WINDOW,
	[CX('O')] = ZNEXT_WINDOW,
	[CX('P')] = 0,
	[CX('Q')] = ZQUOTE,
	[CX('R')] = 0,
	[CX('S')] = ZSAVE_ALL_FILES,
	[CX('T')] = ZTRIM_WHITE_SPACE,
	[CX('U')] = ZUNDENT,
	[CX('V')] = ZSET_VARIABLE,
	[CX('W')] = ZWRITE_FILE,
	[CX('X')] = ZNEXT_BUFFER,
	[CX('Y')] = 0,
	[CX('Z')] = ZZAP_TO_CHAR,
	[CX('^')] = ZGROW_WINDOW,

	/* Special keys */

	[TC_UP] = ZPREVIOUS_LINE,
	[TC_DOWN] = ZNEXT_LINE,
	[TC_RIGHT] = ZNEXT_CHAR,
	[TC_LEFT] = ZPREVIOUS_CHAR,

	[TC_INSERT] = ZINSERT_OVERWRITE,
	[TC_DELETE] = ZDELETE_CHAR,
	[TC_PGUP] = ZPREVIOUS_PAGE,
	[TC_PGDOWN] = ZNEXT_PAGE,
	[TC_HOME] = ZBEGINNING_OF_LINE,
	[TC_END] = ZEND_OF_LINE,

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

	[CX(127)] = ZDELETE_PREVIOUS_WORD, /* C-X Backspace */

	/* Init the Meta functions */

	[M(7)] = ZABORT,		/* M-C-G */
	[M(14)] = ZSCROLL_DOWN,		/* M-C-N */
	[M(16)] = ZSCROLL_UP,		/* M-C-P */
	[M(19)] = ZINCREMENTAL_SEARCH,	/* M-C-S */
	[M(27)] = ZABORT,		/* M-M */
	[M(' ')] = ZSEARCH,
	[M('#')] = ZCALC,
	[M('$')] = ZSPELL_WORD,
	[M('(')] = ZBEGINNING_OF_BUFFER,
	[M(')')] = ZEND_OF_BUFFER,
	[M('*')] = ZUNMODIFY,
	[M('+')] = ZAPPEND_KILL,
	[M('.')] = ZTAG,
	[M('<')] = ZBEGINNING_OF_BUFFER,
	[M('=')] = ZAPPEND_KILL,
	[M('>')] = ZEND_OF_BUFFER,
	[M('@')] = ZCMD_TO_BUFFER,

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
	[M('Q')] = ZFILL_PARAGRAPH,
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
};
#endif

