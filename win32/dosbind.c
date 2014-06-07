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

Byte Keys[] = {
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
	ZHELP,			/* C-X C-H */
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
	/* makedosbind assumes this ends before C-X space */

	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	ZUNDO,			/* C-X / */
	0,
	ZONE_WINDOW,		/* C-X 1 */
	ZSPLIT_WINDOW,		/* C-X 2 */
	0,0,0,0,0,0,0,0,0,0,
	ZPOSITION,		/* C-X = */

	/* All lowercase converted to uppercase */
	0,0,0,
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
	ZZAP_TO_CHAR,		/* C-X Z */
	0,0,0,
	ZGROW_WINDOW,		/* C-X ^ */

	/* Special keys */

	0,0,
	ZPREVIOUS_LINE,		/* TC_UP */
	ZNEXT_LINE,		/* TC_DOWN */
	ZNEXT_CHAR,		/* TC_RIGHT */
	ZPREVIOUS_CHAR,		/* TC_LEFT */

	ZINSERT_OVERWRITE,	/* TC_INSERT */
	ZDELETE_CHAR,		/* TC_DELETE */
	ZPREVIOUS_PAGE,		/* TC_PGUP */
	ZNEXT_PAGE,		/* TC_PGDOWN */
	ZBEGINNING_OF_LINE,	/* TC_HOME */
	ZEND_OF_LINE,		/* TC_END */

	ZFIND_FILE,		/* TC_F1 */
	ZSEARCH,		/* TC_F2 */
	ZAGAIN,			/* TC_F3 */
	ZNEXT_ERROR,		/* TC_F4 */
	ZRE_REPLACE,		/* TC_F5 */
	ZHELP,			/* TC_F6 */
	ZMAKE,			/* TC_F7 */
	ZGREP,			/* TC_F8 */
	ZWORD_SEARCH,		/* TC_F9 */
	ZTAG_WORD,		/* TC_F10 */
	ZNEXT_BOOKMARK,		/* TC_F11 */
	ZREVERT_FILE,		/* TC_F12 */

	ZBEGINNING_OF_BUFFER,	/* TC_C_HOME */
	ZEND_OF_BUFFER,		/* TC_C_END */

	0,0,0,0,0,0,
	ZDELETE_PREVIOUS_WORD, /* C-X Backspace */

	/* Init the Meta functions */

	0,0,0,0,0,0,0,
	ZABORT,		/* M-C-G */
	0,0,0,0,0,0,
	ZSCROLL_DOWN,		/* M-C-N */
	0,
	ZSCROLL_UP,		/* M-C-P */
	0,0,
	ZINCREMENTAL_SEARCH,	/* M-C-S */
	0,0,
	ZVIEW_LINE,		/* M-C-V */
	0,0,0,0,
	ZABORT,		/* M-M */
	0,0,0,0,
	ZSEARCH,		/* M-  */
	0,0,
	ZCALC,			/* M-# */
	0,0,0,0,
	ZBEGINNING_OF_BUFFER,	/* M-( */
	ZEND_OF_BUFFER,		/* M-) */
	ZUNMODIFY,		/* M-* */
	ZAPPEND_KILL,		/* M-+ */
	0,0,
	ZTAG,			/* M-. */
	0,0,0,0,0,0,0,0,0,0,0,0,0,
	ZBEGINNING_OF_BUFFER,	/* M-< */
	ZAPPEND_KILL,		/* M-= */
	ZEND_OF_BUFFER,		/* M-> */
	0,
	ZCMD_TO_BUFFER,		/* M-@ */

	/* All lowercase chars converted to uppercase */
	ZAGAIN,			/* M-A */
	ZPREVIOUS_WORD,		/* M-B */
	ZCAPITALIZE_WORD,	/* M-C */
	ZDELETE_WORD,		/* M-D */
	ZRE_SEARCH,		/* M-E */
	ZNEXT_WORD,		/* M-F */
	ZGOTO_LINE,		/* M-G */
	ZDELETE_PREVIOUS_WORD,	/* M-H */
	ZTAB,			/* M-I */
	ZJOIN,			/* M-J */
	ZKILL,			/* M-K */
	ZLOWERCASE_WORD,	/* M-L */
	ZFILL_PARAGRAPH,	/* M-M */
	ZNEXT_PARAGRAPH,	/* M-N */
	ZREVERT_FILE,		/* M-O */
	ZPREVIOUS_PARAGRAPH,	/* M-P */
	ZQUOTE,			/* M-Q */
	ZQUERY_REPLACE,		/* M-R */
	ZSEARCH,		/* M-S */
	ZSWAP_WORDS,		/* M-T */
	ZUPPERCASE_WORD,	/* M-U */
	ZPREVIOUS_PAGE,		/* M-V */
	ZCOPY_REGION,		/* M-W */
	ZMETA_X,		/* M-X */
	ZYANK,			/* M-Y */
	ZSAVE_AND_EXIT,		/* M-Z */

	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	ZDELETE_PREVIOUS_WORD,	/* M-DEL */
};

static const char *key_label[] = {
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
