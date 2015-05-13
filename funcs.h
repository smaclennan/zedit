/* funcs.h - Zedit function defines
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

#define ZNOTIMPL		0 /* Must be zero */
#define ZONE_WINDOW		(ZNOTIMPL + 1)
#define ZSPLIT_WINDOW		(ZONE_WINDOW + 1)
#define ZABORT			(ZSPLIT_WINDOW + 1)
#define ZAGAIN			(ZABORT + 1)
#define ZARG			(ZAGAIN + 1)
#define ZBEGINNING_OF_LINE	(ZARG + 1)
#define ZPREVIOUS_PARAGRAPH	(ZBEGINNING_OF_LINE + 1)
#define ZPREVIOUS_WORD		(ZPREVIOUS_PARAGRAPH + 1)
#define ZCALC			(ZPREVIOUS_WORD + 1)
#define ZCAPITALIZE_WORD	(ZCALC + 1)
#define ZTOGGLE_CASE		(ZCAPITALIZE_WORD + 1)
#define ZCENTER			(ZTOGGLE_CASE + 1)
#define ZCOPY_REGION		(ZCENTER + 1)
#define ZCTRL_X			(ZCOPY_REGION + 1)
#define ZDELETE_CHAR		(ZCTRL_X + 1)
#define ZDELETE_TO_EOL		(ZDELETE_CHAR + 1)
#define ZDELETE_LINE		(ZDELETE_TO_EOL + 1)
#define ZDELETE_REGION		(ZDELETE_LINE + 1)
#define ZTRIM_WHITE_SPACE	(ZDELETE_REGION + 1)
#define ZDELETE_WORD		(ZTRIM_WHITE_SPACE + 1)
#define ZEND_OF_LINE		(ZDELETE_WORD + 1)
#define ZEXIT			(ZEND_OF_LINE + 1)
#define ZSAVE_AND_EXIT		(ZEXIT + 1)
#define ZREAD_FILE		(ZSAVE_AND_EXIT + 1)
#define ZSAVE_FILE		(ZREAD_FILE + 1)
#define ZWRITE_FILE		(ZSAVE_FILE + 1)
#define ZMODE			(ZWRITE_FILE + 1)
#define ZFILL_PARAGRAPH		(ZMODE + 1)
#define ZFIND_FILE		(ZFILL_PARAGRAPH + 1)
#define ZNEXT_PARAGRAPH		(ZFIND_FILE + 1)
#define ZNEXT_WORD		(ZNEXT_PARAGRAPH + 1)
#define ZCOPY_WORD		(ZNEXT_WORD + 1)
#define ZGROW_WINDOW		(ZCOPY_WORD + 1)
#define ZINSERT			(ZGROW_WINDOW + 1) /* Also used for pinsert */
#define ZDELETE_BUFFER		(ZINSERT + 1)
#define ZGOTO_LINE		(ZDELETE_BUFFER + 1)
#define ZLOWERCASE_WORD		(ZGOTO_LINE + 1)
#define ZLIST_BUFFERS		(ZLOWERCASE_WORD + 1)
#define ZAPPEND_KILL		(ZLIST_BUFFERS + 1)
#define ZMETA			(ZAPPEND_KILL + 1)
#define ZNEXT_CHAR		(ZMETA + 1)
#define ZNEXT_LINE		(ZNEXT_CHAR + 1)
#define ZNEXT_PAGE		(ZNEXT_LINE + 1)
#define ZNEXT_BOOKMARK		(ZNEXT_PAGE + 1)
#define ZOTHER_NEXT_PAGE	(ZNEXT_BOOKMARK + 1)
#define ZOPEN_LINE		(ZOTHER_NEXT_PAGE + 1)
#define ZINSERT_OVERWRITE	(ZOPEN_LINE + 1)
#define ZPREVIOUS_CHAR		(ZINSERT_OVERWRITE + 1)
#define ZPREVIOUS_LINE		(ZPREVIOUS_CHAR + 1)
#define ZOTHER_PREVIOUS_PAGE	(ZPREVIOUS_LINE + 1)
#define ZPREVIOUS_PAGE		(ZOTHER_PREVIOUS_PAGE + 1)
#define ZPOSITION		(ZPREVIOUS_PAGE + 1)
#define ZQUERY_REPLACE		(ZPOSITION + 1)
#define ZQUOTE			(ZQUERY_REPLACE + 1)
#define ZDELETE_PREVIOUS_CHAR	(ZQUOTE + 1)
#define ZDELETE_PREVIOUS_WORD	(ZDELETE_PREVIOUS_CHAR + 1)
#define ZRE_REPLACE		(ZDELETE_PREVIOUS_WORD + 1)
#define ZRE_SEARCH		(ZRE_REPLACE + 1)
#define ZREVERSE_SEARCH		(ZRE_SEARCH + 1)
#define ZSEARCH			(ZREVERSE_SEARCH + 1)
#define ZSET_VARIABLE		(ZSEARCH + 1)
#define ZSET_BOOKMARK		(ZSET_VARIABLE + 1)
#define ZSET_MARK		(ZSET_BOOKMARK + 1)
#define ZSWAP_CHARS		(ZSET_MARK + 1)
#define ZSWAP_MARK		(ZSWAP_CHARS + 1)
#define ZNEXT_WINDOW		(ZSWAP_MARK + 1)
#define ZSWAP_WORDS		(ZNEXT_WINDOW + 1)
#define ZSWITCH_TO_BUFFER	(ZSWAP_WORDS + 1)
#define ZTAB			(ZSWITCH_TO_BUFFER + 1)
#define ZEND_OF_BUFFER		(ZTAB + 1)
#define ZBEGINNING_OF_BUFFER	(ZEND_OF_BUFFER + 1)
#define ZUPPERCASE_WORD		(ZBEGINNING_OF_BUFFER + 1)
#define ZYANK			(ZUPPERCASE_WORD + 1)
#define ZC_INDENT		(ZYANK + 1)
#define ZC_INSERT		(ZC_INDENT + 1)
#define ZFILL_CHECK		(ZC_INSERT + 1)
#define ZNEWLINE		(ZFILL_CHECK + 1)
#define ZREDISPLAY		(ZNEWLINE + 1)
#define ZMETA_X			(ZREDISPLAY + 1)
#define ZNEXT_BUFFER		(ZMETA_X + 1)
#define ZHELP_KEY		(ZNEXT_BUFFER + 1)
#define ZCOUNT			(ZHELP_KEY + 1)
#define ZINCREMENTAL_SEARCH	(ZCOUNT + 1)
#define ZUNMODIFY		(ZINCREMENTAL_SEARCH + 1)
#define ZDELETE_BLANKS		(ZUNMODIFY + 1)
#define ZMARK_PARAGRAPH		(ZDELETE_BLANKS + 1)
#define ZUPPERCASE_REGION	(ZMARK_PARAGRAPH + 1)
#define ZLOWERCASE_REGION	(ZUPPERCASE_REGION + 1)
#define ZJOIN			(ZLOWERCASE_REGION + 1)
#define ZOUT_TO			(ZJOIN + 1)
#define ZMAKE			(ZOUT_TO + 1)
#define ZNEXT_ERROR		(ZMAKE + 1)
#define ZCMD_TO_BUFFER		(ZNEXT_ERROR + 1)
#define ZKILL			(ZCMD_TO_BUFFER + 1)
#define ZINDENT			(ZKILL + 1)
#define ZUNDENT			(ZINDENT + 1)
#define ZEMPTY_BUFFER		(ZUNDENT + 1)
#define ZREPLACE		(ZEMPTY_BUFFER + 1)
#define ZSAVE_ALL_FILES		(ZREPLACE + 1)
#define ZSIZE_WINDOW		(ZSAVE_ALL_FILES + 1)
#define ZREVERT_FILE		(ZSIZE_WINDOW + 1)
#define ZSCROLL_DOWN		(ZREVERT_FILE + 1)
#define ZSCROLL_UP		(ZSCROLL_DOWN + 1)
#define ZSHOW_CONFIG		(ZSCROLL_UP + 1)
#define ZSETENV			(ZSHOW_CONFIG + 1)
#define ZGREP			(ZSETENV + 1)
#define ZUNDO			(ZGREP + 1)
#define ZHELP_VARIABLE		(ZUNDO + 1)
#define ZHELP_FUNCTION		(ZHELP_VARIABLE + 1)
#define ZHELP_APROPOS		(ZHELP_FUNCTION + 1)
#define ZSTATS			(ZHELP_APROPOS + 1)
#define ZWORD_SEARCH		(ZSTATS + 1)
#define ZSPELL_WORD		(ZWORD_SEARCH + 1)
#define ZTAG			(ZSPELL_WORD + 1)
#define ZTAG_WORD		(ZTAG + 1)
#define ZHELP			(ZTAG_WORD + 1)
#define ZZAP_TO_CHAR		(ZHELP + 1)
#define NUMFUNCS		(ZZAP_TO_CHAR + 1)

/* this is used by the getfname command in the PAW for command completion */
#define ZFNAME		NUMFUNCS
