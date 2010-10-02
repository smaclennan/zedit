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
#define Z2WIND		1
#define ZABORT		2
#define ZAGAIN		3
#define ZARG		4
#define ZBEGLINE	5
#define ZBPARA		6
#define ZBWORD		7
#define ZCALC		8
#define ZCAPWORD	9
#define ZCASE		10
#define ZCENTER		11
#define ZCOPYRGN	12
#define ZCTRLX		13
#define ZDELCHAR	14
#define ZDELEOL		15
#define ZDELLINE	16
#define ZDELRGN		17
#define ZDELWHITE	18
#define ZDELWORD	19
#define ZENDLINE	20
#define ZEXIT		21
#define ZFILEREAD	22
#define ZFILESAVE	23
#define ZFILEWRITE	24
#define ZMODE		25
#define ZFILLPARA	26
#define ZFINDFILE	27
#define ZFINDTAG	28
#define ZFPARA		29
#define ZFWORD		30
#define ZGETBWORD	31
#define ZGROWWINDOW	32
#define ZHEXOUT		33
#define ZINSERT		34	/* Also used for pinsert */
#define ZKILLBUFF	35
#define ZLGOTO		36
#define ZLOWWORD	37
#define ZLSTBUFF	38
#define ZMAKEDEL	39
#define ZMETA		40
#define ZNEXTCHAR	41
#define ZNEXTLINE	42
#define ZNEXTPAGE	43
#define ZNOTIMPL	44
#define ZNXTBOOKMRK	45
#define ZNXTOTHRWIND	46
#define ZQUIT		47
#define ZOPENLINE	48
#define ZOVERIN		49
#define ZPREVCHAR	50
#define ZPREVLINE	51
#define ZPREVOTHRWIND	52
#define ZPREVPAGE	53
#define ZPRINTPOS	54
#define ZQUERY		55
#define ZQUOTE		56
#define ZRDELCHAR	57
#define ZRDELWORD	58
#define ZREREPLACE	59
#define ZRESRCH		60
#define ZRSEARCH	61
#define ZSEARCH		62
#define ZSETAVAR	63
#define ZSETBOOKMRK	64
#define ZSETMRK		65
#define ZSHELL		66
#define ZSTAT		67
#define ZSWAPCHAR	68
#define ZSWAPMRK	69
#define ZNEXTWIND	70
#define ZSWAPWORD	71
#define ZSWITCHTO	72
#define ZTAB		73
#define ZTOEND		74
#define ZTOSTART	75
#define ZUPWORD		76
#define ZYANK		77
#define ZCINDENT	78
#define ZCINSERT	79
#define ZFILLCHK	80
#define ZNEWLINE	81
#define ZREDISPLAY	82
#define ZMETAX		83
#define ZBIND		84
#define ZSAVEBIND	85
#define ZDISPBINDS	86
#define ZNEXTBUFF	87
#define ZKEYBIND	88
#define ZCOUNT		89
#define ZINCSRCH	90
#define ZRINCSRCH	91
#define ZUNMODF		92
#define ZISPACE		93
#define ZPRINT		94
#define ZDELBLANKS	95
#define ZMRKPARA	96
#define ZUPREGION	97
#define ZLOWREGION	98
#define ZVIEWLINE	99
#define ZCMD		100
#define ZJOIN		101
#define ZDATE		102
#define ZCGOTO		103
#define ZMAKE		104
#define ZNEXTERR	105
#define ZCMDTOBUFF	106
#define ZKILL		107
#define ZPREVWIND	108
#define ZBEGWIND	109
#define ZENDWIND	110
#define ZHELP		111
#define ZGSEARCH	112
#define ZINDENT		113
#define ZUNDENT		114
#define ZEMPTY		115
#define ZMAIL		116
#define ZMAN		117
#define ZREPLACE	118
#define ZSAVEALL	119
#define ZBEAUTY		120
#define ZVIEWFILE	121
#define ZDELWIND	122
#define ZSHRINKWIND	123
#define ZSIZEWIND	124
#define ZSPELL		125
#define ZREVERTFILE	126
#define ZSCROLLDOWN	127
#define ZSCROLLUP	128
#define ZFORMTAB	129
#define ZCWD		130
#define ZCMDBIND	131
#define ZSAVECONFIG	132
#define ZSETENV		133
#define ZREF		134
#define ZZOOM		135
#define ZGRESRCH	136
#define ZGREP		137
#define ZUNDO		138
#define NUMFUNCS	139

/* this is used by the getfname command in the PAW for command completion */
#define ZFNAME		NUMFUNCS
