#define FCHECK
#define HAVE_MARKS
#define HAVE_GLOBAL_MARKS
#define UNSIGNED_BYTES
#include "../z.h"
#include "../keys.h"

void hang_up(int sig) {}
int InPaw;

#include "../bind.c"
#include "../cnames.c"
#include "../funcs.c"
#include "../varray.c"

static char *heading[] = { "Appendix A: Commands", "Appendix B: Variables" };
static char *fnames[] = { "app-a.html", "app-b.html" };

static char *appanotes =
"<p>After the command there may be letters in brackets. For example (+p).\n"
"A plus sign (+) means a Universal Argument works normally. i.e. It runs \n"
"the command Arg times. A minus sign (-) means that a Universal Argument is\n"
"ignored. A p means that the command works in the PAW\n"
"<p>Note: The bindings shown are the Zedit defaults and may be overridden\n"
"by a bindings file or the bind command.\n";

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

	fputs("strong {\n", out);
	fputs("font-size: 1.7em;\n", out);
	fputs("font-weight: bold;\n", out);
	fputs("line-height: 125%;\n", out);
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

static char *dispkey(unsigned key, char *s)
{
	char *p;
	int j;

	*s = '\0';
	if (is_special(key))
		return strcpy(s, special_label(key));
	if (key > 127)
		strcpy(s, key < 256 ? "M-" : "C-X ");
	j = key & 0x7f;
	if (j == 27)
		strcat(s, "ESC");
	else if (j < 32 || j == 127) {
		strcat(s, "C-");
		p = s + strlen(s);
		*p++ = j ^ '@';
		*p = '\0';
	} else if (j == 32)
		strcat(s, "Space");
	else {
		p = s + strlen(s);
		*p++ = j;
		*p = '\0';
	}
	return s;
}

static void dump_bindings(FILE *out, int fnum)
{
	int k, found = 0;
	char buff[BUFSIZ];

	if (fnum == ZNOTIMPL || fnum == ZINSERT)
		return;

	fputs("\n<p>Binding(s): ", out);

	for (k = 0; k < NUMKEYS; ++k)
		if (Keys[k] == fnum) {
			if (found)
				fputs(",  ", out);
			else
				found = true;
			fputs(dispkey(k, buff), out);
		}

	if (!found)
		fputs("Unbound", out);
}

static void out_one(FILE *out, const char *hdr, const char *opt, const char *doc)
{
	fprintf(out, "<p><strong class=large>%s</strong>", hdr);
	if (opt && *opt)
		fputs(opt, out);
	fprintf(out, "\n<p>");
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

		if (j == 0) {
			fputs(appanotes, out);

			for( i = 0; i < NUMFUNCS; ++i ) {
				char flags[20];

				bool pawok = Cmds[Cnames[i].fnum][1] != Znotimpl;
				if (pawok || Cnames[i].flags) {
					int n = sprintf(flags, " &nbsp;&nbsp;(");
					if (Cnames[i].flags)
						sprintf(flags + n, "%c", Cnames[i].flags);
					if (pawok)
						strcat(flags, "p");
					strcat(flags, ")");
				} else
					*flags = '\0';

				out_one(out, Cnames[i].name, flags, Cnames[i].doc);

				dump_bindings(out, Cnames[i].fnum);
			}
		} else
			for( i = 0; i < NUMVARS; ++i ) {
				out_one(out, Vars[i].vname, NULL, Vars[i].doc);
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
