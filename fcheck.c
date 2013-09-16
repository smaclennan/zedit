#include "z.h"
#include "cnames.h"
#include "vars-array.h"


int main(int argc, char *argv[])
{
	int err = 0;

	if (NUMVARS != VARNUM) {
		/* haven't setup term stuff yet */
		printf("Mismatch in NUMVARS and VARNUM %d:%d\n",
		       NUMVARS, VARNUM);
		err = 1;
	}

	/* check sizes of various stuff */
	int s1 = sizeof(Cnames) / sizeof(struct cnames);
	if (s1 != NUMFUNCS) {
		printf("Cnames: %d NUMFUNCS: %d\n", s1, NUMFUNCS);
		exit(1); /* stop since the loop below might segfault */
	}

	/* validate the Cnames array the best we can */
	for (s1 = 1; s1 < NUMFUNCS; ++s1) {
		if (strcasecmp(Cnames[s1].name,
			       Cnames[s1 - 1].name) <= 0) {
			printf("Problem: (%d) %s and %s\n",
			    s1, Cnames[s1 - 1].name, Cnames[s1].name);
			err = 1;
		}
		if (strlen(Cnames[s1].name) > (size_t)30) {
			printf("%s too long\n", Cnames[s1].name);
			err = 1;
		}
		if (strncmp(Cnames[s1].name, "Top", 3) == 0) {
			printf("Zhelp() Top: %s\n", Cnames[s1].name);
			err = 1;
		}
	}

	return err;
}
