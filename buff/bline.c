#include "buff.h"

/** Return the current line of the point. */
unsigned long bline(struct buff *buff)
{
	struct mark tmark;
	unsigned long line = 1;

	bmrktopnt(buff, &tmark);
	btostart(buff);
	while (bcsearch(buff, '\n') && !bisaftermrk(buff, &tmark))
		++line;
	bpnttomrk(buff, &tmark);
	return line;
}
