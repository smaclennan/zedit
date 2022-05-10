/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Default #bsetmod function. Does nothing ;) */
static void dummy_bsetmod(struct buff *buff) {}
/** If you need to know when the buffer is modified, this callback
 * will get called to notify you. By default it is set to a dummy
 * function that does nothing.
 */
void (*bsetmod)(struct buff *buff) = dummy_bsetmod;

#ifdef GLOBAL_MARKS
/** Global mark list. The buffer code keeps the marks in this list
 * up to date.
 */
struct mark *Marklist;	/* the marks list tail */
#endif
/* @} */


/* \cond skip */
/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x)	#x
#define __stringify(x)	__stringify_1(x)

/* The marker is for `strings libbuff.a | fgrep Version' */
const char *libbuff_marker =
	"Version-" __stringify(LIBBUFF_MAJOR) "." __stringify(LIBBUFF_MINOR);

const char *libbuff_version =
	__stringify(LIBBUFF_MAJOR) "." __stringify(LIBBUFF_MINOR);
/* \endcond */
