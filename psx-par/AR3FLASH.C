#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ar3cmd.h"

int doit(const char *fname);

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
	fprintf(stderr, "%s flashes PAR3 ROM.\n", ptr);
	fprintf(stderr,
		"Usage: %s [-p <LPT port>] <filename>\n\n", ptr);
	fprintf(stderr, "You have to run AR3 reflash tool on the PSX side.\n");

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int c;
	int i = 1;
	int lptPort = 1;
	const char *fname;

	if (argc < 2) Usage(argv[0]);

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

	Ar3SetPortNo(LptGetPortAddr(lptPort));
	i = doit(fname);

	if (i != 0) {
		fprintf(stderr, "Failed.(%d)\n", i);
	} else fputs("Done.\n", stderr);
	
	return i == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int doit(const char *fname)
{
	int rc;
	FILE *fp;
	char buf[256];
	int n, i;
	long sumerr;
	unsigned long addr = (unsigned long)0x80040000;
	unsigned long end = (unsigned long)0x800c0000;
	unsigned long size = (unsigned long)0x80000;
	
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
	if (rc != 'H') {
		fputs("Run the AR3 reflash tool!\n", stderr);
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
		return -5;
	}

	fputs("Sending...\n", stderr);
	while ((n = fread(buf, 1, sizeof (buf), fp)) > 0) {
		for (i = n ; i < sizeof (buf) ; i ++) buf[i] = 0xff;
		for (sumerr = 3 ; sumerr ; sumerr --) {
			rc = Ar3WriteMem(buf, sizeof (buf), addr);
			if (rc < 0) {
				fclose(fp);
				return -7;
			}
			if (rc == 0) break;
		}
		if (sumerr == 0) {
			fputs("Too many checksum errors!\n", stderr);
			fclose(fp);
			Ar3Exit();
			return -8;
		}
		addr += sizeof (buf);
	}
	fclose(fp);
	/* 残った部分は 0xff でうめる。 */
	memset(buf, 0xff, sizeof (buf));
	while (addr < end) {
		rc = Ar3WriteMem(buf, sizeof (buf), addr);
		if (rc < 0) return -9;
		addr += sizeof (buf);
	}

	fputs("Flashing...\n", stderr);
	rc = HnmrAr3Restore();
	Ar3Exit();
	
	return rc;
}

