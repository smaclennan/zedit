/* WARNING: Does not work well with newer shells such as bash and zsh */

/*
Invoke a shell on the other end of a two way pipe.
Returns true if the invocation succeeded.
*/
bool invoke(struct buff *tbuff, char *argv[])
{
	int from[2], to[2];

	/* Zshell may call with tbuff->child not EOF */
	if (tbuff->child != EOF)
		return false;

	if (pipe(from) == 0) {
		if (pipe(to) == 0) {
			tbuff->child = fork();
			if (tbuff->child == 0) {
				/* child */
				(void)close(from[0]);
				(void)close(to[1]);
				dup2(to[0],   0);
				dup2(from[1], 1);
				dup2(from[1], 2);
				execvp(argv[0], argv);
				fputs("Unable to exec shell\n", stderr);
				pause();	/* wait to die */
			}

			(void)close(from[1]);	/* we close these fail or not */
			(void)close(to[0]);
			if (tbuff->child != EOF) {
				/* SUCCESS! */
				tbuff->in_pipe  = from[0];
				tbuff->out_pipe = fdopen(to[1], "w");
				FD_SET(from[0], &SelectFDs);
				if (from[0] >= NumFDs)
					NumFDs = from[0] + 1;
				return true;
			} else {
				/* fork failed - clean up */
				(void)close(from[0]);
				(void)close(to[1]);
				error("Unable to fork shell");
			}
		} else {
			(void)close(from[0]);
			(void)close(from[1]);
		}
	}
	error("Unable to open pipes");
	return false;
}

/*
Send the buffer line to a pipe.
This command is invoked by the Newline command.
*/
void sendtopipe(void)
{
	char line[256 + 1];
	int i;
	struct mark tmark;

	mrktomrk(&tmark, Curbuff->mark);
	if (bisaftermrk(&tmark))
		bswappnt(&tmark);
	for (i = 0; i < 256 && !bisatmrk(&tmark); bmove1(), ++i)
		line[i] = Buff();
	line[i] = '\0';
	fputs(line, Curbuff->out_pipe);
	fflush(Curbuff->out_pipe);
	if (!bisend()) {
		btoend();
		binstr(line);
	}
}

