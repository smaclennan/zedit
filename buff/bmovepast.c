#include "buff.h"

/** Go forward or back past a thingy */
void bmovepast(struct buff *buff, int (*pred)(int), int forward)
{
	if (forward)
		while (!bisend(buff) && pred(*buff->curcptr))
			bmove1(buff);
	else {
		bmove(buff, -1);
		while (!bisstart(buff) && pred(*buff->curcptr))
			bmove(buff, -1);
		if (!pred(*buff->curcptr))
			bmove1(buff);
	}
}
