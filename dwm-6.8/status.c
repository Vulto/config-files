/* Status line: popen() the bar commands, assemble stext. Included from dwm.c. */

#define CMDLENGTH 50
#define STATUSLENGTH (LENGTH(blocks) * CMDLENGTH + 1)

static char statusbar[LENGTH(blocks)][CMDLENGTH];
static char statusstr[2][STATUSLENGTH];
static char statusbtn[2];

static void
getblock(const Block *block, char *output)
{
	char tempstatus[CMDLENGTH] = { 0 };
	int off = 0;
	FILE *cmdf;
	int i;

	if (block->signal)
		tempstatus[off++] = (char)block->signal;
	strcpy(tempstatus + off, block->icon);
	if (statusbtn[0]) {
		setenv("BLOCK_BUTTON", statusbtn, 1);
		setenv("BUTTON", statusbtn, 1);
		cmdf = popen(block->command, "r");
		statusbtn[0] = '\0';
		unsetenv("BLOCK_BUTTON");
		unsetenv("BUTTON");
	} else {
		cmdf = popen(block->command, "r");
	}
	if (!cmdf)
		return;
	i = (int)strlen(block->icon);
	if (!fgets(tempstatus + off + i, CMDLENGTH - off - i - (int)delimLen, cmdf))
		/* empty */;
	i = (int)strlen(tempstatus);
	if (i != 0) {
		i = tempstatus[i - 1] == '\n' ? i - 1 : i;
		if (delim[0] != '\0')
			strncpy(tempstatus + i, delim, delimLen);
		else
			tempstatus[i++] = '\0';
	}
	strcpy(output, tempstatus);
	pclose(cmdf);
}

static void
getblocks(int time)
{
	unsigned int i;

	for (i = 0; i < LENGTH(blocks); i++)
		if ((blocks[i].interval != 0 && time % (int)blocks[i].interval == 0)
		    || time == -1)
			getblock(blocks + i, statusbar[i]);
}

static void
getsigblocks(unsigned int signal)
{
	unsigned int i;

	for (i = 0; i < LENGTH(blocks); i++)
		if (blocks[i].signal == signal)
			getblock(blocks + i, statusbar[i]);
}

static int
assemblestatus(char *str, char *last)
{
	unsigned int i;
	size_t dlen;

	strcpy(last, str);
	str[0] = '\0';
	for (i = 0; i < LENGTH(blocks); i++)
		strcat(str, statusbar[i]);
	dlen = strlen(delim);
	if (dlen && strlen(str) >= dlen)
		str[strlen(str) - dlen] = '\0';
	return strcmp(str, last);
}

static void
writestatus(void)
{
	if (!assemblestatus(statusstr[0], statusstr[1]))
		return;
	snprintf(stext, sizeof stext, "%s", statusstr[0]);
	updatestatus();
}

static void
status_init(void)
{
	delimLen = MIN(delimLen, (unsigned)strlen(delim));
	getblocks(-1);
	writestatus();
}

static void
status_tick(int t)
{
	getblocks(t);
	writestatus();
}

static void
status_click(int signal, int btn)
{
	if (btn)
		statusbtn[0] = (char)('0' + (btn & 0xff));
	getsigblocks((unsigned)signal);
	writestatus();
}
