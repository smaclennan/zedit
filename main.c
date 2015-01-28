#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include "buff.h"

int main(int argc, char *argv[])
{
	struct buff *buff;

	binit();
	buff = bcreate();
	bswitchto(buff);
	binstr("Hello world");
	btostart();
	if (bm_search("hello", false))
		puts("found it!");
	btostart();
	if (bm_search("hello", true))
		puts("Ummm.. shouldn't have found it");
	bdelbuff(buff);
	bfini();
	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall main.c buff.c bmsearch.c -o main"
 * End:
 */
