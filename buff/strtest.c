#include <stdio.h> // for snprintf
#include <assert.h>
#include "buff.h"

int main(int argc, char *argv[])
{
	char str1[100], str2[100], str[100];
	int n1, n2, i;

	i = -1;
	n1 = snprintf(str1, sizeof(str1), "test %d and %u\n", i, i);
	n2 = strfmt(str2, sizeof(str2), "test %d and %u\n", i, i);
	assert(n1 == n2);
	assert(strcmp(str1, str2) == 0);

	i = 42;
	strcpy(str, "hello");
	n1 = snprintf(str1, sizeof(str1), "%s world %d\n", str, i);
	n2 = strfmt(str2, sizeof(str2), "%s world %d\n", str, i);
	assert(n1 == n2);
	assert(strcmp(str1, str2) == 0);

	n1 = snprintf(str1, sizeof(str1), "The quick brown fox\n");
	n2 = strfmt(str2, sizeof(str2), "The quick brown fox\n");
	assert(n1 == n2);
	assert(strcmp(str1, str2) == 0);

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -O2 -Wall strtest.c -o strtest libbuff.a"
 * End:
 */
