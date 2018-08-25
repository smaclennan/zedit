#include "buff.h"

/** Peek the previous byte. Does not move the point. Returns LF at
 * start of buffer.
 */
Byte bpeek(struct buff *buff)
{
	Byte ch;

	if (buff->curchar > 0)
		return *(buff->curcptr - 1);
	if (bisstart(buff))
		/* Pretend we are at the start of a line.
		 * Needed for delete-to-eol and step in reg.c.
		 */
		return '\n';
	bmove(buff, -1);
	ch = *buff->curcptr;
	bmove1(buff);
	return ch;
}
