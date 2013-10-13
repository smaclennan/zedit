#include "../z.h"
#include "../cnames.h"
#include "../vars-array.h"

long Locs[ NUMFUNCS ];

#define COMMANDS	1
#define VARIABLES	2

static bool process(FILE *fp, int type);

int main(int argc, char *argv[])
{
	FILE *fp;
	char buff[ BUFSIZ ];
	int err, i;

	if(argc < 3 || (*argv[1] != 'c' && *argv[1] != 'v')) {
		puts("usage: zhsort {c | v} file");
		exit(2);
	}
	if(!(fp = fopen(argv[2], "r"))) {
		puts("unable to open file");
		exit(2);
	}

	memset(Locs,  0xff, sizeof(long) * NUMFUNCS);
	switch(*argv[1]) {
	case 'c':
		err = process(fp, COMMANDS);
		for( i = 0; i < NUMFUNCS; ++i )
			if( Locs[i] == -1 ) {
				err = true;
				fprintf( stderr, "%s: %s not found\n", argv[2], Cnames[i].name);
			} else if(fseek(fp, Locs[i], 0) == 0) {
				fgets( buff, BUFSIZ, fp );
				do
					printf( "%s", buff );
				while( fgets(buff, BUFSIZ, fp) && *buff != ':' );
			} else {
				fprintf( stderr, "Bad seek to %ld\n", Locs[i] );
				exit( 1 );
			}
			break;

	case 'v':
		err = process(fp, VARIABLES);
		for( i = 0; i < NUMVARS; ++i )
			if( Locs[i] == -1 ) {
				err = true;
				fprintf(stderr, "%s: %s not found\n", argv[2], Vars[i].vname);
			} else if( fseek(fp, Locs[i], 0) == 0 ) {
				fgets( buff, BUFSIZ, fp );
				do
					printf( "%s", buff );
				while( fgets(buff, BUFSIZ, fp) && *buff != ':' );
			} else {
				fprintf( stderr, "Bad seek to %ld\n", Locs[i] );
				exit( 1 );
			}
		break;
	}
	fclose(fp);
	exit( err );
}

static void massage(char *buff)
{
	while (*buff) {
		if (*buff == ' ' || *buff == '/')
			*buff = '-';
		else if (isupper(*buff))
			*buff = tolower(*buff);
		else if (*buff == '\n')
			*buff = '\0';
		++buff;
	}
}

static bool process(FILE *fp, int type)
{
	char buff[BUFSIZ];
	int err = 0, i;
	long loc = 0;

	while( fgets(buff, BUFSIZ, fp) )
	{
		if( *buff == ':' )
		{
			massage(buff);
			switch(type)
			{
			case COMMANDS:
				for( i = 0; i < NUMFUNCS; ++i ) {
					if( strcmp(Cnames[i].name, &buff[1]) == 0 )
					{	/* found it! */
						Locs[ i ] = loc;
						break;
					}
				}
				if(i == NUMFUNCS)
				{
					fprintf(stderr, "unknown command: '%s'\n", &buff[1]);
					err = true;
				}
				break;

			case VARIABLES:
				for( i = 0; i < NUMVARS; ++i )
					if( strcmp(Vars[i].vname, &buff[1]) == 0 )
					{
						Locs[ i ] = loc;
						break;
					}
				if(i == NUMVARS)
				{
					fprintf(stderr, "unknown variable: '%s'\n", &buff[1]);
					err = true;
				}
				break;
			}
		}
		loc = ftell( fp );
	}
	return( err );
}
