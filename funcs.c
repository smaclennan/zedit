/* funcs.c - func lists
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

struct cnames Cnames[] = {
	{"abort",			ZABORT,		H_MISC},
	{"again",			ZAGAIN,		H_SEARCH},
	{"append-kill",			ZMAKEDEL,	H_DELETE},
	{"beginning-of-buffer",		ZTOSTART,	H_CURSOR},
	{"beginning-of-line",		ZBEGLINE,	H_CURSOR},
	{"beginning-of-window",		ZBEGWIND,	H_CURSOR},
	{"bind",			ZBIND,		H_BIND},
	{"bound-to",			ZCMDBIND,	H_BIND},
	{"c-beautify",			ZBEAUTY,	H_SHELL},
	{"c-indent",			ZCINDENT,	H_MODE},
	{"c-Insert",			ZCINSERT,	H_NONE},
	{"c-x",				ZCTRLX,		H_NONE},
	{"calculator",			ZCALC,		H_HELP},
	{"capitalize-word",		ZCAPWORD,	H_DISP},
	{"center-line",			ZCENTER,	H_DISP},
	{"cmd-to-buffer",		ZCMDTOBUFF,	H_SHELL},
	{"cmd-to-screen",		ZCMD,		H_SHELL},
	{"copy-region",			ZCOPYRGN,	H_DELETE},
	{"copy-word",			ZGETBWORD,	H_DELETE},
	{"count",			ZCOUNT,		H_HELP},
	{"cwd",				ZCWD,		H_MISC},
	{"date",			ZDATE,		H_HELP},
	{"delete-blanks",		ZDELBLANKS,	H_DELETE},
	{"delete-buffer",		ZKILLBUFF,	H_BUFF},
	{"delete-character",		ZDELCHAR,	H_DELETE},
	{"delete-line",			ZDELLINE,	H_DELETE},
	{"delete-previous-character",	ZRDELCHAR,	H_DELETE},
	{"delete-previous-word",	ZRDELWORD,	H_DELETE},
	{"delete-region",		ZDELRGN,	H_DELETE},
	{"delete-to-eol",		ZDELEOL,	H_DELETE},
	{"delete-window",		ZDELWIND,	H_BUFF},
	{"delete-word",			ZDELWORD,	H_DELETE},
	{"display-bindings",		ZDISPBINDS,	H_BIND},
	{"empty-buffer",		ZEMPTY,		H_BUFF},
	{"end-of-buffer",		ZTOEND,		H_CURSOR},
	{"end-of-line",			ZENDLINE,	H_CURSOR},
	{"end-of-window",		ZENDWIND,	H_CURSOR},
	{"exit",			ZEXIT,		H_MISC},
	{"expression-replace",		ZREREPLACE,	H_SEARCH},
	{"expression-search",		ZRESRCH,	H_SEARCH},
	{"extended-command",		ZMETAX,		H_MISC},
	{"fill-check",			ZFILLCHK,	H_NONE},
	{"fill-paragraph",		ZFILLPARA,	H_MODE},
	{"find-file",			ZFINDFILE,	H_FILE},
	{"find-tag",			ZFINDTAG,	H_FILE},
	{"form-tab",			ZFORMTAB,	H_MODE},
	{"global-re-search",		ZGRESRCH,	H_SEARCH},
	{"global-search",		ZGSEARCH,	H_SEARCH},
	{"goto-line",			ZLGOTO,		H_CURSOR},
	{"grep",			ZGREP,		H_SHELL},
	{"grow-window",			ZGROWWINDOW,	H_BUFF},
	{"help",			ZHELP,		H_HELP},
	{"hex-output",			ZHEXOUT,	H_HELP},
	{"incremental-search",		ZINCSRCH,	H_SEARCH},
	{"indent",			ZINDENT,	H_DISP},
	{"insert-character",		ZINSERT,	H_NONE},
	{"insert-overwrite",		ZOVERIN,	H_MODE},
	{"insert-space",		ZISPACE,	H_DISP},
	{"join",			ZJOIN,		H_DELETE},
	{"key-binding",			ZKEYBIND,	H_BIND},
	{"kill",			ZKILL,		H_SHELL},
	{"list-buffers",		ZLSTBUFF,	H_BUFF},
	{"lowercase-region",		ZLOWREGION,	H_DISP},
	{"lowercase-word",		ZLOWWORD,	H_DISP},
	{"mail",			ZMAIL,		H_SHELL},
	{"make",			ZMAKE,		H_SHELL},
	{"man",				ZMAN,		H_SHELL},
	{"mark-paragraph",		ZMRKPARA,	H_DELETE},
	{"meta",			ZMETA,		H_NONE},
	{"mode",			ZMODE,		H_MODE},
	{"newline",			ZNEWLINE,	H_NONE},
	{"next-bookmark",		ZNXTBOOKMRK,	H_CURSOR},
	{"next-buffer",			ZNEXTBUFF,	H_BUFF},
	{"next-character",		ZNEXTCHAR,	H_CURSOR},
	{"next-error",			ZNEXTERR,	H_SHELL},
	{"next-line",			ZNEXTLINE,	H_CURSOR},
	{"next-page",			ZNEXTPAGE,	H_CURSOR},
	{"next-paragraph",		ZFPARA,		H_CURSOR},
	{"next-window",			ZNEXTWIND,	H_DISP},
	{"next-word",			ZFWORD,		H_CURSOR},
	{"null",			ZNOTIMPL,	H_BIND},
	{"one-window",			Z1WIND,		H_BUFF},
	{"open-line",			ZOPENLINE,	H_DISP},
	{"other-next-page",		ZNXTOTHRWIND,	H_BUFF},
	{"other-previous-page",		ZPREVOTHRWIND,	H_BUFF},
	{"out-to",			ZCGOTO,		H_CURSOR},
	{"position",			ZPRINTPOS,	H_HELP},
	{"previous-character",		ZPREVCHAR,	H_CURSOR},
	{"previous-line",		ZPREVLINE,	H_CURSOR},
	{"previous-page",		ZPREVPAGE,	H_CURSOR},
	{"previous-paragraph",		ZBPARA,		H_CURSOR},
	{"previous-window",		ZPREVWIND,	H_DISP},
	{"previous-word",		ZBWORD,		H_CURSOR},
	{"print",			ZPRINT,		H_SHELL},
	{"query-replace",		ZQUERY,		H_SEARCH},
	{"quit",			ZQUIT,		H_MISC},
	{"quote",			ZQUOTE,		H_MISC},
	{"read-file",			ZFILEREAD,	H_FILE},
	{"redisplay",			ZREDISPLAY,	H_DISP},
	{"reference",			ZREF,		H_FILE},
	{"replace",			ZREPLACE,	H_SEARCH},
	{"reverse-inc-search",		ZRINCSRCH,	H_SEARCH},
	{"reverse-search",		ZRSEARCH,	H_SEARCH},
	{"revert-file",			ZREVERTFILE,	H_FILE},
	{"save-all-files",		ZSAVEALL,	H_FILE},
	{"save-bindings",		ZSAVEBIND,	H_BIND},
	{"save-config",			ZSAVECONFIG,	H_HELP},
	{"save-file",			ZFILESAVE,	H_FILE},
	{"scroll-down",			ZSCROLLDOWN,	H_DISP},
	{"scroll-up",			ZSCROLLUP,	H_DISP},
	{"search",			ZSEARCH,	H_SEARCH},
	{"set-bookmark",		ZSETBOOKMRK,	H_CURSOR},
	{"set-mark",			ZSETMRK,	H_DELETE},
	{"set-variable",		ZSETAVAR,	H_HELP},
	{"setenv",			ZSETENV,	H_MISC},
	{"shell",			ZSHELL,		H_SHELL},
	{"shrink-window",		ZSHRINKWIND,	H_BUFF},
	{"size-window",			ZSIZEWIND,	H_BUFF},
	{"spell",			ZSPELL,		H_SHELL},
	{"stats",			ZSTAT,		H_HELP},
	{"swap-mark-and-point",		ZSWAPMRK,	H_CURSOR},
	{"switch-to-buffer",		ZSWITCHTO,	H_BUFF},
	{"tab",				ZTAB,		H_NONE},
	{"toggle-case",			ZCASE,		H_SEARCH},
	{"transpose-characters",	ZSWAPCHAR,	H_DISP},
	{"transpose-words",		ZSWAPWORD,	H_DISP},
	{"trim-white-space",		ZDELWHITE,	H_DELETE},
	{"two-windows",			Z2WIND,		H_BUFF},
	{"undent",			ZUNDENT,	H_DISP},
	{"undo",			ZUNDO,		H_MISC},
	{"universal-argument",		ZARG,		H_MISC},
	{"unmodify",			ZUNMODF,	H_DELETE},
	{"uppercase-region",		ZUPREGION,	H_DISP},
	{"uppercase-word",		ZUPWORD,	H_DISP},
	{"view-file",			ZVIEWFILE,	H_FILE},
	{"view-line",			ZVIEWLINE,	H_DISP},
	{"write-file",			ZFILEWRITE,	H_FILE},
	{"yank",			ZYANK,		H_DELETE},
	{"zoom",			ZZOOM,		H_DISP},
};

void (*Cmds[NUMFUNCS + 1][3])() = {
	{ Z1wind,		Z1wind,			Znotimpl },
	{ Z2wind,		Z2wind,			Znotimpl },
	{ Zabort,		Zabort,			Zabort },
	{ Zagain,		Zagain,			Zpart },
	{ Zarg,			Zarg,			Znotimpl },
	{ Zbegline,		Zbegline,		Zbegline },
	{ Zbpara,		Zbpara,			Zbpara },
	{ Zbword,		Zbword,			Zbword },
	{ Zcalc,		Zcalc,			Znotimpl },
	{ Zcapword,		Znotimpl,		Zcapword },
	{ Zcase,		Zcase,			Zcase },
	{ Zcenter,		Znotimpl,		Znotimpl },
	{ Zcopyrgn,		Zcopyrgn,		Zcopyrgn },
	{ Zctrlx,		Zctrlx,			Zctrlx },
	{ Zdelchar,		Znotimpl,		Zdelchar },
	{ Zdeleol,		Znotimpl,		Zdeleol },
	{ Zdelline,		Znotimpl,		Zdelline },
	{ Zdelrgn,		Znotimpl,		Zdelrgn },
	{ Zdelwhite,		Znotimpl,		Zdelwhite },
	{ Zdelword,		Znotimpl,		Zdelword },
	{ Zendline,		Zendline,		Zendline },
	{ Zexit,		Zexit,			Znotimpl },
	{ Zfileread,		Znotimpl,		Znotimpl },
	{ Zfilesave,		Znotimpl,		Znotimpl },
	{ Zfilewrite,		Zfilewrite,		Znotimpl },
	{ Zmode,		Zmode,			Znotimpl },
	{ Zfillpara,		Znotimpl,		Znotimpl },
	{ Zfindfile,		Zfindfile,		Znotimpl },
	{ Zfindtag,		Zfindtag,		Znotimpl },
	{ Zfpara,		Zfpara,			Zfpara },
	{ Zfword,		Zfword,			Zfword },
	{ Zgetbword,		Zgetbword,		Zgetbword },
	{ Zgrowwind,		Zgrowwind,		Znotimpl },
	{ Zhexout,		Zhexout,		Znotimpl },
	{ Zinsert,		Znotimpl,		pinsert },
	{ Zkillbuff,		Zkillbuff,		Znotimpl },
	{ Zlgoto,		Zlgoto,			Znotimpl },
	{ Zlowword,		Znotimpl,		Zlowword },
	{ Zlstbuff,		Zlstbuff,		Znotimpl },
	{ Zmakedel,		Zmakedel,		Zmakedel },
	{ Zmeta,		Zmeta,			Zmeta },
	{ Znextchar,		Znextchar,		Znextchar },
	{ Znextline,		Znextline,		Znextline },
	{ Znextpage,		Znextpage,		Znotimpl },
	{ Znotimpl,		Znotimpl,		Znotimpl },
	{ Znxtbookmrk,		Znxtbookmrk,		Znotimpl },
	{ Znxtothrwind,		Znxtothrwind,		Znotimpl },
	{ Zquit,		Zquit,			Znotimpl },
	{ Zopenline,		Znotimpl,		Znotimpl },
	{ Zoverin,		Znotimpl,		Zoverin },
	{ Zprevchar,		Zprevchar,		Zprevchar },
	{ Zprevline,		Zprevline,		Zprevline },
	{ Zprevothrwind,	Zprevothrwind,		Znotimpl },
	{ Zprevpage,		Zprevpage,		Znotimpl },
	{ Zprintpos,		Zprintpos,		Znotimpl },
	{ Zquery,		Znotimpl,		Znotimpl },
	{ Zquote,		Znotimpl,		Zquote },
	{ Zrdelchar,		Znotimpl,		Zrdelchar },
	{ Zrdelword,		Znotimpl,		Zrdelword },
	{ Zrereplace,		Znotimpl,		Znotimpl },
	{ Zresrch,		Zresrch,		Znotimpl },
	{ Zrsearch,		Zrsearch,		Znotimpl },
	{ Zsearch,		Zsearch,		Znotimpl },
	{ Zsetavar,		Zsetavar,		Znotimpl },
	{ Zsetbookmrk,		Zsetbookmrk,		Znotimpl },
	{ Zsetmrk,		Zsetmrk,		Zsetmrk },
	{ Zshell,		Zshell,			Znotimpl },
	{ Zstat,		Zstat,			Znotimpl },
	{ Zswapchar,		Znotimpl,		Zswapchar },
	{ Zswapmrk,		Zswapmrk,		Zswapmrk },
	{ Znextwind,		Znextwind,		Znotimpl },
	{ Zswapword,		Znotimpl,		Zswapword },
	{ Zswitchto,		Zswitchto,		Znotimpl },
	{ Ztab,			Znotimpl,		pinsert },
	{ Ztoend,		Ztoend,			Ztoend },
	{ Ztostart,		Ztostart,		Ztostart },
	{ Zupword,		Znotimpl,		Zupword },
	{ Zyank,		Znotimpl,		Zyank },
	{ Zcindent,		Znotimpl,		pnewline },
	{ Zcinsert,		Znotimpl,		pinsert },
	{ Zfillchk,		Znotimpl,		Zfillchk },
	{ Znewline,		Znotimpl,		pnewline },
	{ Zredisplay,		Zredisplay,		Znotimpl },
	{ Zmetax,		Zmetax,			Znotimpl },
	{ Zbind,		Zbind,			Znotimpl },
	{ Zsavebind,		Zsavebind,		Znotimpl },
	{ Zdispbinds,		Zdispbinds,		Znotimpl },
	{ Znextbuff,		Znextbuff,		Znotimpl },
	{ Zkeybind,		Zkeybind,		Znotimpl },
	{ Zcount,		Zcount,			Znotimpl },
	{ Zincsrch,		Zincsrch,		Znotimpl },
	{ Zrincsrch,		Zrincsrch,		Znotimpl },
	{ Zunmodf,		Zunmodf,		Znotimpl },
	{ Zispace,		Znotimpl,		Zispace },
	{ Zprint,		Zprint,			Znotimpl },
	{ Zdelblanks,		Znotimpl,		Znotimpl },
	{ Zmrkpara,		Zmrkpara,		Znotimpl },
	{ Zupregion,		Znotimpl,		Zupregion },
	{ Zlowregion,		Znotimpl,		Zlowregion },
	{ Zviewline,		Zviewline,		Znotimpl },
	{ Zcmd,			Zcmd,			Znotimpl },
	{ Zjoin,		Znotimpl,		Znotimpl },
	{ Zdate,		Zdate,			Zdate },
	{ Zcgoto,		Zcgoto,			Znotimpl },
	{ Zmake,		Zmake,			Znotimpl },
	{ Znexterr,		Znexterr,		Znotimpl },
	{ Zcmdtobuff,		Znotimpl,		Znotimpl },
	{ Zkill,		Zkill,			Znotimpl },
	{ Zprevwind,		Zprevwind,		Znotimpl },
	{ Zbegwind,		Zbegwind,		Znotimpl },
	{ Zendwind,		Zendwind,		Znotimpl },
	{ Zhelp,		Zhelp,			Znotimpl },
	{ Zgsearch,		Zgsearch,		Znotimpl },
	{ Zindent,		Znotimpl,		Znotimpl },
	{ Zundent,		Znotimpl,		Znotimpl },
	{ Zempty,		Znotimpl,		Zempty },
	{ Zmail,		Zmail,			Zmail },
	{ Zman,			Zman,			Znotimpl },
	{ Zreplace,		Znotimpl,		Znotimpl },
	{ Zsaveall,		Zsaveall,		Znotimpl },
	{ Zbeauty,		Znotimpl,		Znotimpl },
	{ Zviewfile,		Zviewfile,		Znotimpl },
	{ Zdelwind,		Zdelwind,		Znotimpl },
	{ Zshrinkwind,		Zshrinkwind,		Znotimpl },
	{ Zsizewind,		Zsizewind,		Znotimpl },
	{ Zspell,		Znotimpl,		Znotimpl },
	{ Zrevertfile,		Zrevertfile,		Znotimpl },
	{ Zscrolldown,		Zscrolldown,		Znotimpl },
	{ Zscrollup,		Zscrollup,		Znotimpl },
	{ Zformtab,		Znotimpl,		Znotimpl },
	{ Zcwd,			Zcwd,			Znotimpl },
	{ Zcmdbind,		Zcmdbind,		Znotimpl },
	{ Zsaveconfig,		Zsaveconfig,		Znotimpl },
	{ Zsetenv,		Zsetenv,		Znotimpl },
	{ Zref,			Zref,			Znotimpl },
	{ Zzoom,		Zzoom,			Zzoom },
	{ Zgresrch,		Zgresrch,		Znotimpl },
	{ Zgrep,		Zgrep,			Znotimpl },
	{ Zundo,		Znotimpl,		Znotimpl },
	/* only in the PAW you say? pity... */
	{ Znotimpl,		Znotimpl,		Zfname },
};
int Curcmds;


