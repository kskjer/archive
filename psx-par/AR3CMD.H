#ifndef _AR3_LPT_H_
#define _AR3_LPT_H_


#ifdef __cplusplus
extern "C" {
#endif

/* 
 * ar3lpt.c : LOW LEVEL ROUTINES
 */

unsigned short LptGetPortAddr(int lptNo);
unsigned short Ar3SetPortNo(unsigned short no);

int Ar3Send(int n, unsigned int timeOut);
int Ar3Recv(unsigned int timeOut);

int Ar3StartSession(int cmd, unsigned int timeOut);

int Ar3ExchangeNybble(int n, unsigned int timeOut);
int Ar3ExchangeByte(int n, unsigned int timeOut);
long Ar3Exchange2Byte(long n, unsigned int timeOut);

/*
 *	ar3cmd.c
 */
int Ar3Enter(void);
int Ar32Enter(void);
int Ar3IssueCommand(int command);
int Ar3Exit(void);
int Ar3GetVersion(char *dest, int destSize);
int Ar3ReadMem(char *dest, int size, unsigned long addr);
int Ar3WriteMem(char *src, int size, unsigned long addr);

/*
 *	My own protocol
 */
int HnmrAr3Enter(void);
int HnmrAr3JumpAndLink(unsigned long addr);
int HnmrAr3Backup(void);
int HnmrAr3Restore(void);

#ifdef __cplusplus
}
#endif
#endif /* _AR3_LPT_H_ */
