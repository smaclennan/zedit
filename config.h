/* config.h - Zedit user configurable defines */

#if !defined(WIN32)
#define ZLIB		1	/* Requires zlib -lz */
#define SPELL		1	/* Requires libaspell */
#endif
#if defined(__linux__)
#define GPM_MOUSE	1	/* Requires libgpm */
#endif

#define UNDO		1	/* EXPERIMENTAL, but portable! */
#define SHOW_REGION	1

/* DOPIPES: Output in real time and you can continue editing.
 * default: Output in real time but you must wait until command complete.
 */
#if defined(__unix__)
// SAM #define DOPIPES 1
#endif
