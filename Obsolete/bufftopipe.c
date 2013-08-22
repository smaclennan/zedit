/* Returns -1 if popen failed, else exit code.
 * Leaves Point and Mark where they where.
 */
static int bufftopipe(struct buff *buff, char *cmd)
{
	FILE *pfp;
	struct mark spnt, end;
	struct buff *was = Curbuff;

	strcat(cmd, ">/dev/null 2>&1");
	pfp = popen(cmd, "w");
	if (pfp == NULL)
		return -1;

	bswitchto(buff);
	bmrktopnt(&spnt);	/* save current Point */
	if (Argp) {
		/* Use the region - make sure mark is after Point */
		mrktomrk(&end, Curbuff->mark);
		if (bisaftermrk(&end))
			bswappnt(&end);
	} else {
		/* use entire buffer */
		btoend();
		bmrktopnt(&end);
		btostart();
	}
	Argp = FALSE;
	Arg = 0;

	for (; bisbeforemrk(&end); bmove1())
		putc(Buff(), pfp);

	bpnttomrk(&spnt);
	bswitchto(was);

	return pclose(pfp) >> 8;
}

