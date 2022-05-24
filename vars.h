/* Copyright (C) 1988-2016 Sean MacLennan */

#ifndef _VARS_H_
#define _VARS_H_

/** @addtogroup zedit
 * @{
 */

enum v_type { V_STRING, V_DECIMAL, V_FLAG };

/* The variable structure. */
struct avar {
	const char *vname;
	enum v_type vtype;
	union {
		char *str;
		int val;
	} u;
	const char *doc;
};
extern struct avar Vars[];
/* getplete structs must be aligned */
_Static_assert(((sizeof(struct avar) % sizeof(char *)) == 0),
	       "struct avar not aligned");

/* Handy macros */
#define VAR(n)		Vars[n].u.val
#define VARSTR(n)	Vars[n].u.str

/* var defines */
#define VCEXTS		0
#define VCTABS		(VCEXTS + 1)
#define VCOMMENTS	(VCTABS + 1)
#define VEXACT		(VCOMMENTS + 1)
#define VFILLWIDTH	(VEXACT + 1)
#define VMAKE		(VFILLWIDTH + 1)
#define VSEXTS		(VMAKE + 1)
#define VSHTABS		(VSEXTS + 1)
#define VSPACETAB	(VSHTABS + 1)
#define VTEXTS		(VSPACETAB + 1)
#define VTABS		(VTEXTS + 1)
#define NUMVARS		(VTABS + 1)
/* @} */

#endif
