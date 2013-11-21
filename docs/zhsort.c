#include "../z.h"
#include "../cnames.h"
#include "../vars-array.h"

static int usage(void)
{
	puts("usage: zhsort {c | v}");
	exit(2);
}

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

static void query_replace(char *doc)
{
	for (; *doc; ++doc)
		if (*doc == '\n' && *(doc + 1) == '\n') {
			printf("\n<p><table border=1>\n");
			while (1) {
				fputs("<tr><td>", stdout);
				while (*doc && isspace(*doc)) ++doc;
				while (*doc && !isspace(*doc)) putchar(*doc++);
				while (*doc && isspace(*doc)) ++doc;
				fputs("<td>", stdout);
				while (*doc && *doc != '\n') putchar(*doc++);
				putchar('\n');
				if (doc) ++doc;
				if (*doc == '\n')
					break;
			}
			puts("</table>\n<p>");
		} else
			putchar(*doc);
}

static void out_one(char *hdr, char *doc)
{
	fprintf(stdout, "<h3>%s</h3>\n", hdr);
	fprintf(stdout, "<p>");
	if (strcmp(hdr, "query-replace") == 0)
		query_replace(doc);
	else
		for (;*doc;++doc)
			if (*doc == '\n')
				printf("<br>\n");
			else
				putchar(*doc);
	putchar('\n');
}

int main(int argc, char *argv[])
{
	int c, i, cmds = 0;
	char *heading = NULL;

	while ((c = getopt(argc, argv, "cvH:")) != EOF)
		switch (c) {
		case 'c':
			cmds = 1;
			break;
		case 'v':
			break;
		case 'H':
			heading = optarg;
			break;
		default:
			puts("Sorry!");
			exit(1);
		}

	header(stdout);

	if (heading)
		fprintf(stdout, "<h1>%s</h1>\n\n", heading);

	if (argc == 1)
		usage();

	if (cmds)
		for( i = 0; i < NUMFUNCS; ++i )
			out_one(Cnames[i].name, Cnames[i].doc);
	else
		for( i = 0; i < NUMVARS; ++i )
			out_one(Vars[i].vname, Vars[i].doc);

	footer(stdout);
	return 0;
}
