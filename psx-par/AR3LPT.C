/*
 *	Action Replay 3 communication program for MSDOS.
 */
#include <stdio.h>
#include "ar3cmd.h"

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#define OUTP	outp
#define INP	inp

static ushort PortNo = 0;

ushort LptGetPortAddr(int lptNo)
{
	ushort far *biosWork = (ushort far *)0x00400008L;
	if (lptNo < 1 || lptNo > 4) return 0;
	lptNo --;
	biosWork += lptNo;
	return *biosWork;
}

ushort Ar3SetPortNo(ushort no)
{
	ushort old = PortNo;
	PortNo = no;

	OUTP(PortNo + 2, 0);	/* AT mode? */
	OUTP(PortNo, 0);
	return old;
}


int Ar3Send(int n, uint timeOut)
{
	if ((INP(PortNo + 1) & 0x08) != 0) {
		OUTP(PortNo, 0);
		if (timeOut == 0) while ((INP(PortNo + 1) & 0x08) != 0);
		else {
			while (timeOut) {
				if ((INP(PortNo + 1) & 0x08) == 0) break;
				timeOut --;
			}
			if (timeOut == 0) return -1;
		}
	}
	n &= 0x0f;
	n |= 0x10;
	OUTP(PortNo, n);

	return 0;
}

int Ar3Recv(uint timeOut)
{
	int rc;
	
	if (timeOut == 0) while ((INP(PortNo + 1) & 0x08) == 0);
	else {
		while (timeOut) {
			if (INP(PortNo + 1) & 0x08) break;
			timeOut --;
		}
		if (timeOut == 0) return -1;
	}
	rc = (INP(PortNo + 1) ^ 0x80) >> 4;
	/*** OUTP(PortNo, 0); ***/
	rc &= 0x0f;	/* 念のため */
	return rc;
}

/* cmd=3, usually. timeOut=10000??? */
int Ar3StartSession(int cmd, uint timeOut)
{
	int rc;
	
	if (PortNo == 0) return -10;

	cmd &= 0x0f;
	cmd |= 0x10;
	OUTP(PortNo, cmd);
	rc = Ar3Recv(timeOut);
	OUTP(PortNo, 0);
	
	return rc;
}

int Ar3ExchangeNybble(int n, uint timeOut)
{
	int rc;
	
	if (Ar3Send(n, timeOut) != 0) return -1;
	rc = Ar3Recv(timeOut);
	OUTP(PortNo, 0);
	
	return rc;
}

int Ar3ExchangeByte(int n, uint timeOut)
{
	int rc;
	int ret;
	
	rc = Ar3ExchangeNybble(n >> 4, timeOut);
	if (rc < 0) return -1;
	ret = rc << 4;
	rc = Ar3ExchangeNybble(n, timeOut);
	if (rc < 0) return -2;
	ret |= rc;
	
	return ret;
}

long Ar3Exchange2Byte(long n, uint timeOut)
{
	int rc;
	long ret;
	
	rc = Ar3ExchangeByte(n >> 8, timeOut);
	if (rc < 0) return (long)rc;
	ret = (long)rc << 8;
	
	rc = Ar3ExchangeByte(n, timeOut);
	if (rc < 0) return (long)rc - 2L;
	ret |= (long)rc;
	
	return ret;
}
