/* funcs.h - Zedit function defines
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

#define Z1WIND		0
#define Z2WIND		Z1WIND + 1
#define ZABORT		Z2WIND + 1
#define ZAGAIN		ZABORT + 1
#define ZARG		ZAGAIN + 1
#define ZBEGLINE	ZARG + 1
#define ZBPARA		ZBEGLINE + 1
#define ZBWORD		ZBPARA + 1
#define ZCALC		ZBWORD + 1
#define ZCAPWORD	ZCALC + 1
#define ZCASE		ZCAPWORD + 1
#define ZCENTER		ZCASE + 1
#define ZCOPYRGN	ZCENTER + 1
#define ZCTRLX		ZCOPYRGN + 1
#define ZDELCHAR	ZCTRLX + 1
#define ZDELEOL		ZDELCHAR + 1
#define ZDELLINE	ZDELEOL + 1
#define ZDELRGN		ZDELLINE + 1
#define ZDELWHITE	ZDELRGN + 1
#define ZDELWORD	ZDELWHITE + 1
#define ZENDLINE	ZDELWORD + 1
#define ZEXIT		ZENDLINE + 1
#define ZFILEREAD	ZEXIT + 1
#define ZFILESAVE	ZFILEREAD + 1
#define ZFILEWRITE	ZFILESAVE + 1
#define ZMODE		ZFILEWRITE + 1
#define ZFILLPARA	ZMODE + 1
#define ZFINDFILE	ZFILLPARA + 1
#define ZFINDTAG	ZFINDFILE + 1
#define ZFPARA		ZFINDTAG + 1
#define ZFWORD		ZFPARA + 1
#define ZGETBWORD	ZFWORD + 1
#define ZGROWWINDOW	ZGETBWORD + 1
#define ZHEXOUT		ZGROWWINDOW + 1
#define ZINSERT		ZHEXOUT + 1 /* Also used for pinsert */
#define ZKILLBUFF	ZINSERT + 1
#define ZLGOTO		ZKILLBUFF + 1
#define ZLOWWORD	ZLGOTO + 1
#define ZLSTBUFF	ZLOWWORD + 1
#define ZMAKEDEL	ZLSTBUFF + 1
#define ZMETA		ZMAKEDEL + 1
#define ZNEXTCHAR	ZMETA + 1
#define ZNEXTLINE	ZNEXTCHAR + 1
#define ZNEXTPAGE	ZNEXTLINE + 1
#define ZNOTIMPL	ZNEXTPAGE + 1
#define ZNXTBOOKMRK	ZNOTIMPL + 1
#define ZNXTOTHRWIND	ZNXTBOOKMRK + 1
#define ZOVERIN		ZNXTOTHRWIND + 1
#define ZOPENLINE	ZOVERIN + 1
#define ZPREVCHAR	ZOPENLINE + 1
#define ZPREVLINE	ZPREVCHAR + 1
#define ZPREVOTHRWIND	ZPREVLINE + 1
#define ZPREVPAGE	ZPREVOTHRWIND + 1
#define ZPRINTPOS	ZPREVPAGE + 1
#define ZQUERY		ZPRINTPOS + 1
#define ZQUOTE		ZQUERY + 1
#define ZRDELCHAR	ZQUOTE + 1
#define ZRDELWORD	ZRDELCHAR + 1
#define ZREREPLACE	ZRDELWORD + 1
#define ZRESRCH		ZREREPLACE + 1
#define ZRSEARCH	ZRESRCH + 1
#define ZSEARCH		ZRSEARCH + 1
#define ZSETAVAR	ZSEARCH + 1
#define ZSETBOOKMRK	ZSETAVAR + 1
#define ZSETMRK		ZSETBOOKMRK + 1
#define ZSHELL		ZSETMRK + 1
#define ZSTAT		ZSHELL + 1
#define ZSWAPCHAR	ZSTAT + 1
#define ZSWAPMRK	ZSWAPCHAR + 1
#define ZNEXTWIND	ZSWAPMRK + 1
#define ZSWAPWORD	ZNEXTWIND + 1
#define ZSWITCHTO	ZSWAPWORD + 1
#define ZTAB		ZSWITCHTO + 1
#define ZTOEND		ZTAB + 1
#define ZTOSTART	ZTOEND + 1
#define ZUPWORD		ZTOSTART + 1
#define ZYANK		ZUPWORD + 1
#define ZCINDENT	ZYANK + 1
#define ZCINSERT	ZCINDENT + 1
#define ZFILLCHK	ZCINSERT + 1
#define ZNEWLINE	ZFILLCHK + 1
#define ZREDISPLAY	ZNEWLINE + 1
#define ZMETAX		ZREDISPLAY + 1
#define ZBIND		ZMETAX + 1
#define ZSAVEBIND	ZBIND + 1
#define ZDISPBINDS	ZSAVEBIND + 1
#define ZNEXTBUFF	ZDISPBINDS + 1
#define ZKEYBIND	ZNEXTBUFF + 1
#define ZCOUNT		ZKEYBIND + 1
#define ZINCSRCH	ZCOUNT + 1
#define ZRINCSRCH	ZINCSRCH + 1
#define ZUNMODF		ZRINCSRCH + 1
#define ZISPACE		ZUNMODF + 1
#define ZDELBLANKS	ZISPACE + 1
#define ZMRKPARA	ZDELBLANKS + 1
#define ZUPREGION	ZMRKPARA + 1
#define ZLOWREGION	ZUPREGION + 1
#define ZVIEWLINE	ZLOWREGION + 1
#define ZCMD		ZVIEWLINE + 1
#define ZJOIN		ZCMD + 1
#define ZDATE		ZJOIN + 1
#define ZCGOTO		ZDATE + 1
#define ZMAKE		ZCGOTO + 1
#define ZNEXTERR	ZMAKE + 1
#define ZCMDTOBUFF	ZNEXTERR + 1
#define ZKILL		ZCMDTOBUFF + 1
#define ZPREVWIND	ZKILL + 1
#define ZBEGWIND	ZPREVWIND + 1
#define ZENDWIND	ZBEGWIND + 1
#define ZHELP		ZENDWIND + 1
#define ZGSEARCH	ZHELP + 1
#define ZINDENT		ZGSEARCH + 1
#define ZUNDENT		ZINDENT + 1
#define ZEMPTY		ZUNDENT + 1
#define ZREPLACE	ZEMPTY + 1
#define ZSAVEALL	ZREPLACE + 1
#define ZDELWIND	ZSAVEALL + 1
#define ZSHRINKWIND	ZDELWIND + 1
#define ZSIZEWIND	ZSHRINKWIND + 1
#define ZSPELL		ZSIZEWIND + 1
#define ZREVERTFILE	ZSPELL + 1
#define ZSCROLLDOWN	ZREVERTFILE + 1
#define ZSCROLLUP	ZSCROLLDOWN + 1
#define ZCWD		ZSCROLLUP + 1
#define ZCMDBIND	ZCWD + 1
#define ZSAVECONFIG	ZCMDBIND + 1
#define ZSETENV		ZSAVECONFIG + 1
#define ZREF		ZSETENV + 1
#define ZZOOM		ZREF + 1
#define ZGRESRCH	ZZOOM + 1
#define ZGREP		ZGRESRCH + 1
#define ZUNDO		ZGREP + 1
#define NUMFUNCS	ZUNDO + 1

/* this is used by the getfname command in the PAW for command completion */
#define ZFNAME		NUMFUNCS
