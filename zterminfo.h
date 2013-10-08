/* Unfortunately the lowercasing of the Zedit functions causes a lot
 * of collisions with curses.h. So we rename things here to get around
 * this. See termcap.c for how to include this.
 */

#undef echo
#define savetty _n_savetty

#include <term.h>
#include <curses.h>

#undef echo
#undef savetty
#undef hangup
