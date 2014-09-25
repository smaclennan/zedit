/* config.h - Zedit user configurable defines */

#if !defined(WIN32) && !defined(DOS)
#define ZLIB		1	/* Requires zlib -lz */
#define SPELL		1	/* Requires libaspell */
#define GPM_MOUSE	1	/* Requires libgpm */
#endif

#define UNDO		1	/* EXPERIMENTAL, but portable! */
#define SHOW_REGION	1

/* DOPIPES: Output in real time and you can continue editing.
 * DOPOPEN: Output in real time but you must wait until command complete.
 * default: No output or editing until command complete.
 */
#if defined(__unix__)
#define DOPIPES 1
#elif !defined(DOS)
#define DOPOPEN 1
#endif
