#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ar3cmd.h"

int doit(const char *fname, unsigned long addr);

static int xtoi(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static unsigned long xstrtol(const char *str)
{
	int i;
	int n;
	unsigned long ret = 0;

	for (i = 0 ; i < 8 ; i ++) {
		n = xtoi(str[i]);
		if (n < 0) break;
		ret <<= 4;
		ret |= n;
	}

	return ret;
}


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
	fprintf(stderr, "%s uploads file via PAR3.\n", ptr);
	fprintf(stderr,
		"Usage: %s [-p <LPT port>] <filename> <address>\n",
		ptr
		);

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int c;
	int i = 1;
	int lptPort = 1;
	const char *fname;
	unsigned long addr;

	if (argc < 3) Usage(argv[0]);

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
	if (i >= argc) Usage(argv[0]);
	fname = argv[i ++];
	if (i >= argc) Usage(argv[0]);
	addr = xstrtol(argv[i ++]);

	Ar3SetPortNo(LptGetPortAddr(lptPort));
	i = doit(fname, addr);

	if (i != 0) {
		fputs("Failed.\n", stderr);
	} else fputs("Done.\n", stderr);
	
	return i == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int doit(const char *fname, unsigned long addr)
{
	int rc;
	FILE *fp;
	char buf[256];
	int n;
	long sumerr;
	
	rc = Ar3Enter();
	if (rc != 0) return -1;
	rc = Ar3Exit();
	/*
	 *	1  : PAR3 is in Main Menu.
	 *	2  : PSX is in Game.
	 *	'H': Hanimar's PAR3 tool is running.
	 *	neg: error.
	 */
	if (rc < 0) return -2;
	if (rc == 1) {
		fputs("PSX must be in game.\n", stderr);
		return -3;
	}

	fp = fopen(fname, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open file [%s].\n", fname);
		return -4;
	}

	rc = Ar3Enter();
	if (rc != 0) {
		fclose(fp);
		return -1;
	}

	while ((n = fread(buf, 1, sizeof (buf), fp)) > 0) {
		for (sumerr = 10 ; sumerr ; sumerr --) {
			rc = Ar3WriteMem(buf, n, addr);
			if (rc < 0) {
				fclose(fp);
				return -1;
			}
			if (rc == 0) break;
		}
if (sumerr < 10) printf("%d\n", 10 - sumerr);
		if (sumerr == 0) {
			fputs("Too many checksum errors!\n", stderr);
			fclose(fp);
			Ar3Exit();
			return -1;
		}
		addr += (unsigned long)n;
	}
	fclose(fp);
	Ar3Exit();
	
	return 0;
}