#if DBG
void fcheck(void)
{
	int s1, s2;
	int error = 0;

	/* check the TOLOWER macro */
	if (TOLOWER('c') != 'c')
		error("OLDLOWER set wrong in config.h");

	/* check sizes of various stuff */
	s1 = sizeof(Cnames) / sizeof(struct cnames);
	s2 = (sizeof(Cmds) / sizeof(void *) / 3) - 1;
	if (s1 != NUMFUNCS || s2 != NUMFUNCS) {
		++error;
		Dbg("Cnames: %d Cmds: %d NUMFUNCS: %d\n", s1, s2, NUMFUNCS);
	}

	/* validate the Cnames array the best we can */
	for (s1 = 1; s1 < NUMFUNCS; ++s1) {
		if (strcasecmp(Cnames[s1].name, Cnames[s1 - 1].name) <= 0) {
			++error;
			Dbg("Problem: (%d) %s and %s\n",
			    s1, Cnames[s1 - 1].name, Cnames[s1].name);
		}
		if (strlen(Cnames[s1].name) > (size_t)30) {
			++error;
			Dbg("%s too long\n", Cnames[s1].name);
		}
		if (strncmp(Cnames[s1].name, "Top", 3) == 0) {
			++error;
			Dbg("Zhelp() Top: %s\n", Cnames[s1].name);
		}
	}

	if (error)
		error("INTERNAL ERRORS: check z.out file");
}
#else
void fcheck(void) {}
#endif
