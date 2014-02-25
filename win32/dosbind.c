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
};

void bind_init(void)
{

	/* Init the Meta functions */

	Keys[128 + 7] = ZABORT;			/* M-C-G */
	Keys[128 + 14] = ZSCROLL_DOWN;		/* M-C-N */
	Keys[128 + 16] = ZSCROLL_UP;		/* M-C-P */
	Keys[128 + 19] = ZINCREMENTAL_SEARCH;	/* M-C-S */
	Keys[128 + 22] = ZVIEW_LINE;		/* M-C-V */
	Keys[128 + 27] = ZABORT;			/* M-M */
	Keys[128 + ' '] = ZSEARCH;
/*	[128 + '!'] = , */
	Keys[128 + '@'] = ZCMD_TO_BUFFER;
/*	[128 + '#'] = , */
	Keys[128 + '*'] = ZUNMODIFY;
	Keys[128 + '('] = ZBEGINNING_OF_BUFFER;
	Keys[128 + ')'] = ZEND_OF_BUFFER;
/*	[128 + ','] = , */
	Keys[128 + '<'] = ZBEGINNING_OF_BUFFER;
	Keys[128 + '>'] = ZEND_OF_BUFFER;
	Keys[128 + '.'] = ZTAG;
/*	[128 + '/'] = , */
/*	[128 + '_'] = , */
/*	[128 + '-'] = , */
	Keys[128 + '+'] = ZAPPEND_KILL;
	Keys[128 + '='] = ZAPPEND_KILL;
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

	Keys[256 +  1]  = ZSAVE_ALL_FILES;		/* C-X C-A */
	Keys[256 +  2]  = ZSWITCH_TO_BUFFER;	/* C-X C-B */
	Keys[256 +  3]  = ZEXIT;			/* C-X C-C */
	Keys[256 +  4]  = ZDELETE_BUFFER;		/* C-X C-D */
	Keys[256 +  6]  = ZFIND_FILE;		/* C-X C-F */
	Keys[256 +  7]  = ZABORT;			/* C-X C-G */
	Keys[256 + 11]  = ZDELETE_LINE;		/* C-X C-K */
	Keys[256 + 12]  = ZLOWERCASE_REGION;	/* C-X C-L */
	Keys[256 + 13]  = ZMAKE;			/* C-X C-M */
	Keys[256 + 14]  = ZNEXT_ERROR;		/* C-X C-N */
	Keys[256 + 15]  = ZOUT_TO;			/* C-X C-O */
	Keys[256 + 16]  = ZMARK_PARAGRAPH;		/* C-X C-P */
	Keys[256 + 18]  = ZREAD_FILE;		/* C-X C-R */
	Keys[256 + 19]  = ZSAVE_FILE;		/* C-X C-S */
	Keys[256 + 21]  = ZUPPERCASE_REGION;	/* C-X C-U */
	Keys[256 + 22]  = ZOTHER_NEXT_PAGE;		/* C-X C-V */
	Keys[256 + 23]  = ZWRITE_FILE;		/* C-X C-W */
	Keys[256 + 24]  = ZSWAP_MARK;		/* C-X C-X */
	Keys[256 + 26]  = ZOTHER_PREVIOUS_PAGE;	/* C-X C-Z */
	Keys[256 + '='] = ZPOSITION;
	Keys[256 + '1'] = ZONE_WINDOW;
	Keys[256 + '2'] = ZSPLIT_WINDOW;
	/* All lowercase converted to uppercase */
	Keys[256 + 'A'] = ZGLOBAL_SEARCH;
	Keys[256 + 'B'] = ZNEXT_BOOKMARK;
	Keys[256 + 'C'] = ZCOUNT;
/*	[256 + 'D'] = , */
	Keys[256 + 'E'] = ZRE_REPLACE;
	Keys[256 + 'F'] = ZNEXT_PARAGRAPH;
/*	[256 + 'G'] = , */
	Keys[256 + 'H'] = ZHELP_FUNCTION;
	Keys[256 + 'I'] = ZINDENT;
/*	[256 + 'J'] = , */
	Keys[256 + 'K'] = ZDELETE_BUFFER;
	Keys[256 + 'L'] = ZLIST_BUFFERS;
	Keys[256 + 'M'] = ZSET_BOOKMARK;
	Keys[256 + 'N'] = ZNEXT_WINDOW;
	Keys[256 + 'O'] = ZNEXT_WINDOW;
/*	[256 + 'P'] = , */
/*	[256 + 'Q'] = , */
/*	[256 + 'R'] = , */
	Keys[256 + 'S'] = ZSAVE_ALL_FILES;
	Keys[256 + 'T'] = ZTRIM_WHITE_SPACE;
	Keys[256 + 'U'] = ZUNDENT;
	Keys[256 + 'V'] = ZSET_VARIABLE;
	Keys[256 + 'W'] = ZWRITE_FILE;
	Keys[256 + 'X'] = ZNEXT_BUFFER;
/*	[256 + 'Y'] = , */
	Keys[256 + 'Z'] = ZEXIT;
	Keys[256 + '^'] = ZGROW_WINDOW;

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
