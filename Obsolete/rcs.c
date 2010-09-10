#include "z.h"

#if USE_RCS

static int RCSCheckOut();

/* Checkout the current buffer locked! */
/* SAM MARK AS CHECKEDOUT! */
Proc Zrcsco()
{
	if(Curbuff->fname)
	{
		if(access(Curbuff->fname, R_OK | W_OK) == 0 &&
			Ask("File already writable. Checkout? ") != YES)
				return;
		if(!Promptsave(Curbuff, FALSE)) return;
		if(RCSCheckOut(Curbuff->fname) == 0)
		{	/* Save current point location */
			unsigned long offset = Blocation(0);
			
			/* Reread the file */
			Breadfile(Curbuff->fname);
			Curbuff->bmode &= ~VIEW;
			
			/* Go back */
			Boffset(offset);

#if XWINDOWS
			Zredisplay();
#endif
		}
	}
}


int CheckRCS(fname, locked)
char *fname;
Boolean locked;
{
	char rcsname[PATHMAX + 10], *p;

	if(access(fname, F_OK) == 0) return 0;	/* file exists */

	strcpy(rcsname, fname);	
	if((p = strrchr(rcsname, '/')) != NULL)
	{
		int len = strlen(p) + 1;	/* copy the null */
		memmove(p + 4, p, len);
		memcpy(p, "/RCS", 4);
		strcat(rcsname, ",v");
	}
	else
		sprintf(rcsname, "RCS/%s,v", fname);

	if(access(rcsname, F_OK) == 0)
	{
		sprintf(PawStr, "Checkout %s from RCS? ", fname);
		if(Ask(PawStr) == YES)
			return RCSCheckOut(fname);
	}

	return 0;
}


static int RCSCheckOut(fname)
char *fname;
{
	extern char *Cwd;
	char call[PATHMAX + 10], *p;
	int rc, changed;

	if((p = strrchr(fname, '/')))
	{	/* change to directory */
		*p = '\0';
		chdir(fname);
		*p++ = '/';		/* put it back */
		changed = 1;
	}
	else
	{
		p = fname;
		changed = 0;
	}

	sprintf(call, "co -l %s >/tmp/rcs 2>&1", p);
	if((rc = system(call)))
		Error("Unable to checkout file.");

	if(changed) chdir(Cwd);
	return rc;
}

#else

void Zrcsco() { Echo("Not supported."); }
#endif

Proc Zrcsci() { Echo("Not supported."); }
