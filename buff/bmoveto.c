#include "buff.h"

/** Go forward or back to a thingy */
void bmoveto(struct buff *buff, int (*pred)(int), bool forward)
{
	if (forward)
		while (!bisend(buff) && !pred(*buff->curcptr))
			bmove1(buff);
	else {
		bmove(buff, -1);
		while (!bisstart(buff) && !pred(*buff->curcptr))
			bmove(buff, -1);
		if (!bisstart(buff))
			bmove1(buff);
	}
}
