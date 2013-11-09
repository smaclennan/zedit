#include "../z.h"
#include "../cnames.h"
#include "../vars-array.h"
#include <dirent.h>

long Locs[ NUMFUNCS ];

static char *cmd_docs[NUMFUNCS];

static char doc_buffer[4096];

#define COMMANDS	1
#define VARIABLES	2

static bool process(FILE *fp, int type);
static bool process_c_files(void);

int main(int argc, char *argv[])
{
	FILE *fp;
	char buff[ BUFSIZ ];
	int err = 1, i, n, maxlen = 0;

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
		err = process_c_files();
		err = process(fp, COMMANDS);
		for( i = 0; i < NUMFUNCS; ++i )
			if( cmd_docs[i] == NULL) {
				err = true;
				fprintf( stderr, "%s: %s not found\n", argv[2], Cnames[i].name);
			} else {
				printf(":%s\n", Cnames[i].name);
				fputc('\n', stdout);
				fputs(cmd_docs[i], stdout);
				fputs(".sp 0\n", stdout);
				n = strlen(cmd_docs[i]); // SAM DBG
				if (n > maxlen) maxlen = i; // SAM DBG
			}
		fprintf(stderr, "Max length %d\n", maxlen); // SAM DBG
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
	int err = 0, i, func = -1;
	long loc = 0;

	*doc_buffer = '\0';

	while( fgets(buff, BUFSIZ, fp) )
	{
		if( *buff == ':' )
		{
			if (func != -1) {
				cmd_docs[func] = strdup(doc_buffer);
				*doc_buffer = '\0';
			}
			func = -1;

			massage(buff);
			switch(type)
			{
			case COMMANDS:
				for( i = 0; i < NUMFUNCS; ++i ) {
					if( strcmp(Cnames[i].name, &buff[1]) == 0 )
					{	/* found it! */
						if (cmd_docs[i])
							fprintf(stderr, "DUP %s\n", Cnames[i].name);
						else
							func = i;
						break;
					}
				}
				if(i == NUMFUNCS)
				{
					fprintf(stderr, "unknown command: '%s'\n", &buff[1]);
					err = true;
				} else if (fgets(buff, BUFSIZ, fp))
					if (*buff != '\n') {
						fprintf(stderr,
							"Empty line missing for %s\n",
							Cnames[i].name);
						exit(1);
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
		} else if (strncmp(buff, ".sp 0", 4) && func != -1)
			strcat(doc_buffer, buff);
		loc = ftell( fp );
	}

	if (func != -1)
		cmd_docs[func] = strdup(doc_buffer);

	return( err );
}

static int c_filter(const struct dirent *ent)
{
	char *p = strrchr(ent->d_name, '.');
	if (p)
		return strcmp(p, ".c") == 0;
	return 0;
}

static void add_one(char *func, char *doc)
{
	char *p;
	int i;

	for (p = func; *p; ++p)
		if (*p == '_')
			*p = '-';

	for( i = 0; i < NUMFUNCS; ++i ) {
		if( strcmp(Cnames[i].name, func) == 0 )
		{	/* found it! */
			if (cmd_docs[i]) {
				fprintf(stderr, "Already has doc!");
				exit(1);
			}

			cmd_docs[i] = strdup(doc);
			if (!cmd_docs[i]) {
				fputs("Out of memory!\n", stderr);
			}
			return;
		}
	}

	fprintf(stderr, "%s not found in cnames\n", func);
	exit(1);
}

static void process_one_file(char *fname)
{
	char path[256];
	snprintf(path, sizeof(path), "../%s", fname);

	FILE *fp = fopen(path, "r");
	if (!fp) {
		perror(fname);
		exit(1);
	}

	char line[128], *p;
	while (fgets(line, sizeof(line), fp))
		if (strncmp(line, "/***", 4) == 0) {
			*doc_buffer = '\0';
			while (fgets(line, sizeof(line), fp))
				if (strncmp(line, " */", 3) == 0)
					break;
				else
					strcat(doc_buffer, line + 3);
			if (fgets(line, sizeof(line), fp)) {
				if (strncmp(line, "void Z", 6)) {
					fprintf(stderr, "Doc with no func in %s", fname);
					exit(1);
				}
				for (p = line + 6; *p && *p != '('; ++p) ;
				*p = '\0';
				add_one(line + 6, doc_buffer);
			}
		}

	fclose(fp);
}

static bool process_c_files(void)
{
	struct dirent **namelist;
	int n = scandir("..", &namelist, c_filter, alphasort);
	if (n < 0) {
		perror("scandir");
		exit(1);
	}

	while (n--) {
		process_one_file(namelist[n]->d_name);
		free(namelist[n]);
	}
	free(namelist);

	return true;
}
