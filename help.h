/* Help Types */
#define H_NONE				0
#define H_MISC				1
#define H_VAR				2
#define H_CURSOR			3
#define H_DELETE			4
#define H_SEARCH			5
#define H_MACRO				6
#define H_FILE				7
#define H_BUFF				8
#define H_DISP				9
#define H_MODE				10
#define H_HELP				11
#define H_BIND				12
#define H_SHELL				13

struct cnames
{
	char *name;
	Short fnum;
	Short htype;
};
#define CNAMESIZE			sizeof( struct cnames )
