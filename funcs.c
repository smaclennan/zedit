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

void (*Cmds[][2])() = {
	{ Z1wind,		Znotimpl },
	{ Z2wind,		Znotimpl },
	{ Zabort,		Zabort },
	{ Zagain,		Zpart },
	{ Zarg,			Znotimpl },
	{ Zbegline,		Zbegline },
	{ Zbpara,		Zbpara },
	{ Zbword,		Zbword },
	{ Zcalc,		Znotimpl },
	{ Zcapword,		Zcapword },
	{ Zcase,		Zcase },
	{ Zcenter,		Znotimpl },
	{ Zcopyrgn,		Zcopyrgn },
	{ Zctrlx,		Zctrlx },
	{ Zdelchar,		Zdelchar },
	{ Zdeleol,		Zdeleol },
	{ Zdelline,		Zdelline },
	{ Zdelrgn,		Zdelrgn },
	{ Zdelwhite,		Zdelwhite },
	{ Zdelword,		Zdelword },
	{ Zendline,		Zendline },
	{ Zexit,		Znotimpl },
	{ Zfileread,		Znotimpl },
	{ Zfilesave,		Znotimpl },
	{ Zfilewrite,		Znotimpl },
	{ Zmode,		Znotimpl },
	{ Zfillpara,		Znotimpl },
	{ Zfindfile,		Znotimpl },
	{ Zfindtag,		Znotimpl },
	{ Zfpara,		Zfpara },
	{ Zfword,		Zfword },
	{ Zgetbword,		Zgetbword },
	{ Zgrowwind,		Znotimpl },
	{ Zhexout,		Znotimpl },
	{ Zinsert,		pinsert },
	{ Zkillbuff,		Znotimpl },
	{ Zlgoto,		Znotimpl },
	{ Zlowword,		Zlowword },
	{ Zlstbuff,		Znotimpl },
	{ Zmakedel,		Zmakedel },
	{ Zmeta,		Zmeta },
	{ Znextchar,		Znextchar },
	{ Znextline,		Znextline },
	{ Znextpage,		Znotimpl },
	{ Znotimpl,		Znotimpl },
	{ Znxtbookmrk,		Znotimpl },
	{ Znxtothrwind,		Znotimpl },
	{ Zopenline,		Znotimpl },
	{ Zoverin,		Zoverin },
	{ Zprevchar,		Zprevchar },
	{ Zprevline,		Zprevline },
	{ Zprevothrwind,	Znotimpl },
	{ Zprevpage,		Znotimpl },
	{ Zprintpos,		Znotimpl },
	{ Zquery,		Znotimpl },
	{ Zquote,		Zquote },
	{ Zrdelchar,		Zrdelchar },
	{ Zrdelword,		Zrdelword },
	{ Zrereplace,		Znotimpl },
	{ Zresrch,		Znotimpl },
	{ Zrsearch,		Znotimpl },
	{ Zsearch,		Znotimpl },
	{ Zsetavar,		Znotimpl },
	{ Zsetbookmrk,		Znotimpl },
	{ Zsetmrk,		Zsetmrk },
	{ Zshell,		Znotimpl },
	{ Zstat,		Znotimpl },
	{ Zswapchar,		Zswapchar },
	{ Zswapmrk,		Zswapmrk },
	{ Znextwind,		Znotimpl },
	{ Zswapword,		Zswapword },
	{ Zswitchto,		Znotimpl },
	{ Ztab,			pinsert },
	{ Ztoend,		Ztoend },
	{ Ztostart,		Ztostart },
	{ Zupword,		Zupword },
	{ Zyank,		Zyank },
	{ Zcindent,		pnewline },
	{ Zcinsert,		pinsert },
	{ Zfillchk,		Zfillchk },
	{ Znewline,		pnewline },
	{ Zredisplay,		Znotimpl },
	{ Zmetax,		Znotimpl },
	{ Zbind,		Znotimpl },
	{ Zsavebind,		Znotimpl },
	{ Zdispbinds,		Znotimpl },
	{ Znextbuff,		Znotimpl },
	{ Zkeybind,		Znotimpl },
	{ Zcount,		Znotimpl },
	{ Zincsrch,		Znotimpl },
	{ Zrincsrch,		Znotimpl },
	{ Zunmodf,		Znotimpl },
	{ Zdelblanks,		Znotimpl },
	{ Zmrkpara,		Znotimpl },
	{ Zupregion,		Zupregion },
	{ Zlowregion,		Zlowregion },
	{ Zviewline,		Znotimpl },
	{ Zcmd,			Znotimpl },
	{ Zjoin,		Znotimpl },
	{ Zdate,		Zdate },
	{ Zcgoto,		Znotimpl },
	{ Zmake,		Znotimpl },
	{ Znexterr,		Znotimpl },
	{ Zcmdtobuff,		Znotimpl },
	{ Zkill,		Znotimpl },
	{ Zprevwind,		Znotimpl },
	{ Zbegwind,		Znotimpl },
	{ Zendwind,		Znotimpl },
	{ Zhelp,		Znotimpl },
	{ Zgsearch,		Znotimpl },
	{ Zindent,		Znotimpl },
	{ Zundent,		Znotimpl },
	{ Zempty,		Zempty },
	{ Zreplace,		Znotimpl },
	{ Zsaveall,		Znotimpl },
	{ Zdelwind,		Znotimpl },
	{ Zshrinkwind,		Znotimpl },
	{ Zsizewind,		Znotimpl },
	{ Zspell,		Znotimpl },
	{ Zrevertfile,		Znotimpl },
	{ Zscrolldown,		Znotimpl },
	{ Zscrollup,		Znotimpl },
	{ Zcwd,			Znotimpl },
	{ Zcmdbind,		Znotimpl },
	{ Zshowconfig,		Znotimpl },
	{ Zsetenv,		Znotimpl },
	{ Zref,			Znotimpl },
	{ Zgresrch,		Znotimpl },
	{ Zgrep,		Znotimpl },
	{ Zundo,		Znotimpl },
	/* only in the PAW you say? pity... */
	{ Znotimpl,		Zfname },
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
	s2 = (sizeof(Cmds) / sizeof(void *) / 2) - 1;
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
