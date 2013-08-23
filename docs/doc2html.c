#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>


static void header(FILE *out)
{
	fputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n", out);
	fputs("<html lang=\"en\">\n", out);
	fputs("<head>\n", out);
	fputs("<title>Zedit</title>\n", out);
	fputs("<style type=\"text/css\">\n", out);
	fputs("<!--\n", out);
	fputs("body {\n", out);
	fputs("background-color: #C0C0C0;\n", out);
	fputs("margin: 1em 8% 0 8%;\n", out);
	fputs("/* ie9 defaults to small */\n", out);
	fputs("font-size: medium;\n", out);
	fputs("}\n", out);
	fputs("-->\n", out);
	fputs("</style>\n", out);
	fputs("</head>\n", out);
	fputs("<body bgcolor=\"#C0C0C0\">\n", out);
}

static void footer(FILE *out)
{
	fputs("</body>\n", out);
	fputs("</html>\n", out);
}


int main(int argc, char *argv[])
{
	if (argc == 1) {
		puts("I need a file.");
		exit(1);
	}

	FILE *in = fopen(argv[1], "r");
	if (!in) {
		perror(argv[1]);
		exit(1);
	}

	char outname[255], *p;
	snprintf(outname, sizeof(outname) - 4, "%s", argv[1]);
	p = strrchr(outname, '.');
	if (!p)
		exit(1);
	strcpy(p, ".html");
	FILE *out = fopen(outname, "w");
	if (!out) {
		perror(outname);
		exit(1);
	}

	header(out);

	char line[128];
	int need_para = 0;
	while (fgets(line, sizeof(line), in)) {
		if ((p = strrchr(line, '\n'))) *p = '\0';
		if (*line == ':')
			fprintf(out, "<p><b>%s</b>\n", line + 1);
		else if (*line == '\0')
			need_para = 1;
		else if (strcmp(line, ".sp 0") == 0)
			fputs("\n", out);
		else {
			if (need_para) {
				fputs("<p>", out);
				need_para = 0;
			}
			fprintf(out, "%s\n", line);
		}
	}

	footer(out);

	fclose(in);
	fclose(out);

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall doc2html.c -o doc2html"
 * End:
 */
