#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>


static void pad(int cur, int meta, FILE *out)
{
	if (cur == meta) return;
	assert(cur > meta);
	fputs("\t", out);
	while (meta++ < cur)
		fputs("0,", out);
	fputs("\n", out);
}

static void comment(FILE *out, int len, char *fmt, ...)
{
	va_list arg_ptr;

	len += 7; /* there is one tab */
	while (len < 32) {
		fputs("\t", out);
		len += 8;
	}

	va_start(arg_ptr, fmt);
	vfprintf(out, fmt, arg_ptr);
	va_end(arg_ptr);
}

int main(int argc, char *argv[])
{
	FILE *in = fopen("bind.c", "r");
	if (!in) {
		in = fopen("../bind.c", "r");
		if (!in) {
			perror("bind.c");
			exit(1);
		}
	}

	FILE *out = fopen("dosbind.c", "w");
	if (!out) {
		perror("dosbind.c");
		exit(1);
	}

	char line[128], *p;
	while (fgets(line, sizeof(line), in)) {
		for (p = line; isspace(*p); ++p) ;
		if (*p == '[') /* first of array type */
			break;
		fputs(line, out);
	}

	int n, cur = 32, meta = 32; /* first 32 done - start at C-X space */
	do {
		char ch, rest[128], tc[40];

		if (*line == '}') {
			fputs(line, out);
			break;
		} else if (sscanf(line, " [CX('%c')] = %s", &ch, rest) == 2) {
			if ((p = strchr(line, '\n'))) *p = '\0';
			cur = ch - ' ' + 32;
			pad(cur, meta, out);
			n = fprintf(out, "\t%s", rest);
			comment(out, n, "/* C-X %c */\n", ch);
		} else if (sscanf(line, " [CX(%d)] = %s", &cur, rest) == 2) {
			pad(cur, meta, out);
			p = strchr(line, ','); assert(p); ++p;
			fprintf(out, "\t%s%s", rest, p);
		} else if (sscanf(line, " [M('%c')] = %s", &ch, rest) == 2) {
			if ((p = strchr(line, '\n'))) *p = '\0';
			cur = ch - ' ' + 32 + 128;
			pad(cur, meta, out);
			n = fprintf(out, "\t%s", rest);
			comment(out, n, "/* M-%c */\n", ch);
		} else if (sscanf(line, " [M(%d)] = %s", &cur, rest) == 2) {
			cur += 128;
			pad(cur, meta, out);
			p = strchr(line, ','); assert(p); ++p;
			fprintf(out, "\t%s%s", rest, p);
		} else if (sscanf(line, " [TC_%[A-Z0-9_]] = %s", tc, rest) == 2) {
			if (strcmp(tc, "UP") == 0)
				cur = 'a';
			else
				++cur;
			pad(cur, meta, out);
			n = fprintf(out, "\t%s", rest);
			comment(out, n, "/* TC_%s */\n", tc);
		} else {
			for (p = line; isspace(*p); ++p) ;
			assert(*p != '[');
			fputs(line, out);
		}

		meta = cur + 1;
	} while (fgets(line, sizeof(line), in));

	while (fgets(line, sizeof(line), in))
		fputs(line, out);

	int rc = 0;
	if (ferror(in)) rc = 1;
	if (ferror(out)) rc = 1;

	fclose(in);
	fclose(out);

	return rc;
}

/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall makedosbind.c -o makedosbind"
 * End:
 */
