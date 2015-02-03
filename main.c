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

	binit(NULL);
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

	Byte ep[512];
	compile((Byte *)"el.l", ep, &ep[512]);
	btostart();
	step(ep);
	return 0;
}
