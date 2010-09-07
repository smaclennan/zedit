/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
/* OPERATING SYSTEM - define only one */
#define LINUX   	1			/* Linux */
#define SYSV		0			/* Unix System V Release 2/3 */
#define SYSV4		0			/* Unix System V Release 4   */
#define BSD			0			/* Berkely */
#define SUNBSD		0			/* Sun BSD */
#define ULTRIX		0			/* almost a BSD... */

/* SCREEN DRIVER - choose terminfo over termcaps if possible */
#ifdef XWINDOWS
#define TERMINFO	0		/* don't change this one */
#else
#define XWINDOWS	0
#define TERMINFO	1		/* set terminfo here */
#endif
#define TERMCAP		0

/* USER CONFIGURABLE - don't define any, see if I care */
#define DBG			1			/* turn debugs on */
#define SLOW_DISK	0			/* File writes try to buffer up the data
								 * to a block size.
								 */
#define SPELL		1			/* ispell interface */
#define USE_RCS		1			/* Use RCS files */
#define FORK_ZHELP	1			/* Fork zhelp */
#define ABORT_DISP	0			/* Allows the user to abort the screen update
								 * by hitting a key. For slowww terminals.
								 * NOT XWINDOWS
								 */
#define COMMENTBOLD	1			/* bold C comments */
#define HAS_RESIZE	1			/* define this if you are using an xterm
								 * that supports resize command
								 */
#define XORCURSOR	1			/* Some machined (e.g. sun3 && sun4) xor the
								 * attribute for the cursor. This means that
								 * when the cursor is on the mark, they both
								 *	disappear. Sighhhh. This kludge fixes the
								 *	problem.
								 */
#define FLOATCALC	1			/* Allow floats in calc command */

/* DON'T TOUCH THESE */
#if SUNBSD || ULTRIX
#undef  BSD
#define BSD			1
#endif
#if LINUX
#undef  SYSV4
#define SYSV4		1
#endif
#if SYSV4
#undef  SYSV
#define SYSV		1
#endif
#define PIPESH		1

/*
 * When this define is set, a count is kept of all the commands executed.
 * The program zprof can then be used to produce formatted output from
 * this file. See zprof.c for more documentation.
#define CMDPROF
 */
