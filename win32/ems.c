#include "z.h"

#ifdef DOS_EMS

#include <dos.h>

#if PSIZE != 1024
#error PSIZE must be 1024
#endif


/* Configurable */
#define MIN_PAGES 32	/* 512k */
#define MAX_PAGES 256	/* 4M */


/***************************************************************
 * LOW LEVEL ROUTINES
 ***************************************************************/

#define EMMPAGESZ 16384 /* EMM pages are 16k */
#define PSHIFT 10

static int EMMhandle;

typedef unsigned long Dword;
typedef unsigned int Word;

struct emm_move {
	Dword region_length;
	Byte source_memory_type;
	Word source_handle;
	Word source_initial_offset;
	Word source_initial_seg_page;
	Byte dest_memory_type;
	Word dest_handle;
	Word dest_initial_offset;
	Word dest_initial_seg_page;
};

/* Returns 0 if EMM unavailable, else pages allocated. */
static int EMMinit(void)
{
	char *name;
	int pages;
	union REGS regs;
	struct SREGS sregs;

	/* check if EMM driver exists */
	regs.x.ax = 0x3567;
	segread(&sregs);
	intdosx(&regs, &regs, &sregs);
	FP_SEG(name) = sregs.es;
	FP_OFF(name) = 10;
	if (strncmp(name, "EMMXXXX0", 8))
		return 0;

	/* EMM driver exists - check status of hardware/software */
	regs.h.ah = 0x40;
	int86(0x67, &regs, &regs);
	if (regs.h.ah)
		return 0;

	/* We need version 4.0 */
	regs.h.ah = 0x46;
	int86(0x67, &regs, &regs);
	if ((regs.h.al >> 4) < 4)
		return 0;

	/* Get available pages */
	regs.h.ah = 0x42;
	int86(0x67, &regs, &regs);
	if (regs.h.ah)
		return 0;

	pages = regs.x.bx;
	if (pages < MIN_PAGES) {
		Dbg("Not enough pages: %d\n", pages);
		return 0;
	}
	if (pages > MAX_PAGES)
		pages = MAX_PAGES;

	/* alloc the pages */
	regs.x.bx = pages;
	regs.h.ah = 0x43;
	int86( 0x67, &regs, &regs );
	if (regs.h.ah)
		return 0;

	EMMhandle = regs.x.dx;
	return pages;
}

/* free the handle and associated pages */
static void EMMfree(void)
{
	if (EMMhandle) {
		union REGS regs;

		regs.h.ah = 0x45;
		regs.x.dx = EMMhandle;
		int86(0x67, &regs, &regs);
		EMMhandle = 0;
	}
}

static int EMMread(int lpage, Byte *data)
{
	struct emm_move ems;
	union REGS regs;
	struct SREGS sregs;

	ems.region_length = EMMPAGESZ;
	ems.source_memory_type = 1;
	ems.source_handle = EMMhandle;
	ems.source_initial_offset = 0;
	ems.source_initial_seg_page = lpage;
	ems.dest_memory_type = 0;
	ems.dest_handle = 0;
	ems.dest_initial_offset = FP_OFF(data);
	ems.dest_initial_seg_page = FP_SEG(data);

	sregs.ds = FP_SEG(&ems);
	regs.x.si = FP_OFF(&ems);

	regs.x.ax = 0x5700;
	int86x(0x67, &regs, &regs, &sregs);

	return regs.h.ah;
}

static int EMMwrite(int lpage, Byte *data)
{
	struct emm_move ems;
	union REGS regs;
	struct SREGS sregs;

	ems.region_length = EMMPAGESZ;
	ems.source_memory_type = 0;
	ems.source_handle = 0;
	ems.source_initial_offset = FP_OFF(data);
	ems.source_initial_seg_page = FP_SEG(data);
	ems.dest_memory_type = 1;
	ems.dest_handle = EMMhandle;
	ems.dest_initial_offset = 0;
	ems.dest_initial_seg_page = lpage;

	sregs.ds = FP_SEG(&ems);
	regs.x.si = FP_OFF(&ems);

	regs.x.ax = 0x5700;
	int86x(0x67, &regs, &regs, &sregs);

	return regs.h.ah;
}

/***************************************************************
 * BUFFER LEVEL ROUTINES
 ***************************************************************/

static unsigned page_masks[MAX_PAGES];
int ems_pages;
static int pim;
static int pim_modf;

static Byte emmpage[EMMPAGESZ];

void ems_init(void)
{
	ems_pages = EMMinit();
	if (ems_pages)
		/* Read the first page */
		EMMread(pim, emmpage);
}

void ems_free(void)
{
	EMMfree();
}

bool ems_newpage(struct page *page)
{
	int i, j;

	if (EMMhandle == 0) {
		page->pdata = malloc(PSIZE);
		return page->pdata != NULL;
	}

	/* Find a free page */
	for (i = pim; i < ems_pages; ++i)
		if (page_masks[i] != 0xffff)
			for (j = 0; j < 16; ++j)
				if ((page_masks[i] & (1 << j)) == 0)
					goto found;
	for (i = 0; i < pim; ++i)
		if (page_masks[i] != 0xffff)
			for (j = 0; j < 16; ++j)
				if ((page_masks[i] & (1 << j)) == 0)
					goto found;
	return false; /* out of pages */

found:
	page_masks[i] |= (1 << j);
	page->emmpage = i;
	page->emmoff = j;
	return true;
}

void ems_freepage(struct page *page)
{
	if (EMMhandle)
		page_masks[page->emmpage] &= ~(1 << page->emmoff);
}

void ems_makecur(struct page *page, bool curmodf)
{
	if (EMMhandle == 0)
		return;

	if (curmodf)
		pim_modf = true;

	if (page->emmpage != pim) {
		if (pim_modf) {
			EMMwrite(pim, emmpage);
			pim_modf = false;
		}
		pim = page->emmpage;
		EMMread(pim, emmpage);
	}

	/* Even if in memory it may not be assigned */
	page->pdata = emmpage + (page->emmoff << PSHIFT);
}

void ems_pagesplit(struct page *newp, bool curmodf)
{
	if (newp->emmpage == pim) {
		/* both pages in same emm page */
		ems_makecur(newp, false); /* need to init pdata */
		memmove(newp->pdata, Curpage->pdata + HALFP, HALFP);
	} else {
		/* Need jump buffer to span emm pages */
		Byte jump[HALFP];
		struct page *cur = Curpage;

		memcpy(jump, Curpage->pdata + HALFP, HALFP);
		ems_makecur(newp, curmodf);
		memcpy(newp->pdata, jump, HALFP);
		ems_makecur(cur, true);
	}
}

#endif /* DOS_EMS */
