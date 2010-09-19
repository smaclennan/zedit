/* This mimics the Zedit end of the application */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#define FNAME		"/tmp/.Zblist"

#define FATAL(s)	{ perror(s); exit(1); }

main(int argc, char *argv[])
{
	int s, sock, len;
	struct sockaddr outSock;
	char cmd[1024];
	fd_set keep, readfds;

	if((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		FATAL("socket");

	unlink(FNAME);

	outSock.sa_family = AF_UNIX;
	strcpy(outSock.sa_data, FNAME);
	len = sizeof(outSock);
	if(bind(s, &outSock, len) == -1) FATAL("bind");

	if(listen(s, 5) == -1) FATAL("listen");

	if((sock = accept(s, &outSock, &len)) == -1) FATAL("accept");
	
	printf("CONNECTED!\n");

	FD_ZERO(&keep);
	FD_SET(sock, &keep);
	FD_SET(fileno(stdin), &keep);
	readfds = keep;

	printf("> ");
	while(select(sock + 1, &readfds, 0, 0, 0) > 0)
	{
		if(FD_ISSET(fileno(stdin), &readfds))
		{
			gets(cmd);
			write(sock, cmd, strlen(cmd) + 1);
			printf("> ");
		}
		else if(FD_ISSET(sock, &readfds))
		{
			if(read(sock, cmd, sizeof(cmd)) > 0)
				printf("Got '%s' from socket\n", cmd);
			else
			{	/* socket closed */
				printf("Socket closed.\n");
				break;
			}
		}
		readfds = keep;
	}
	
	unlink(FNAME);
	exit(0);
}
