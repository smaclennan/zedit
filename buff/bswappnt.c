#include "buff.h"

/** Swap the point and the mark. */
int bswappnt(struct buff *buff, struct mark *tmark)
{
	struct mark tmp;

	if (tmark->mbuff != buff)
		return 0;

	bmrktopnt(buff, &tmp);
	bpnttomrk(buff, tmark);
	mrktomrk(tmark, &tmp);
	return 1;
}
