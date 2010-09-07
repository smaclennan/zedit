int AudioExit = 0;

#if AUDIO_EXIT

#include <fcntl.h>
#include <unistd.h>
#include <sys/audioio.h>


static int audioOut(char *fname);

void audioExit()
{
	if(AudioExit || ((rand() >> 1) & 15) == 0)
	{
		extern char *Thispath;
		char fname[256];
	
		sprintf(fname, "%s/goodnight", Thispath);
		audioOut(fname);
	}
}

static int audioOut(char *fname)
{
	int audio, fd;
	char buf[1024];
	int n;
	
	if((fd = open(fname, O_RDONLY)) == -1)
		return -1;

	if((audio = open("/dev/audio", O_WRONLY)) == -1)
	{
		close(fd);
		return -1;
	}
	
	/* skip audiotool header */	
	read(fd, buf, 0x20);

	while((n = read(fd, buf, 1024)) > 0)
		write(audio, buf, n);

	close(fd);
	close(audio);
	
	return 0;
}

#else
void audioExit() {}
#endif
