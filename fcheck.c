/* Copyright (C) 1988-2018 Sean MacLennan */

static int saw_err;

#ifdef BUILTIN_FCHECK
#include "z.h"

static void fcheck_err(const char *fmt, ...)
{
	// Only print first error
	if (saw_err) return;

	char line[128];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(line, sizeof(line), fmt, ap);
	va_end(ap);
	putpaw(line);
	tbell();
}

void Zfcheck(void)
#else
#define FCHECK
#include "z.h"

#include "varray.c"
#include "funcs.c"
#include "cnames.c"
#include "bind.c"

static void fcheck_err(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

int main(int argc, char *argv[])
#endif
{
	saw_err = 0;

	/* Spot check keys array */
	Byte array[97];
	memset(array, ZINSERT, sizeof(array));
	array[0] = ZUNDO;
	array[96] = ZDELETE_PREVIOUS_CHAR;
	if (memcmp(array, Keys + 31, sizeof(array))) {
		fcheck_err("Problems with Keys array 1\n");
		saw_err = 1;
	}

	if (Keys[CX('1')] != ZONE_WINDOW ||
	    Keys[CX('=')] != ZPOSITION ||
	    Keys[CX('O')] != ZNEXT_WINDOW ||
	    Keys[CX('Z')] != ZZAP_TO_CHAR ||
	    Keys[CX(127)] != ZDELETE_PREVIOUS_WORD) {
		fcheck_err("Problems with Keys array 2\n");
		saw_err = 1;
	}

	if (Keys[M('A')] != ZAGAIN ||
	    Keys[M('Z')] != ZSAVE_AND_EXIT ||
	    Keys[TC_UP] != ZPREVIOUS_LINE ||
	    Keys[TC_F12] != ZREVERT_FILE ||
	    Keys[M(127)] != ZDELETE_PREVIOUS_WORD) {
		fcheck_err("Problems with Keys array 3\n");
		saw_err = 1;
	}

	/* validate the Cnames array the best we can */
	for (int s1 = 1; s1 < NUMFUNCS; ++s1) {
		if (strcasecmp(Cnames[s1].name, Cnames[s1 - 1].name) <= 0) {
			fcheck_err("Problem: (%d) %s and %s\n",
				   s1, Cnames[s1 - 1].name, Cnames[s1].name);
			saw_err = 1;
		}
		if (strlen(Cnames[s1].name) > (size_t)30) {
			fcheck_err("%s too long\n", Cnames[s1].name);
			saw_err = 1;
		}
	}

	/* validate the Vars array */
	for (int s1 = 1; s1 < NUMVARS; ++s1)
		if (strcasecmp(Vars[s1].vname, Vars[s1 - 1].vname) <= 0) {
			fcheck_err("Problem: (%d) %s and %s\n",
				   s1, Vars[s1 - 1].vname, Vars[s1].vname);
			saw_err = 1;
		}

#ifdef BUILTIN_FCHECK
	putpaw("Ok");
#else
	return saw_err;
#endif
}
