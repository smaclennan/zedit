/* Unfortunately the lowercasing of the Zedit functions causes a lot
 * of collisions with curses.h. So we rename things here to get around
 * this. See termcap.c for how to include this.
 */

#undef TRUE
#undef FALSE
#undef SYSV4
#undef echo
#define tgoto _n_tgoto
#define refresh _n_refresh
#define savetty _n_savetty

#include <term.h>
#include <curses.h>

#undef tgoto
#undef refresh
#undef echo
#undef savetty
#undef hangup
