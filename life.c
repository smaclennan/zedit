#include "z.h"

#if LIFE

#define SROWS	(tmaxrow() - 2)	/* Screen rows */
#define SCOLS	(tmaxcol() - 1)	/* Screen columns */
#define MATRIX	(SROWS * SCOLS)	/* Matrix buffer size */

#define NW	(-1 - SCOLS)	/* Directional constants, within */
#define N	(-SCOLS)	/* matrix 2. For example, NW */
#define NE	(1-SCOLS)	/* refers to the upper, left- */
#define E	(1)		/* hand neighbour */
#define SE	(1+SCOLS)
#define S	(SCOLS)
#define SW	(-1+SCOLS)
#define W	(-1)

#define LIFECH	'*'
#define EMPTY	' '
#define LIVING	'+'
#define DEAD	'X'

static char *matrix1, *matrix2;

/* Do one generation of life. First matrix 2 is cleared, then
 * matrix 1 is scanned. Wherever a living cell is found, the CORRESPONDING
 * NEIGHBOR CELLS IN MATRIX 2 are incremented by 1, and the corresponding
 * cell itself is incremented by 100. If the cell is not living, do nothing.
 * This provides a fast method of determining neighbor count, which is
 * kept track of in matrix 2.
 *
 * The "zone" of each cell is checked, and used as a guide for determining
 * neighbors. Nothern neighbors of northernmost row are found in the
 * southernmost row, so that the game has a "boundless" effect...formations
 * that move off one side automatically circle around to the other side.
 *
 * Pass 2 is called to determine what actually lives or dies, based on
 * the neighbor-count of each cell.
 */
static void generation(void)
{
	char *p1, *p2, *end;
	int msize, row = 0, col;

	/* Initialization */
	msize = MATRIX;
	end = matrix1 + msize;
	memset(matrix2, 0, msize);

	for (p1 = matrix1, p2 = matrix2; p1 < end; ++p1, ++p2) {
		/* If matrix 1 cell is alive . . . */
		if (*p1 > 100) {
			/* Update matrix 2 cell. */
			*p2 += 100;

			/* Get the zone and update the neighbors accordingly. */
			switch (*p1 - 100) {
			case 1:
				++*(p2 + NW);
				++*(p2 + N);
				++*(p2 + NE);
				++*(p2 + E);
				++*(p2 + SE);
				++*(p2 + S);
				++*(p2 + SW);
				++*(p2 + W);
				break;
			case 2:
				++*(p2 + NW + msize);
				++*(p2 + N + msize);
				++*(p2 + NE + msize);
				++*(p2 + E);
				++*(p2 + SE);
				++*(p2 + S);
				++*(p2 + SW);
				++*(p2 + W);
				break;
			case 3:
				++*(p2 + NW);
				++*(p2 + N);
				++*(p2 + NE);
				++*(p2 + E);
				++*(p2 + SE - msize);
				++*(p2 + S - msize);
				++*(p2 + SW - msize);
				++*(p2 + W);
				break;
			case 4:
				++*(p2 + NW + SCOLS);
				++*(p2 + N);
				++*(p2 + NE);
				++*(p2 + E);
				++*(p2 + SE);
				++*(p2 + S);
				++*(p2 + SW + SCOLS);
				++*(p2 + W + SCOLS);
				break;
			case 5:
				++*(p2 + NW);
				++*(p2 + N);
				++*(p2 + NE - SCOLS);
				++*(p2 + E - SCOLS);
				++*(p2 + SE - SCOLS);
				++*(p2 + S);
				++*(p2 + SW);
				++*(p2 + W);
				break;
			case 6:
				++*(p2 + NW + msize + SCOLS);
				++*(p2 + N + msize);
				++*(p2 + NE + msize);
				++*(p2 + E);
				++*(p2 + SE);
				++*(p2 + S);
				++*(p2 + SW + SCOLS);
				++*(p2 + W + SCOLS);
				break;
			case 7:
				++*(p2 + NW + msize);
				++*(p2 + N + msize);
				++*(p2 + NE + msize - SCOLS);
				++*(p2 + E - SCOLS);
				++*(p2 + SE - SCOLS);
				++*(p2 + S);
				++*(p2 + SW);
				++*(p2 + W);
				break;
			case 8:
				++*(p2 + NW + SCOLS);
				++*(p2 + N);
				++*(p2 + NE);
				++*(p2 + E);
				++*(p2 + SE - msize);
				++*(p2 + S - msize);
				++*(p2 + SW + SCOLS - msize);
				++*(p2 + W + SCOLS);
				break;
			case 9:
				++*(p2 + NW);
				++*(p2 + N);
				++*(p2 + NE - SCOLS);
				++*(p2 + E - SCOLS);
				++*(p2 + SE - msize - SCOLS);
				++*(p2 + S - msize);
				++*(p2 + SW - msize);
				++*(p2 + W);
				break;
			default:
				break;
			}
		}
	}

	/* pass2 - Scan matrix 2 and update matrix 1 according to the following:
	 *
	 *    Matrix 2 value        Matrix 1 result
	 *    --------------        ----------------------
	 *          3               Birth
	 *          other < 100     No change
	 *          102, 103        No change
	 *          other >= 100    Live cell becomes dead
	 */
	col = 0;
	for (btostart(), p1 = matrix1, p2 = matrix2; p1 < end; ++p1, ++p2) {
		if (*p2 == 3) {	/* Birth */
			*p1 += 100;
			Buff() = LIFECH;
			Scrnmarks[row].modf = true;
		} else if (*p2 >= 100 && *p2 != 102 && *p2 != 103) { /* Death */
			*p1 -= 100;
			Buff() = EMPTY;
			Scrnmarks[row].modf = true;
		}
		bmove1();
		if (Buff() == NL) {
			++row;
			col = 0;
			bmove1();
		} else
			++col;
	}
}


