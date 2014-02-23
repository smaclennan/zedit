#include <conio.h>

void main()
{
	int cmd;
	
	do {
		cmd = getch();
		if (cmd == 0) {
			cmd = getch();
			printf("00 %02X\n", cmd);
		} else
			printf("%02X\n", cmd);
	} while (cmd != 'q');
}

	
