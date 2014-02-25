#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>


static void hack(FILE *out)
{
	fputs("\n\t/* HACK. I wanted to keep this in DOS specific code. */\n", out);
	fputs("\tinstall_ints();\n", out);
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

	char line[128], *p, *s;
	while (fgets(line, sizeof(line), in)) {
		fputs(line, out);
		if (strstr(line, "ZDELETE_PREVIOUS_CHAR"))
			break;
	}
	fputs("};\n\n", out);

	fputs("void bind_init(void)\n{\n", out);

	while (fgets(line, sizeof(line), in)) {
		for (p = line; isspace(*p); ++p) ;
		if (*p == '[') {
			if ((s = strchr(p, ','))) *s = ';';
			fprintf(out, "\tKeys%s", p);
		} else if (*p == '}') {
			hack(out);
			fputs("}\n", out);
			break;
		} else
			fputs(line, out);
	}

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
