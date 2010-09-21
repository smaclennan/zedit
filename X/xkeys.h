/* xkeys.h - Zedit X key definitions
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

#define ZXK_START		384

/* Cursor control & motion */

#define ZXK_Home		(ZXK_START + 0)
#define ZXK_Left		(ZXK_START + 1)	/* Move left, left arrow */
#define ZXK_Up			(ZXK_START + 2)	/* Move up, up arrow */
#define ZXK_Right		(ZXK_START + 3)	/* Move right, right arrow */
#define ZXK_Down		(ZXK_START + 4)	/* Move down, down arrow */
#define ZXK_Prior		(ZXK_START + 5)	/* Prior, previous */
#define ZXK_Next		(ZXK_START + 6)	/* Next */
#define ZXK_End			(ZXK_START + 7)	/* EOL */
#define ZXK_Begin		(ZXK_START + 8)	/* BOL */


/* Misc Functions */

#define ZXK_Select		(ZXK_START + 9)	 /* Select, mark */
#define ZXK_Print		(ZXK_START + 10)
#define ZXK_Execute		(ZXK_START + 11) /* Execute, run, do */
#define ZXK_Insert		(ZXK_START + 12) /* Insert, insert here */
#define ZXK_dummy		(ZXK_START + 13)
#define ZXK_Undo		(ZXK_START + 14) /* Undo, oops */
#define ZXK_Redo		(ZXK_START + 15) /* redo, again */
#define ZXK_Menu		(ZXK_START + 16)
#define ZXK_Find		(ZXK_START + 17) /* Find, search */
#define ZXK_Cancel		(ZXK_START + 18) /* Cancel, stop, abort, exit */
#define ZXK_Help		(ZXK_START + 19) /* Help, ? */
#define ZXK_Break		(ZXK_START + 20)

/*
 * Auxilliary Functions; note the duplicate definitions for left and right
 * function keys;  Sun keyboards and a few other manufactures have such
 * function key groups on the left and/or right sides of the keyboard.
 * We've not found a keyboard with more than 35 function keys total.
 */

#define ZXK_F1			(ZXK_START + 21)
#define ZXK_F2			(ZXK_START + 22)
#define ZXK_F3			(ZXK_START + 23)
#define ZXK_F4			(ZXK_START + 24)
#define ZXK_F5			(ZXK_START + 25)
#define ZXK_F6			(ZXK_START + 26)
#define ZXK_F7			(ZXK_START + 27)
#define ZXK_F8			(ZXK_START + 28)
#define ZXK_F9			(ZXK_START + 29)
#define ZXK_F10			(ZXK_START + 30)
#define ZXK_F11			(ZXK_START + 31)
#define ZXK_F12			(ZXK_START + 32)
#define ZXK_F13			(ZXK_START + 33)
#define ZXK_F14			(ZXK_START + 34)
#define ZXK_F15			(ZXK_START + 35)
#define ZXK_F16			(ZXK_START + 36)
#define ZXK_F17			(ZXK_START + 37)
#define ZXK_F18			(ZXK_START + 38)
#define ZXK_F19			(ZXK_START + 39)
#define ZXK_F20			(ZXK_START + 40)
#define ZXK_F21			(ZXK_START + 41)
#define ZXK_F22			(ZXK_START + 42)
#define ZXK_F23			(ZXK_START + 43)
#define ZXK_F24			(ZXK_START + 44)
#define ZXK_F25			(ZXK_START + 45)
#define ZXK_F26			(ZXK_START + 46)
#define ZXK_F27			(ZXK_START + 47)
#define ZXK_F28			(ZXK_START + 48)
#define ZXK_F29			(ZXK_START + 49)
#define ZXK_F30			(ZXK_START + 50)
#define ZXK_F31			(ZXK_START + 51)
#define ZXK_F32			(ZXK_START + 52)
#define ZXK_F33			(ZXK_START + 53)
#define ZXK_F34			(ZXK_START + 54)
#define ZXK_F35			(ZXK_START + 55)

/* Control cursor keys */

#define ZXK_CHome		(ZXK_START + 56) /* Home */
#define ZXK_CLeft		(ZXK_START + 57) /* Move left, left arrow */
#define ZXK_CUp			(ZXK_START + 58) /* Move up, up arrow */
#define ZXK_CRight		(ZXK_START + 59) /* Move right, right arrow */
#define ZXK_CDown		(ZXK_START + 60) /* Move down, down arrow */
#define ZXK_CPrior		(ZXK_START + 61) /* Prior, previous */
#define ZXK_CNext		(ZXK_START + 62) /* Next */
#define ZXK_CEnd		(ZXK_START + 63) /* EOL */
#define ZXK_CBegin		(ZXK_START + 64) /* BOL */

/* Shift cursor keys */

#define ZXK_SHome		(ZXK_START + 65) /* Home */
#define ZXK_SLeft		(ZXK_START + 66) /* Move left, left arrow */
#define ZXK_SUp			(ZXK_START + 67) /* Move up, up arrow */
#define ZXK_SRight		(ZXK_START + 68) /* Move right, right arrow */
#define ZXK_SDown		(ZXK_START + 69) /* Move down, down arrow */
#define ZXK_SPrior		(ZXK_START + 70) /* Prior, previous */
#define ZXK_SNext		(ZXK_START + 71) /* Next */
#define ZXK_SEnd		(ZXK_START + 72) /* EOL */
#define ZXK_SBegin		(ZXK_START + 73) /* BOL */

#define NUMKEYS			(ZXK_START + 74)
