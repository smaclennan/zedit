/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"

/** @addtogroup zedit
 * @{
 */

static const char *const key_label[] = {
	"up", "down", "right", "left",
	"insert", "delete", "page up", "page down", "home", "end",
	"f1", "f2", "f3", "f4", "f5", "f6",
	"f7", "f8", "f9", "f10", "f11", "f12",
	"C-home", "C-end",
};

static const char *special_label(int key)
{
	if (key >= SPECIAL_START && key <= SPECIAL_END)
		return key_label[key - SPECIAL_START];
	else
		return "???";
}

static int is_special(int cmd)
{
	return cmd >= SPECIAL_START && cmd <= SPECIAL_END;
}

static char *dispkey(unsigned int key, char *s)
{
	char *p;
	int j;

	*s = '\0';
	if (is_special(key))
		return strcpy(s, special_label(key));
	if (key > 127)
		strcpy(s, key < 256 ? "C-X " : "M-");
	j = key & 0x7f;
	if (j == 27)
		strcat(s, "ESC");
	else if (j < 32 || j == 127) {
		strcat(s, "C-");
		p = s + strlen(s);
		*p++ = j ^ '@';
		*p = '\0';
	} else if (j == 32)
		strcat(s, "Space");
	else {
		p = s + strlen(s);
		*p++ = j;
		*p = '\0';
	}
	return s;
}

static void dump_bindings(int fnum)
{
	int k, found = 0;

	binstr(Bbuff, "\nBinding(s): ");

	for (k = 0; k < NUMKEYS; ++k)
		if (Keys[k] == fnum) {
			if (found) {
				binsert(Bbuff, ',');
				Cmd = ' ';
				Zfill_check();
			} else
				found = true;
			binstr(Bbuff, dispkey(k, PawStr));
		}

	if (!found)
		binstr(Bbuff, "Unbound");
}

void dump_doc(const char *doc)
{
	if (doc) {
		binstr(Bbuff, "\n\n");

		for (; *doc; ++doc)
			if (*doc == ' ') {
				Cmd = *doc;
				Zfill_check();
			} else
				binsert(Bbuff, *doc);
	}
	binsert(Bbuff, '\n');
}

void Zhelp(void)
{
	Cmd = delayprompt("Help: a=apropos f=function k=key v=variable");
	switch (Cmd) {
	case 'a':
		Zhelp_apropos();
		break;
	case 'f':
		Zhelp_function();
		break;
	case 'k':
		Zhelp_key();
		break;
	case 'v':
		Zhelp_variable();
		break;
	default:
		Zabort();
	}
}

void Zhelp_function(void)
{
	bool pawok;
	int rc = getplete("Function: ", NULL, (char **)Cnames,
			  CNAMESIZE, NUMFUNCS);
	if (rc == -1)
		return;

	if (Argp) {
		struct zbuff *buff = cmakebuff(HELPBUFF, NULL);
		if (buff == NULL)
			return;
		cswitchto(buff);
		bempty(Bbuff);
	} else
		wuseother(HELPBUFF);

	binstr(Bbuff, Cnames[rc].name);

	pawok = Cmds[Cnames[rc].fnum][1] != Znotimpl;
	if (pawok || Cnames[rc].flags) {
		binstr(Bbuff, " (");
		if (Cnames[rc].flags)
			binsert(Bbuff, Cnames[rc].flags);
		if (pawok)
			binsert(Bbuff, 'P');
		binsert(Bbuff, ')');
	}

	dump_doc(Cnames[rc].doc);
	dump_bindings(Cnames[rc].fnum);

	btostart(Bbuff);
	Bbuff->bmodf = false;
}

void Zhelp_variable(void)
{
	int rc = getplete("Variable: ", NULL, (char **)Vars,
			  sizeof(struct avar), NUMVARS);
	if (rc == -1)
		return;

	wuseother(HELPBUFF);

	binstr(Bbuff, "%s: ", Vars[rc].vname);
	switch (Vars[rc].vtype) {
	case V_STRING:
		binstr(Bbuff, VARSTR(rc) ? VARSTR(rc) : "NONE");
		break;
	case V_FLAG:
		binstr(Bbuff, VAR(rc) ? "On" : "Off");
		break;
	case V_DECIMAL:
		binstr(Bbuff, "%d", VAR(rc));
	}

	dump_doc(Vars[rc].doc);

	btostart(Bbuff);
	Bbuff->bmodf = false;
}

void Zhelp_apropos(void)
{
	char word[STRMAX], line[80];
	int i, j, n, match = 0;

	*word = '\0';
	if (getarg("Word: ", word, sizeof(word)))
		return;

	for (i = j = n = 0; i < NUMFUNCS; ++i)
		if (strstr(Cnames[i].name, word)) {
			if (match++ == 0)
				wuseother(HELPBUFF);
			n += strfmt(line + n, sizeof(line) - n,
				    "%-24s", Cnames[i].name);
			if (++j == 3) {
				binstr(Bbuff, "%s\n", line);
				j = n = 0;
			}
		}

	if (j)
		binstr(Bbuff, "%s\n", line);

	if (match == 0)
		putpaw("No matches.");
	else {
		btostart(Bbuff);
		Bbuff->bmodf = false;
	}
}

void Zhelp_key(void)
{
	char kstr[12];
	int rc;
	unsigned int key, was = Cmd;

	Arg = 0;
	putpaw("Key: ");
	Cmd = tgetkb();
	if (Cmd == TC_UNKNOWN) {
		putpaw("Sorry, I don't recognize that key.");
		Cmd = was;
		return;
	}
	key = Keys[Cmd];
	if (key == ZCTRL_X) {
		putpaw("Key: C-X ");
		Cmd = toupper(tgetkb()) + CX(0);
		key = Keys[Cmd];
	} else if (key == ZMETA) {
		putpaw("Key: M-");
		Cmd = toupper(tgetkb()) + M(0);
		key = Keys[Cmd];
	}

	if (key == ZNOTIMPL)
		putpaw("%s (%03o) Unbound", dispkey(Cmd, kstr), Cmd);
	else
		for (rc = 0; rc < NUMFUNCS; ++rc)
			if (Cnames[rc].fnum == key)
				putpaw("%s (%03o) Bound to %s",
					dispkey(Cmd, kstr), Cmd,
					Cnames[rc].name);

	/* We need to set Cmd above since some keystrokes will be
	 * wrong if not set. Put it back to the original value.
	 */
	Cmd = was;
}
/* @} */
