/* This program calculate the shift table used in tk3d.c for the 3d
 * polygon function.
 */

#include <stdio.h>
#include <math.h>

main()
{
    static int shiftTable[129] = {0};
	int i;
	double tangent, cosine;

	printf("\tstatic int shiftTable[129] = {\n\t\t");
	for (i = 0; i <= 128; i++)
	{
	    tangent = i/128.0;
	    cosine = 128/cos(atan(tangent)) + .5;
	    shiftTable[i] = cosine;
		if(i == 128)
			printf("%d};\n", shiftTable[i]);
		else
		{
			printf("%d,", shiftTable[i]);
			if((i & 15) == 15) printf("\n\t\t");
		}
	}
}
