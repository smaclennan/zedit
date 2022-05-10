/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Move the current buffer Point to the mark.
 * @param buff The buffer the point is in.
 * @param mark The mark to move the Point to.
 * @return 1 on success, 0 if the mark is not in the same buffer as
 * the Point.
 */
int bpnttomrk(struct buff *buff, struct mark *mark)
{
	if (mark->mbuff != buff)
		return 0;
	if (mark->mpage)
		makecur(buff, mark->mpage, mark->moffset);
	return 1;
}
/* @} */
