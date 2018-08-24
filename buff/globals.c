#include "buff.h"

/** Default #bsetmod function. Does nothing ;) */
static void dummy_bsetmod(struct buff *buff) {}
/** If you need to know when the buffer is modified, this callback
 * will get called to notify you. By default it is set to a dummy
 * function that does nothing.
 */
void (*bsetmod)(struct buff *buff) = dummy_bsetmod;

#ifdef HAVE_GLOBAL_MARKS
/** Global mark list. The buffer code keeps the marks in this list
 * up to date.
 */
struct mark *Marklist;	/* the marks list tail */
#endif
