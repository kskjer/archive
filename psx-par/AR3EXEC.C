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
	fprintf(stderr, "%s executes PS-X EXE.\n", ptr);
	fprintf(stderr, "Usage: %s [-p <LPT port>] <filename>\n", ptr);
	fprintf(stderr, "You have to run PAR3 reflash tool on the PSX side.\n");
	fprintf(stderr, "Do not override reflash tool that is placed at 801f0000.\n");

	exit(EXIT_FAILURE);
}

int doit(const char *fname)
{
	int rc;
	FILE *fp;
	unsigned long addr;
	char buf[256];
	long sumerr = 0;
	unsigned long pc;
	unsigned long gp;
	unsigned long text;
	long n;
	
	rc = Ar3Enter();
	if (rc != 0) {
		fputs("Connection refused.\n", stderr);
		return -1;
	}
	rc = Ar3Exit();
	if (rc != 'H') {
		fputs("You should run PAR3 reflash tool on PSX side.\n", stderr);
		return -1;
	}
	Ar3Enter();

	fp = fopen(fname, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open file [%s].\n", fname);
		rc = -1;
		goto END_DOIT;
	}
	fread(buf, 1, 4, fp);
	if (buf[0] != 'P' || buf[1] != 'S' || buf[2] != '-' || buf[3] != 'X') {
		fprintf(stderr, "File %s cannot be a PS-X EXE.\n", fname);
		rc = -1;
		fclose(fp);
		goto END_DOIT;
	}
	fseek(fp, 0x10 , SEEK_SET);
	fread(&pc, 1, 4, fp);
	fread(&gp, 1, 4, fp);
	fread(&text, 1, 4, fp);
	fseek(fp, 0x800 , SEEK_SET);
	
	/* +10 PC  +14 GP +18 TEXT */

	addr = text;
printf("text=%08lx\n", addr);
printf("pc=%08lx\n", pc);
	while ((n = fread(buf, 1, sizeof (buf), fp)) > 0) {
		while ((rc = Ar3WriteMem(buf, n, addr)) == 1) sumerr ++;
		if (rc < 0) {
			fputs("Broken PIPE.\n", stderr);
			fclose(fp);
			return -1;
		}
		addr += n;
	}
	fclose(fp);

	HnmrAr3JumpAndLink(pc);
	if (sumerr > 0) {
		fprintf(stderr, "WARNNING:# of checksum err: %d\n", sumerr);
	} else fputs("Done.\n", stderr);

	return 0;
END_DOIT:
	Ar3Exit();
	return rc;
}

int main(int argc, char **argv)
{
	int c;
	int i = 1;
	int lptPort = 1;
	const char *fname;
	unsigned long addr;
	unsigned long size;

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
	doit(fname);
	
	return 0;
}
