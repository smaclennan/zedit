/* funcs.c - func lists
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

void (*Cmds[][2])() = {
	{ Znotimpl,			Znotimpl },
	{ Zone_window,			Znotimpl },
	{ Zsplit_window,		Znotimpl },
	{ Zabort,			Zabort },
	{ Zagain,			Zpart },
	{ Zarg,				Znotimpl },
	{ Zbeginning_of_line,		Zbeginning_of_line },
	{ Zprevious_paragraph,		Zprevious_paragraph },
	{ Zprevious_word,		Zprevious_word },
	{ Zcalc,			Znotimpl },
	{ Zcapitalize_word,		Zcapitalize_word },
	{ Ztoggle_case,			Ztoggle_case },
	{ Zcenter,			Znotimpl },
	{ Zcopy_region,			Zcopy_region },
	{ Zctrl_x,			Zctrl_x },
	{ Zdelete_char,			Zdelete_char },
	{ Zdelete_to_eol,		Zdelete_to_eol },
	{ Zdelete_line,			Zdelete_line },
	{ Zdelete_region,		Zdelete_region },
	{ Ztrim_white_space,		Ztrim_white_space },
	{ Zdelete_word,			Zdelete_word },
	{ Zend_of_line,			Zend_of_line },
	{ Zexit,			Znotimpl },
	{ Zsave_and_exit,		Znotimpl },
	{ Zread_file,			Znotimpl },
	{ Zsave_file,			Znotimpl },
	{ Zwrite_file,			Znotimpl },
	{ Zmode,			Znotimpl },
	{ Zfill_paragraph,		Znotimpl },
	{ Zfind_file,			Znotimpl },
	{ Znext_paragraph,		Znext_paragraph },
	{ Znext_word,			Znext_word },
	{ Zcopy_word,			Zcopy_word },
	{ Zgrow_window,			Znotimpl },
	{ Zinsert,			pinsert },
	{ Zdelete_buffer,		Znotimpl },
	{ Zgoto_line,			Znotimpl },
	{ Zlowercase_word,		Zlowercase_word },
	{ Zlist_buffers,		Znotimpl },
	{ Zappend_kill,			Zappend_kill },
	{ Zmeta,			Zmeta },
	{ Znext_char,			Znext_char },
	{ Znext_line,			Znext_line },
	{ Znext_page,			Znotimpl },
	{ Znext_bookmark,		Znotimpl },
	{ Zother_next_page,		Znotimpl },
	{ Zopen_line,			Znotimpl },
	{ Zinsert_overwrite,		Zinsert_overwrite },
	{ Zprevious_char,		Zprevious_char },
	{ Zprevious_line,		Zprevious_line },
	{ Zother_previous_page,		Znotimpl },
	{ Zprevious_page,		Znotimpl },
	{ Zposition,			Znotimpl },
	{ Zquery_replace,		Znotimpl },
	{ Zquote,			Zquote },
	{ Zdelete_previous_char,	Zdelete_previous_char },
	{ Zdelete_previous_word,	Zdelete_previous_word },
	{ Zre_replace,			Znotimpl },
	{ Zre_search,			Znotimpl },
	{ Zreverse_search,		Znotimpl },
	{ Zsearch,			Znotimpl },
	{ Zset_variable,		Znotimpl },
	{ Zset_bookmark,		Znotimpl },
	{ Zset_mark,			Zset_mark },
	{ Zswap_chars,			Zswap_chars },
	{ Zswap_mark,			Zswap_mark },
	{ Znext_window,			Znotimpl },
	{ Zswap_words,			Zswap_words },
	{ Zswitch_to_buffer,		Znotimpl },
	{ Ztab,				pinsert },
	{ Zend_of_buffer,		Zend_of_buffer },
	{ Zbeginning_of_buffer,		Zbeginning_of_buffer },
	{ Zuppercase_word,		Zuppercase_word },
	{ Zyank,			Zyank },
	{ Zc_indent,			pnewline },
	{ Zc_insert,			pinsert },
	{ Zfill_check,			Zfill_check },
	{ Znewline,			pnewline },
	{ Zredisplay,			Znotimpl },
	{ Zmeta_x,			Znotimpl },
	{ Znext_buffer,			Znotimpl },
	{ Zhelp_key,			Znotimpl },
	{ Zcount,			Znotimpl },
	{ Zincremental_search,		Znotimpl },
	{ Zunmodify,			Znotimpl },
	{ Zdelete_blanks,		Znotimpl },
	{ Zmark_paragraph,		Znotimpl },
	{ Zuppercase_region,		Zuppercase_region },
	{ Zlowercase_region,		Zlowercase_region },
	{ Zjoin,			Znotimpl },
	{ Zout_to,			Znotimpl },
	{ Zmake,			Znotimpl },
	{ Znext_error,			Znotimpl },
	{ Zcmd_to_buffer,		Znotimpl },
	{ Zkill,			Znotimpl },
	{ Zglobal_search,		Znotimpl },
	{ Zindent,			Znotimpl },
	{ Zundent,			Znotimpl },
	{ Zempty_buffer,		Zempty_buffer },
	{ Zreplace,			Znotimpl },
	{ Zsave_all_files,		Znotimpl },
	{ Zsize_window,			Znotimpl },
	{ Zrevert_file,			Znotimpl },
	{ Zscroll_down,			Znotimpl },
	{ Zscroll_up,			Znotimpl },
	{ Zshow_config,			Znotimpl },
	{ Zsetenv,			Znotimpl },
	{ Zglobal_re_search,		Znotimpl },
	{ Zgrep,			Znotimpl },
	{ Zundo,			Znotimpl },
	{ Zhelp_variable,		Znotimpl },
	{ Zhelp_function,		Znotimpl },
	{ Zhelp_apropos,		Znotimpl },
	{ Zstats,			Znotimpl },
	{ Zword_search,			Znotimpl },
	{ Zspell_word,			Znotimpl },
	{ Ztag,				Znotimpl },
	{ Ztag_word,			Znotimpl },
	{ Zhelp,			Znotimpl },
	{ Zzap_to_char,			Zzap_to_char },
	/* only in the PAW you say? pity... */
	{ Znotimpl,			Zfname },
};
int Curcmds;
