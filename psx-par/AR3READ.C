#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ar3cmd.h"

int doit(const char *fname, unsigned long addr, unsigned long size);

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
	fprintf(stderr, "%s downloads PSX RAM via PAR3.\n", ptr);
	fprintf(stderr,
		"Usage: %s [-p <LPT port>] <filename> <address>"
		" <size>\n\n",
		ptr
		);

	fprintf(stderr, "Ex. To download scratch, using LPT2, then type\n"
		"\t%s -p2 scratch.bin 1f800000 400\n\n", ptr);
	fprintf(stderr, "Ex. To download whole RAM, using LPT1, then type\n"
		"\t%s all.bin 80010000 1f0000\n\n", ptr);
	fprintf(stderr, "With 3.2 ROM, you can download anywhere you want!\n"
		"\t%s 80000000.bin a0000000 10000\n"
		"\t%s 1f000000.bin 9f000000 60000\n", ptr, ptr);

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int c;
	int i = 1;
	int lptPort = 1;
	const char *fname;
	unsigned long addr;
	unsigned long size;

	if (argc < 4) Usage(argv[0]);

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
	if (i >= argc) Usage(argv[0]);
	size = xstrtol(argv[i ++]);

	printf("%08lx to %08lx > [%s], Using LPT%d.\n",
		addr, addr + size - 1, fname, lptPort);

	Ar3SetPortNo(LptGetPortAddr(lptPort));
	i = doit(fname, addr, size);

	if (i != 0) {
		fputs("Failed.\n", stderr);
	} else fputs("Done.\n", stderr);
	
	return i == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int doit(const char *fname, unsigned long addr, unsigned long size)
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

	fp = fopen(fname, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open file [%s].\n", fname);
		return -4;
	}

	rc = Ar3Enter();
	if (rc != 0) {
		fclose(fp);
		remove(fname);
		return -1;
	}
	

	while (size) {
		n = sizeof (buf) < size ? sizeof (buf) : (int)size;
		for (sumerr = 10 ; sumerr ; sumerr --) {
			rc = Ar3ReadMem(buf, n, addr);
			if (rc < 0) {
				fclose(fp);
				remove(fname);
				return -1;
			}
			if (rc == 0) break;
		}
if (sumerr < 10) printf("%d\n", 10 - sumerr);
		if (sumerr == 0) {
			fputs("Too many checksum errors!\n", stderr);
			fclose(fp);
			remove(fname);
			Ar3Exit();
			return -1;
		}
		if (fwrite(buf, 1, n, fp) != n) {
			fputs("Write error.\n", stderr);
			fclose(fp);
			remove(fname);
			Ar3Exit();
			return -1;
		}
		size -= (unsigned long)n;
		addr += (unsigned long)n;
	}
	fclose(fp);
	Ar3Exit();
	
	return 0;
}

