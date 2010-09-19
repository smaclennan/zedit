#include "z.h"

#if COMMENTBOLD

static struct comment *CPPhead, *CPPtail;	/* list of CPP statements */
static struct comment *COMhead, *COMtail;	/* list of Comments */

static struct comment *new_comment(struct mark *start, struct mark *end, int type)
{
	struct comment *new = calloc(sizeof(struct comment), 1);
	if (!new)
		return NULL;
	new->start = Bcremrk();
	new->end   = Bcremrk();

	new->type  = type;
	if (start)
		Mrktomrk(new->start, start);
	if (end)
		Mrktomrk(new->end,   end);

	return new;
}

/* Mark a new comment from start to end.
 * If start or end are NULL, use Point.
 */
static void NewComment(struct mark *start, struct mark *end)
{
	struct comment *new = new_comment(start, end, T_COMMENT);
	if (!new)
		return;

	if (!COMhead)
		COMhead = new;
	else
		COMtail->next = new;
	COMtail = new;
}

static void NewCPP(struct mark *start, struct mark *end, int type)
{
	struct comment *new = new_comment(start, end, type);
	if (!new)
		return;

	if (!CPPhead)
		CPPhead = new;
	else
		CPPtail->next = new;
	CPPtail = new;
}

/* Merge the COMlist and the CPPlist into the buffer->comments */
static void MergeComments(void)
{
	struct buff *buff = Curbuff;

	if (!CPPhead) {
		buff->comments = COMhead;
		return;
	}
	if (!COMhead) {
		buff->comments = CPPhead;
		return;
	}

	while (COMhead && CPPhead)
		if (Mrkaftermrk(CPPhead->start, COMhead->start)) {
			if (!buff->comments)
				buff->comments = COMhead;
			else
				buff->ctail->next = COMhead;
			buff->ctail = COMhead;
			COMhead = COMhead->next;
		} else {
			if (!buff->comments)
				buff->comments = CPPhead;
			else
				buff->ctail->next = CPPhead;
			buff->ctail = CPPhead;
			CPPhead = CPPhead->next;
		}

	while (COMhead) {
		buff->ctail->next = COMhead;
		buff->ctail = COMhead;
		COMhead = COMhead->next;
	}

	while (CPPhead) {
		buff->ctail->next = CPPhead;
		buff->ctail = CPPhead;
		CPPhead = CPPhead->next;
	}
}

#if 1
/* Highlight a CPP. */
static void CPPstatement(void)
{
	struct mark start;
	int type;

	Bmrktopnt(&start);

	/* Check for: if/elif/else/endif */
	/* WARNING: Getbword is too deadly to use here */
	do {
		Bmove1();			/* skip '#' and whitespace */
		if (Bisend())
			return;
	} while (Iswhite());

	if (Buff() == 'i' && Bmove1() && Buff() == 'f')
		type = T_CPPIF;
	else if (Buff() == 'e' && Bmove1() && (Buff() == 'l' || Buff() == 'n'))
		type = T_CPPIF;
	else
		type = T_CPP;

again:
	while (Buff() != '\n' && !Bisend()) {
		if (Buff() == '/') {
			Bmove1();
			if (Buff() == '*') {
				/* found comment start */
				Bmove(-2);
				NewCPP(&start, NULL, type);

				/* find comment end */
				if (Bsearch("*/", FORWARD)) {
					Bmrktopnt(&start);
					if (Bcsearch('\n')) {
						Bmove(-2);
						if (Buff() == '\\')
							goto again;
					}
				}
				return;
			} else if (Buff() == '/') {
				/* found c++ comment start */
				Bmove(-2);
				NewCPP(&start, NULL, type);

				/* find comment end */
				Bcsearch('\n');
				return;
			}
		} else
			Bmove1();
	}

	if (ISNL(Buff())) {
		/* check for continuation line */
		Bmove(-1);
		if (Buff() == '\\') {
			/* continuation line */
			Bmove(2);
			goto again;
		}
		Bmove1();

	}
	NewCPP(&start, NULL, type);
}
#endif

/* Remove all comments from buffer and mark unscanned */
static void UnComment(struct buff *buff)
{
	struct comment *com, *next;
	int i;

	for (com = buff->comments; com; com = next) {
		Unmark(com->start);
		Unmark(com->end);
		next = com->next;
		free(com);
	}
	buff->comstate = 0;
	buff->comments = 0;

	for (i = 0; i < Tmaxrow() - 2; ++i)
		Scrnmarks[i].modf = 1;
}

/* Scan an entire buffer for comments. */
static void ScanBuffer(void)
{
	struct mark tmark;
	struct mark start;
	char comchar = (Curbuff->bmode & ASMMODE) ? Curbuff->comchar : 0;

	COMhead = COMtail = CPPhead = CPPtail = 0;

	/* free existings comments */
	while (Curbuff->comments) {
		struct comment *com = Curbuff->comments;
		Curbuff->comments = Curbuff->comments->next;
		Unmark(com->start);
		Unmark(com->end);
		free(com);
	}

	Bmrktopnt(&tmark);

	Btostart();
	if (comchar)
		while (Bcsearch(comchar) && !Bisend()) {
			/* mark to end of line as comment */
			Bmove(-1);
			Bmrktopnt(&start);
			Bcsearch('\n');
			Bmove(-1);
			NewComment(&start, NULL);
		}
	else
		/* Look for both C and C++ comments. */
		while (Bcsearch('/') && !Bisend()) {
			if (Buff() == '*') {
				Bmove(-1);
				Bmrktopnt(&start);
				if (Bsearch("*/", FORWARD)) {
					Bmove1();
					NewComment(&start, NULL);
				}
			} else if (Buff() == '/') {
				Bmove(-1);
				Bmrktopnt(&start);
				Toendline();
				NewComment(&start, NULL);
			}
		}

	/* find CPP statements */
	Btostart();
	do {
		if (Buff() == '#')
			CPPstatement();
		else if (comchar) {
			/* for assembler */
			Movepast(Iswhite, 1);
			if (Buff() == '.')
				CPPstatement();
		}
	} while (Bcsearch('\n') && !Bisend());

	MergeComments();
	Bpnttomrk(&tmark);
}

/* The following are called by the Innerdsp routine. */
static struct comment *start;

/* Called from Innerdsp before display loop */
void ResetComments(void)
{
	if (DelcmdAll()) {
		for (start = Curbuff->comments; start; start = start->next)
			if (Markch(start->end) != '/') {
				UnComment(Curbuff);
				break;
			}
	} else if (Lfunc == ZYANK)
		UnComment(Curbuff);
	start = Curbuff->comments;
}

/* Called from Innerdsp before each char displayed. */
void CheckComment(void)
{
	if (!Curbuff->comstate) {
		if (!(Curbuff->bmode & (CMODE | ASMMODE)))
			return;
		ScanBuffer();
		Curbuff->comstate = 1;
		start = Curbuff->comments;
	}
	for ( ; start; start = start->next)
		if (Bisbeforemrk(start->start))
			break;
		else if (Bisbeforemrk(start->end) || Bisatmrk(start->end)) {
			Tstyle(start->type);
			return;
		}

	Tstyle(T_NORMAL);
}

/* Called from Zcinsert when end comment entered */
void AddComment(void)
{
	ScanBuffer();
}

/* Called from Zcinsert when '#' entered at start of line */
void AddCPP(void)
{
	ScanBuffer();
}

/* Called from Zredisplay */
void Recomment(void)
{
	struct buff *buff;

	for (buff = Bufflist; buff; buff = buff->next)
		UnComment(buff);
}
#endif
