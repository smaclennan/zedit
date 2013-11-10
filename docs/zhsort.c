#include "../z.h"
#include "../cnames.h"
#include "../vars-array.h"

static int usage(void)
{
	puts("usage: zhsort {c | v}");
	exit(2);
}

static int do_commands(void)
{
	int i;

	for( i = 0; i < NUMFUNCS; ++i ) {
		printf(":%s\n", Cnames[i].name);
		fputc('\n', stdout);
		fputs(Cnames[i].doc, stdout);
		fputs(".sp 0\n", stdout);
	}

	return 0;
}

static int do_variables(void)
{
	int i;

	for( i = 0; i < NUMVARS; ++i ) {
		printf(":%s\n", Vars[i].vname);
		puts(Vars[i].doc);
		fputs(".sp 0\n", stdout);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		usage();

	switch (*argv[1]) {
	case 'c':
		return do_commands();
	case 'v':
		return do_variables();
	default:
		return usage();
	}
}
