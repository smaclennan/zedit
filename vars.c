/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"

/** @addtogroup zedit
 * @{
 */

static void setavar(const char *vin, bool display);

void Zset_variable(void)
{
	char pstr[STRMAX], arg[STRMAX];
	int rc = getplete("Variable: ", NULL, (char **)Vars,
			  sizeof(struct avar), NUMVARS);
	if (rc == -1)
		return;

	if (!Argp || Vars[rc].vtype == V_STRING) {
		strconcat(pstr, sizeof(pstr), Vars[rc].vname, ": ", NULL);
		if (Vars[rc].vtype == V_STRING)
			if (VARSTR(rc))
				strlcpy(arg, VARSTR(rc), sizeof(arg));
			else
				*arg = '\0';
		else
			strfmt(arg, sizeof(arg), "%d", VAR(rc));
		if (getarg(pstr, arg, STRMAX))
			return;
		strconcat(pstr, sizeof(pstr), Vars[rc].vname, " ", arg, NULL);
		setavar(pstr, true);
	} else
		setavar(Vars[rc].vname, true);
}

/* If there is a config.z file, read it! */
void readvfile(const char *fname)
{
	struct buff *buff = bcreate();

	if (!buff || breadfile(buff, fname, NULL)) {
		terror("Unable to read config file\n");
		exit(1);
	}

	while (bstrline(buff, PawStr, PAWSTRLEN))
		setavar(PawStr, false);

	bdelbuff(buff);
}

static void setit(int i, const char *ptr)
{
	switch (Vars[i].vtype) {
	case V_STRING:
		if (strcmp(VARSTR(i), ptr))
			VARSTR(i) = strdup(ptr);
		break;
	case V_FLAG:
		if (strncasecmp(ptr, "true", 4) == 0)
			VAR(i) = 1;
		else if (strncasecmp(ptr, "false", 5) == 0)
			VAR(i) = 0;
		else
			VAR(i) = strtol(ptr, NULL, 0);
		break;
	case V_DECIMAL:
		VAR(i) = strtol(ptr, NULL, 0);
	}
}

static void do_var_match(int i, const char *vin)
{
	const char *ptr;

	if (Argp && Vars[i].vtype != V_STRING)
		VAR(i) = Arg;
	else {
		for (ptr = vin; *ptr && !isspace(*ptr); ++ptr)
			;
		while (isspace(*ptr))
			++ptr;
		if (*ptr)
			setit(i, ptr);
	}

	/* Fillwidth must be > 0 */
	if (VAR(VFILLWIDTH) == 0)
		VAR(VFILLWIDTH) = 1;

	/* Tabs must be >= 2 and <= 8 */
	if (i == VTABS || i == VCTABS || i == VSHTABS) {
		if (VAR(i) < 2)
			VAR(i) = 2;
		else if (VAR(i) > 8)
			VAR(i) = 8;
	}
}

static void setavar(const char *vin, bool display)
{
	char msg[STRMAX + 1];
	int i = 0;

	strlcpy(msg, vin, sizeof(msg));
	strtok(msg, " \t");

	for (i = 0; i < NUMVARS; ++i)
		if (strcasecmp(msg, Vars[i].vname) == 0) {
			do_var_match(i, vin);
			if (display) {
				if (i == VTABS || i == VCTABS || i == VSHTABS) {
					settabsize(Curbuff->bmode);
					redisplay();
				}
				if (Vars[i].vtype == V_STRING) {
					if (VARSTR(i))
						putpaw("%s = %s",
							   Vars[i].vname,
							   VARSTR(i));
					else
						putpaw("%s = NONE",
							   Vars[i].vname);
				} else
					putpaw("%s = %d",
						Vars[i].vname, VAR(i));
				if (i == VCOMMENTS)
					redisplay();
			}
			break;
		}
	if (i == NUMVARS) {
		if (display)
			putpaw("Variable '%s' Not Found", vin);
	}
	Arg = 0;
}

/* Tabs must be >= 2 and <= 8. Enforced in setavar. */
int Tabsize = 8;

/* Returns non-zero if tab size changed */
int settabsize(unsigned int mode)
{
	switch (mode & MAJORMODE) {
	case CMODE:
		if (Tabsize == VAR(VCTABS))
			return 0;
		return Tabsize = VAR(VCTABS);
	case SHMODE:
		if (Tabsize == VAR(VSHTABS))
			return 0;
		return Tabsize = VAR(VSHTABS);
	default:
		if (Tabsize == VAR(VTABS))
			return 0;
		return Tabsize = VAR(VTABS);
	}
}
/* @} */
