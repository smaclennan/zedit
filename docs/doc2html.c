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

/* In practice the lists are two column tables */
static void do_list(FILE *in, FILE *out)
{
	char line[128], *p;

	fputs("<p><table border=1>\n", out);
	while (fgets(line, sizeof(line), in))
	       if (strcmp(line, ".)l\n") == 0)
		       break;
	       else {
		       for (p = line; *p && !isspace(*p); ++p) ;
		       *p++ = '\0';
		       while (isspace(*p)) ++p;
		       fprintf(out, "<tr><td>%s<td>%s", line, p);
	       }
	fputs("</table>\n", out);
}

static void help(FILE *in, FILE *out)
{
	char line[128];

	while (fgets(line, sizeof(line), in))
	       if (strcmp(line, ".sp 0\n") == 0)
		       break;
	       else printf("Skipping: %s", line);

	/* SAM Need doc about help in the manual proper */
	fputs("<p>Enters the help subsytem if help is enabled.\n"
	      "Help requires the help.z file to exist, usually in\n"
	      "/usr/share/zedit/help.z\n\n"
	      , out);
}

int main(int argc, char *argv[])
{
	int c;
	char *heading = NULL;

	while ((c = getopt(argc, argv, "H:")) != EOF)
		if (c == 'H')
			heading = optarg;
		else {
			puts("Sorry!");
			exit(1);
		}

	if (optind == argc) {
		puts("I need a file.");
		exit(1);
	}

	FILE *in = fopen(argv[optind], "r");
	if (!in) {
		perror(argv[optind]);
		exit(1);
	}

	char outname[255], *p;
	snprintf(outname, sizeof(outname) - 4, "%s", argv[optind]);
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

	if (heading)
		fprintf(out, "<h1>%s</h1>\n\n", heading);

	char line[128];
	int need_para = 0;
	while (fgets(line, sizeof(line), in)) {
		if ((p = strrchr(line, '\n'))) *p = '\0';
		if (*line == ':')
			fprintf(out, "<p><h3>%s</h3>\n", line + 1);
		else if (*line == '\0')
			need_para = 1;
		else if (strcmp(line, ".sp 0") == 0)
			fputs("\n", out);
		else if (strcmp(line, ".(l") == 0) {
			do_list(in, out);
			need_para = 1;
		} else if (strcmp(line, ".(b C") == 0)
			help(in, out);
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
