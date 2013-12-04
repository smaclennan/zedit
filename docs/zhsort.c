#include "../z.h"
#include "../cnames.h"
#include "../vars-array.c"

static char *heading[] = { "Appendix A: Commands", "Appendix B: Variables" };
static char *fnames[] = { "app-a.html", "app-b.html" };

static void header(FILE *out, char *heading)
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
	fprintf(out, "<h1>%s</h1>\n\n", heading);
}

static void footer(FILE *out)
{
	fputs("</body>\n", out);
	fputs("</html>\n", out);
}

static void query_replace(FILE *out, const char *doc)
{
	for (; *doc; ++doc)
		if (*doc == '\n' && *(doc + 1) == '\n') {
			fputs("\n<p><table border=1>\n", out);
			while (1) {
				fputs("<tr><td>", out);
				while (*doc && isspace(*doc)) ++doc;
				while (*doc && !isspace(*doc)) fputc(*doc++, out);
				while (*doc && isspace(*doc)) ++doc;
				fputs("<td>", out);
				while (*doc && *doc != '\n') fputc(*doc++, out);
				fputc('\n', out);
				if (doc) ++doc;
				if (*doc == '\n')
					break;
			}
			fputs("</table>\n<p>", out);
		} else
			fputc(*doc, out);
}

static void out_one(FILE *out, const char *hdr, const char *doc)
{
	fprintf(out, "<h3>%s</h3>\n", hdr);
	fprintf(out, "<p>");
	if (strcmp(hdr, "query-replace") == 0)
		query_replace(out, doc);
	else
		for (;*doc;++doc)
			if (*doc == '\n')
				fputs("<br>\n", out);
			else
				fputc(*doc, out);
	fputc('\n', out);
}

int main(int argc, char *argv[])
{
	int i, j;

	for (j = 0; j < 2; ++j) {
		FILE *out = fopen(fnames[j], "w");
		if (!out) {
			perror(fnames[j]);
			exit(1);
		}

		header(out, heading[j]);

		if (j == 0)
			for( i = 0; i < NUMFUNCS; ++i )
				out_one(out, Cnames[i].name, Cnames[i].doc);
		else
			for( i = 0; i < NUMVARS; ++i ) {
				out_one(out, Vars[i].vname, Vars[i].doc);
				switch (Vars[i].vtype) {
				case V_FLAG:
					fprintf(out, "<p>Default: %s\n", VAR(i) ? "on" : "off");
					break;
				case V_DECIMAL:
					fprintf(out, "<p>Default: %d\n", VAR(i));
					break;
				case V_STRING:
					if (VARSTR(i))
						fprintf(out, "<p>Default: %s\n", VARSTR(i));
					break;
				}
			}

		footer(out);
		fclose(out);
	}

	return 0;
}
