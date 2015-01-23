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
	binit();
	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall main.c buff.c -o main"
 * End:
 */
