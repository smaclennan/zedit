#include <stdio.h>

int main(int argc, char *argv[])
{
	int c;

	while((c = getchar()) != EOF)
		switch(c)
		{
		case '_':
			if((c = getchar()) != '\b')
			{
				ungetc(c, stdin);
				putchar('_');
			}
			break;
		case '\033':
			if((c = getchar()) != '9') ungetc(c, stdin);
			break;
		default:
			putchar(c);
			break;
		}

	return 0;
}
