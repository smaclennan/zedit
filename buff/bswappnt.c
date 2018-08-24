#include "buff.h"

/** Swap the point and the mark. */
bool bswappnt(struct buff *buff, struct mark *tmark)
{
	struct mark tmp;

	if (tmark->mbuff != buff)
		return false;

	bmrktopnt(buff, &tmp);
	bpnttomrk(buff, tmark);
	mrktomrk(tmark, &tmp);
	return true;
}