/* Initializes the zones (1-9) of matrix 1.
 *
 * The "zones" are used by the LIFE algorithm to determine the method
 * of calculating neighbors. Zones are pertinent to edges and corners:
 *
 *    +-+--------------+-+
 *    |6|      2       |7|
 *    +-+--------------+-+
 *    | |              | |
 *    |4|      1       |5|
 *    | |              | |
 *    +-+--------------+-+
 *    |8|      3       |9|
 *    +-+--------------+-+
 *
 * Zones are recorded in matrix 1 for ease of computation. If a cell
 * lives, then add 100 to flag cell's existence.
 */
static void init_matrix1(char *p1)
{
	int row, col;

	/* Initilialize row 1 to zones 6, 2, and 7. */
	*p1++ = 6;
	for (col = 2; col < SCOLS; col++)
		*p1++ = 2;
	*p1++ = 7;

	/* Initialize center rows to zones 4, 1, and 5. */
	for (row = 2; row < SROWS; row++) {
		*p1++ = 4;
		for (col = 2; col < SCOLS; ++col)
			*p1++ = 1;
		*p1++ = 5;
	}

	/* Initialize bottom row to zones 8, 3, and 9. */
	*p1++ = 8;
	for (col = 2; col < SCOLS; col++)
		*p1++ = 3;
	*p1++ = 9;
}


/*
  Go throught the current buffer and find any 'lives'. If a life is found,
  convert the buffer character to a '*' and fill in matrix1. While going
  though the buffer it is padded with spaces to fill SROWS * SCOLS
*/
static void fill_matrix1(char *p1)
{
	bool eol;
	int row = 0, col = 0;

	btostart();
	for (row = 0; row < SROWS; ++row) {
		for (eol = false, col = 0; col < SCOLS; ++col, ++p1)
			if (eol || bisend() || Buff() == NL) {
				/* at end of buffer line */
				binsert(EMPTY);
				eol = true;
			} else {
				if (isspace(Buff()))
					Buff() = EMPTY;
				else {
					*p1 += 100;
					Buff() = LIFECH;
				}
				bmove1();
			}

		if (!bisend() && Buff() == NL)
			bmove1();
		else
			binsert(NL);

		Scrnmarks[row].modf = true;
	}
}

void Zlife(void)
{
	bool go = true, step = true;
	unsigned cmd;

	if (!promptsave(Curbuff, false))
		return;

	matrix1 = malloc(MATRIX);
	matrix2 = malloc(MATRIX);
	if (!matrix1 || !matrix2) {
		if (matrix1)
			free(matrix1);
		if (matrix2)
			free(matrix2);
		error("Not enough memory.");
		return;
	}
	echo("Setting up...");

	init_matrix1(matrix1);
	fill_matrix1(matrix1);

	echo("The Game of Life");
	while (go) {
		btostart();
		zrefresh();
		if (step || tkbrdy()) {
			cmd = tgetcmd();
			if (cmd == CR)
				step = false;
			else if (Keys[cmd] == ZABORT)
				go = false;
			else
				step = true;
		} else
			usleep(100000);
		if (go)
			generation();
	}
	clrecho();
	free(matrix1);
	free(matrix2);
	Curbuff->bmodf = 0;
}
#else
void Zlife(void) { tbell(); }
#endif