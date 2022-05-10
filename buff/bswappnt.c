/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Swap the Point and the mark.
 * @param buff The buffer the point is in.
 * @param mark The mark to swap with.
 * @return 1 on success, 0 if mark not in buffer.
 */
int bswappnt(struct buff *buff, struct mark *mark)
{
	struct mark tmp;

	if (mark->mbuff != buff)
		return 0;

	bmrktopnt(buff, &tmp);
	bpnttomrk(buff, mark);
	mrktomrk(mark, &tmp);
	return 1;
}
/* @} */
