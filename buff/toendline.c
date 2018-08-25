#include "buff.h"

/** Move point to the end of the line. */
void toendline(struct buff *buff)
{
	if (bcsearch(buff, '\n'))
		bmove(buff, -1);
}
