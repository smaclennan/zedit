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

	/* Init the Meta functions */

	[128 + 7] = ZABORT,			/* M-C-G */
	[128 + 14] = ZSCROLL_DOWN,		/* M-C-N */
	[128 + 16] = ZSCROLL_UP,		/* M-C-P */
	[128 + 19] = ZINCREMENTAL_SEARCH,	/* M-C-S */
	[128 + 22] = ZVIEW_LINE,		/* M-C-V */
	[128 + 27] = ZABORT,			/* M-M */
	[128 + ' '] = ZSEARCH,
/*	[128 + '!'] = , */
	[128 + '@'] = ZCMD_TO_BUFFER,
/*	[128 + '#'] = , */
	[128 + '*'] = ZUNMODIFY,
	[128 + '('] = ZBEGINNING_OF_BUFFER,
	[128 + ')'] = ZEND_OF_BUFFER,
/*	[128 + ','] = , */
	[128 + '<'] = ZBEGINNING_OF_BUFFER,
	[128 + '>'] = ZEND_OF_BUFFER,
	[128 + '.'] = ZTAG,
/*	[128 + '/'] = , */
/*	[128 + '_'] = , */
/*	[128 + '-'] = , */
	[128 + '+'] = ZAPPEND_KILL,
	[128 + '='] = ZAPPEND_KILL,
	/* All lowercase chars converted to uppercase */
	[128 + 'A'] = ZAGAIN,
	[128 + 'B'] = ZPREVIOUS_WORD,
	[128 + 'C'] = ZCAPITALIZE_WORD,
	[128 + 'D'] = ZDELETE_WORD,
	[128 + 'E'] = ZRE_SEARCH,
	[128 + 'F'] = ZNEXT_WORD,
	[128 + 'G'] = ZGOTO_LINE,
	[128 + 'H'] = ZDELETE_PREVIOUS_WORD,
	[128 + 'I'] = ZTAB,
	[128 + 'J'] = ZJOIN,
	[128 + 'K'] = ZKILL,
	[128 + 'L'] = ZLOWERCASE_WORD,
	[128 + 'M'] = ZFILL_PARAGRAPH,
	[128 + 'N'] = ZNEXT_PARAGRAPH,
	[128 + 'O'] = ZREVERT_FILE,
	[128 + 'P'] = ZPREVIOUS_PARAGRAPH,
	[128 + 'Q'] = ZQUOTE,
	[128 + 'R'] = ZQUERY_REPLACE,
	[128 + 'S'] = ZSEARCH,
	[128 + 'T'] = ZSWAP_WORDS,
	[128 + 'U'] = ZUPPERCASE_WORD,
	[128 + 'V'] = ZPREVIOUS_PAGE,
	[128 + 'W'] = ZCOPY_REGION,
	[128 + 'X'] = ZMETA_X,
	[128 + 'Y'] = ZYANK,
	[128 + 'Z'] = ZSAVE_AND_EXIT,
	[128 + 127] = ZDELETE_PREVIOUS_WORD,	/* M-DEL */

	/* Init the CTRL-X functions */

	[256 +  1]  = ZSAVE_ALL_FILES,		/* C-X C-A */
	[256 +  2]  = ZSWITCH_TO_BUFFER,	/* C-X C-B */
	[256 +  3]  = ZEXIT,			/* C-X C-C */
	[256 +  4]  = ZDELETE_BUFFER,		/* C-X C-D */
/* C-X C-E */
	[256 +  6]  = ZFIND_FILE,		/* C-X C-F */
	[256 +  7]  = ZABORT,			/* C-X C-G */
/* C-X C-H */
/* C-X C-I */
/* C-X C-J */
	[256 + 11]  = ZDELETE_LINE,		/* C-X C-K */
	[256 + 12]  = ZLOWERCASE_REGION,	/* C-X C-L */
	[256 + 13]  = ZMAKE,			/* C-X C-M */
	[256 + 14]  = ZNEXT_ERROR,		/* C-X C-N */
	[256 + 15]  = ZOUT_TO,			/* C-X C-O */
	[256 + 16]  = ZMARK_PARAGRAPH,		/* C-X C-P */
/* C-X C-Q */
	[256 + 18]  = ZREAD_FILE,		/* C-X C-R */
	[256 + 19]  = ZSAVE_FILE,		/* C-X C-S */
/* C-X C-T */
	[256 + 21]  = ZUPPERCASE_REGION,	/* C-X C-U */
	[256 + 22]  = ZOTHER_NEXT_PAGE,		/* C-X C-V */
	[256 + 23]  = ZWRITE_FILE,		/* C-X C-W */
	[256 + 24]  = ZSWAP_MARK,		/* C-X C-X */
/* C-X C-Y */
	[256 + 26]  = ZOTHER_PREVIOUS_PAGE,	/* C-X C-Z */
	[256 + '='] = ZPOSITION,
	[256 + '1'] = ZONE_WINDOW,
	[256 + '2'] = ZSPLIT_WINDOW,
/* C-X ( */
/* C-X ) */
	/* All lowercase converted to uppercase */
	[256 + 'A'] = ZGLOBAL_SEARCH,
	[256 + 'B'] = ZNEXT_BOOKMARK,
	[256 + 'C'] = ZCOUNT,
/* C-X D */
	[256 + 'E'] = ZRE_REPLACE,
	[256 + 'F'] = ZNEXT_PARAGRAPH,
/* C-X G */
	[256 + 'H'] = ZHELP_FUNCTION,
	[256 + 'I'] = ZINDENT,
/* C-X J */
	[256 + 'K'] = ZDELETE_BUFFER,
	[256 + 'L'] = ZLIST_BUFFERS,
	[256 + 'M'] = ZSET_BOOKMARK,
	[256 + 'N'] = ZNEXT_WINDOW,
	[256 + 'O'] = ZNEXT_WINDOW,
/* C-X P */
/* C-X Q */
/* C-X R */
	[256 + 'S'] = ZSAVE_ALL_FILES,
	[256 + 'T'] = ZTRIM_WHITE_SPACE,
	[256 + 'U'] = ZUNDENT,
	[256 + 'V'] = ZSET_VARIABLE,
	[256 + 'W'] = ZWRITE_FILE,
	[256 + 'X'] = ZNEXT_BUFFER,
/* C-X Y */
	[256 + 'Z'] = ZEXIT,
	[256 + '^'] = ZGROW_WINDOW,

	/* Special keys */

	[TC_UP]	= ZPREVIOUS_LINE,
	[TC_DOWN] = ZNEXT_LINE,
	[TC_LEFT] = ZPREVIOUS_CHAR,
	[TC_RIGHT] = ZNEXT_CHAR,

	[TC_NPAGE] = ZNEXT_PAGE,
	[TC_PPAGE] = ZPREVIOUS_PAGE,
	[TC_HOME] = ZBEGINNING_OF_LINE,
	[TC_END] = ZEND_OF_LINE,
	[TC_INSERT] = ZINSERT_OVERWRITE,
	[TC_DELETE] = ZDELETE_CHAR,

	[TC_F1]	= ZFIND_FILE,
	[TC_F2]	= ZSEARCH,
	[TC_F3]	= ZAGAIN,
	[TC_F4]	= ZNEXT_ERROR,
	[TC_F5]	= ZRE_REPLACE,
	/* [TC_F6]	= , */
	[TC_F7]	= ZMAKE,
	[TC_F8]	= ZGREP,
	[TC_F9]	= ZWORD_SEARCH,
	[TC_F10] = ZTAG_WORD,
	[TC_F11] = ZNEXT_BOOKMARK,
	[TC_F12] = ZREVERT_FILE,
};
