#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#define NTHREADS  5
#define SLEEP     0
#define BUFSIZE   5
#define WBUFSIZE  10

Lock* rwMon;          /* Monitor */
Condition* space;          /* Empty slots in buffer */
Condition* chars;          /* Unread characters in buffer */
int       inBuf;          /* Number of characters in the buffer */
char      buf[2 * BUFSIZE]; /* Allow write past end.  Will see in DEBUG */
int       wPtr;           /* Position of next write */
int       rPtr;           /* Position of next read */

void Writer(int arg){
	int i;
	// int WBUFSIZE;
	char printBuf[100];
	char str[WBUFSIZE];

	for (i = 0; i < WBUFSIZE; i++) {
		str[i] = ('A' + arg);
	}
	str[i] = '\0';
	
	snprintf(printBuf, 100, "New writer <%d>\n", arg); 
	fflush(stdout);
	write(1, printBuf, strlen(printBuf));

	rwMon->Acquire();
	snprintf(printBuf, 100, "Writer %d here\n", arg);
	write(1, printBuf, strlen(printBuf));
	for (i = 0; i < WBUFSIZE; i++) {
		while (inBuf == BUFSIZE) { 
			space->Wait(rwMon); 
		}
		buf[wPtr] = str[i];
		printf("W <%c>: <%s>\n", str[i], buf);
		fflush(stdout);
		wPtr = (wPtr + 1) % BUFSIZE;
		inBuf++;
		chars->Signal(rwMon);
	}
	rwMon->Release();
}

void Reader(int arg){
	// int WBUFSIZE;
	int i;
	char printBuf[100];
	char ch;

	printf("New reader [%d]\n", arg);
	fflush(stdout);

	rwMon->Acquire();
	for (i = 0; i < WBUFSIZE; i++) {
		while (inBuf==0) {
			chars->Wait(rwMon); 
		}
		ch = buf[rPtr];
		buf[rPtr] = 'X';
		printf("R[%d]<%c>: <%s>\n", arg, ch, buf);
		fflush(stdout);
		rPtr = (rPtr + 1) % BUFSIZE;
		inBuf--;
		space->Signal(rwMon); 
	}
	rwMon->Release();
}


void ThreadTest(){

	int        i, j;
	char       wBufs[NTHREADS][WBUFSIZE + 1];
	Thread* t;

	inBuf = 0;
	wPtr = 0;
	rPtr = 0;

	rwMon = new Lock("rwmonitor lock");
	space = new Condition("space condition");
	chars = new Condition("chars condition");

	bzero(buf, BUFSIZE);

	for (i = 0; i < NTHREADS; i++) {
		t = new Thread("writer thread", 1);
		t->Fork(Writer, i);
	}

	for (i = 0; i < NTHREADS; i++) {
		t = new Thread("reader thread", 0);
		t->Fork(Reader, i);
	}
	
}
