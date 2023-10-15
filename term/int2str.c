static void reverse_string(char *start, char *end)
{
	int len = (end - start) / 2;
	--end;
	for (int i = 0; i < len; ++i) {
		char temp = *end;
		*end = *start;
		*start = temp;
		start++;
		end--;
	}
}

/** Integer to ascii.
 * @param val The integer to convert.
 * @param out The output string.
 * @return A pointer to the end of the string.
 */
char *int2str(long val, char *out)
{
	char *p = out;

	if (val == 0) {
		*p++ = '0';
		*p = 0;
	} else {
		int neg = val < 0;

		if (neg)
			val = -val;

		while (val > 0) {
			*p++ = (val % 10) + '0';
			val /= 10;
		}
		if (neg)
			*p++ = '-';
		*p = 0;

		reverse_string(out, p);
	}

	return p;
}
