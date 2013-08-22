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
#include "cnames.h"

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
	{ Zreplace,		Znotimpl,		Znotimpl },
	{ Zsaveall,		Zsaveall,		Znotimpl },
	{ Zviewfile,		Zviewfile,		Znotimpl },
	{ Zdelwind,		Zdelwind,		Znotimpl },
	{ Zshrinkwind,		Zshrinkwind,		Znotimpl },
	{ Zsizewind,		Zsizewind,		Znotimpl },
	{ Zspell,		Znotimpl,		Znotimpl },
	{ Zrevertfile,		Zrevertfile,		Znotimpl },
	{ Zscrolldown,		Zscrolldown,		Znotimpl },
	{ Zscrollup,		Zscrollup,		Znotimpl },
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
