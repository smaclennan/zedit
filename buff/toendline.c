/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Move point to the end of the line.
 * @param buff The buffer to move the Point in.
 */
void toendline(struct buff *buff)
{
	if (bcsearch(buff, '\n'))
		bmove(buff, -1);
}

/** Move point to the beginning of the line.
 * @param buff The buffer to move the Point in.
 */
void tobegline(struct buff *buff)
{
	if (buff->curchar > 0 && *(buff->curcptr - 1) == '\n')
		return;
	if (bcrsearch(buff, '\n'))
		bmove1(buff);
}
/* @} */
