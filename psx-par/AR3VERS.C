#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ar3cmd.h"

void Usage(const char *me)
{
	const char *ptr;

	ptr = strrchr(me, '\\');
	if (ptr) ptr ++;
	else {
		ptr = strrchr(me, ':');
		if (ptr) ptr ++;
		else ptr = me;
	}
	fprintf(stderr, "Build:%s\n", __DATE__);
	fprintf(stderr, "%s displays the version of Action Replay 3.\n", ptr);
	fprintf(stderr, "Usage: %s [-p <LPT port>]\n", ptr);

	exit(EXIT_FAILURE);
}

int doit(void)
{
	int rc;
	char buf[256];
	
	rc = Ar3Enter();
	if (rc != 0) {
		fputs("Connection refused.\n", stderr);
		return -1;
	}
	rc = Ar3GetVersion(buf, sizeof(buf));
	if (rc < 0) {
		fputs("Connection closed.\n", stderr);
		return -1;
	}
	printf("%02x%02x%02x%02x \"%s\"\n",
		buf[0] & 0xff,
		buf[1] & 0xff,
		buf[2] & 0xff,
		buf[3] & 0xff,
		&buf[4]
		);

	Ar3Exit();
	return 0;
}

int main(int argc, char **argv)
{
	int c;
	int i = 1;
	int lptPort = 1;
	const char *fname;
	unsigned long addr;
	unsigned long size;

	if (argc > 1) {
		c = argv[i][0];
		if (c == '-' || c == '/') {
		c = argv[i][1];
			if (c == 'p') {
				if (argv[i][2] != '\0') {
					lptPort = argv[i][2] - '0';
				} else {
				i ++;
					lptPort = argv[i][0] - '0';
				}
			}
			i ++;
		}
	}
	Ar3SetPortNo(LptGetPortAddr(lptPort));
	doit();
	
	return 0;
}
