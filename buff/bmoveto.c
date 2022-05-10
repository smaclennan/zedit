/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Go forward or back to a thingy. See bistoken() for an example
 * thingy predicate function.
 * @param buff The buffer to move in.
 * @param pred The thingy function.
 * @param forward 1 to move forward, 0 to move backward.
 */
void bmoveto(struct buff *buff, int (*pred)(int), int forward)
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
/* @} */
