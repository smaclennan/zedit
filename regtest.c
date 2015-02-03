#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "buff.h"

#define ESIZE 256

static void zgrepit(Byte *ebuf, const char *fname)
{
	while (step(ebuf)) {
		printf("%s:", fname);
		tobegline();
		while (*Curcptr != '\n' && !bisend()) {
			putchar(*Curcptr);
			bmove1();
		}
		putchar('\n');
		bmove1();
	}
}

/* Poor man's grep using Zedit regex functions. */
int main(int argc, char *argv[])
{
	if (argc < 2) {
		puts("I need a regular expression and a file.");
		exit(1);
	}

	Byte ebuf[ESIZE];

	int rc = compile((Byte *)argv[1], ebuf, &ebuf[ESIZE]);
	if (rc) {
		puts(regerr(rc));
		exit(1);
	}

	binit(NULL);
	struct buff *buff = bcreate();
	if (!buff)
		exit(2);
	bswitchto(buff);

	int arg;
	for (arg = 2; arg < argc; ++arg) {
		bempty();
		if (breadfile(argv[arg]) == 0)
			zgrepit(ebuf, argv[arg]);
		else
			printf("Unable to read %s\n", argv[arg]);
	}

	bfini();

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -g -Wall regtest.c buff.c reg.c -o regtest"
 * End:
 */
