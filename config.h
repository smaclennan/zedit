/* config.h - Zedit user configurable defines */

#if !defined(WIN32)
#define ZLIB		0	/* Requires zlib -lz */
#define SPELL		1	/* Requires libaspell */
#endif
#if defined(__linux__)
#define GPM_MOUSE	0
#endif

#define SHOW_REGION	1
#define UNDO        1

/* DOPIPES: Output in real time and you can continue editing.
 * default: Output in real time but you must wait until command complete.
 */
#if defined(__unix__)
#define DOPIPES 1
#endif
