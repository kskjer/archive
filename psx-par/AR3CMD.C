#include "ar3cmd.h"

#define TOVAL_L	((unsigned int)16384)
#define TOVAL_S	((unsigned int)8192)

int Ar3Enter(void)
{
	int retry = 3;
	int rc;
	unsigned int timeOut = TOVAL_L;
	
	while (retry) {
		rc = Ar3StartSession(3, timeOut);
	/*	if (rc == 0 || rc == 7) break; */
		if (rc == 7) break;
		retry --;
		timeOut = TOVAL_S;
	}
	if (rc != 7) return Ar32Enter(); /* いかないはず */
	return 0;
}


/* for Action Replay Ver3.2 念のため */
int Ar32Enter(void)
{
	int retry = 2;
	int rc;
	unsigned int timeOut = TOVAL_L;
	
	while (retry) {
		rc = Ar3ExchangeByte(0x33, timeOut);
		if (rc == 0x76) {
			rc = 0x60 | Ar3ExchangeNybble(3, timeOut);
		}
		if (rc == 'g') break;
		retry --;
		timeOut = TOVAL_S;
	}
	return rc == 'g' ? 0:-1;
}

int Ar3IssueCommand(int command)
{
	int retry;
	int rc;
	unsigned int timeOut = TOVAL_L;
	
	for (retry = 3 ; retry ; retry --) {
		rc = Ar3ExchangeByte('G', timeOut);
		if (rc == 'g') {
			rc = Ar3ExchangeByte('T', TOVAL_S);
			if (rc == 't') break;
		}
		timeOut = TOVAL_S;
	}
	if (rc != 't') return -1;
	rc = Ar3ExchangeByte(command, TOVAL_S);
	return rc;
}

/*
 *	RETURN VALUE: Running status
 */
int Ar3Exit(void)
{
	int rc;
	
	rc = Ar3IssueCommand(101);
	if (rc < 0) return rc;
	rc = Ar3ExchangeByte(0, TOVAL_S);
	
	return rc;
}

int Ar3GetVersion(char *dest, int destSize)
{
	int rc;
	int i;
	int len;
	
	rc = Ar3IssueCommand(102);
	if (rc < 0) return rc;
	
	for (i = 0 ; i < 4 ; i ++) {
		rc = Ar3ExchangeByte(0, TOVAL_S);
		if (i < destSize) dest[i] = (char)rc;
	}
	if (rc < 0) return -1;
	len = dest[3] + 4;
	for (i = 4 ; i < len ; i ++) {
		rc = Ar3ExchangeByte(0, TOVAL_S);
		if (i < destSize) dest[i] = (char)rc;
	}
	if (len < destSize) dest[len] = '\0';
	return len;
}

int Ar3ReadMem(char *dest, int size, unsigned long addr)
{
	int rc;
	int sum = 0;
	int i;
	
	rc = Ar3IssueCommand(1);
	if (rc < 0) return rc;
	rc = Ar3Exchange2Byte(addr >> 16, TOVAL_S);
	if (rc < 0) return rc;
	Ar3Exchange2Byte(addr, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte((long)size, TOVAL_S);
	
	for (i = 0 ; i < size ; i ++) {
		rc = Ar3ExchangeByte(0, TOVAL_S);
		if (rc < 0) return rc;
		dest[i] = (char)rc;
		sum += rc;
		sum &= 0xff;
	}
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	rc = Ar3ExchangeByte(0, TOVAL_S);
	if (rc < 0) return rc;
	
	return sum == rc ? 0 : 1;
}

int Ar3WriteMem(char *src, int size, unsigned long addr)
{
	int rc;
	int sum = 0;
	int i;
	int n;
	
	rc = Ar3IssueCommand(2);
	if (rc < 0) return rc;
	rc = Ar3Exchange2Byte(addr >> 16, TOVAL_S);
	if (rc < 0) return rc;
	Ar3Exchange2Byte(addr, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte((long)size, TOVAL_S);
	
	for (i = 0 ; i < size ; i ++) {
		n = src[i];
		n &= 0xff;
		rc = Ar3ExchangeByte(n, TOVAL_S);
		if (rc < 0) return rc;
		sum += n;
		sum &= 0xff;
	}
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	Ar3Exchange2Byte(0L, TOVAL_S);
	rc = Ar3ExchangeByte(0, TOVAL_S);
	if (rc < 0) return rc;
	
	return sum == rc ? 0 : 1;
}

/********************************************************/
/* MY OWN PROTOCOL, AFTER THIS                          */
/********************************************************/
int HnmrAr3Enter(void)	/* Not implemented yet. */
{
	int retry = 2;
	int rc;
	unsigned int timeOut = TOVAL_L;
	
	while (retry) {
		rc = Ar3StartSession(5, timeOut);
		if (rc == 0 || rc == 7) break;
		retry --;
		timeOut = TOVAL_S;
	}
	
	return rc;
}

int HnmrAr3JumpAndLink(unsigned long addr)
{
	int rc;
	
	rc = Ar3IssueCommand('X');
	if (rc < 0) return rc;
	rc = Ar3Exchange2Byte(addr >> 16, TOVAL_S);
	if (rc < 0) return rc;
	rc = Ar3Exchange2Byte(addr, TOVAL_S);
	if (rc < 0) return rc;

	return 0;
}

int HnmrAr3Backup(void)
{
	int rc;
	rc = Ar3IssueCommand('B');
	if (rc == 0) rc = Ar3ExchangeByte(0, 0);
	return rc;
}

int HnmrAr3Restore(void)
{
	int rc;
	rc = Ar3IssueCommand('U');
	if (rc == 0) rc = Ar3ExchangeByte(0, 0);
	return rc;
}

