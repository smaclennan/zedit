#include "buff.h"

/** Put the current buffer point at the mark */
int bpnttomrk(struct buff *buff, struct mark *tmark)
{
	if (tmark->mbuff != buff)
		return 0;
	if (tmark->mpage)
		makecur(buff, tmark->mpage, tmark->moffset);
	return 1;
}
