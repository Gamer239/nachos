
#include "copyright.h"
#include "system.h"
#include "synch.h"

#define  NUMTHREADS 6
int      sharedCtr=0;

//===================================
// Run using -rs option  
//===================================  
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void
Increment(int which)
{
    int num;
    for (num = 0; num < 4; num++) { 
        sharedCtr++;
        printf("*** Thread %d has sharedCtr=%d\n", which, sharedCtr);
        fflush(stdout);
        currentThread->Yield();	/* Force scheduling decision */
    } 
    printf("*** Thread %d exits\n", which);
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void
Decrement(int which)
{
    int num;
    for (num = 0; num < 4; num++) { 
        sharedCtr--;
        printf("*** Thread %d has sharedCtr=%d\n", which, sharedCtr);
        fflush(stdout);
        currentThread->Yield();	 /* Force scheduling decision */
    } 
    printf("*** Thread %d exits.\n", which);
}


//----------------------------------------------------------------------
// 
// 
//
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering ThreadTest");
    Thread *t;

	for (int i=0;i<NUMTHREADS;i++) {	   

		/* Alternate creation of high and low priority threads so
		   simply pulling off ready queue in creation order should 
		   cause execution of low than high priority thread (Not
		   what we want) 

		   Expect all the incrementers will run then all the
		   decrementers, if priority scheduling implemented */
		   

		/* Create low priorty thread */
		t = new Thread("priority 1",1);
		t->Fork(Decrement, i);

		/* Create high priority thread */
		t = new Thread("priority 0",0);
		t->Fork(Increment, NUMTHREADS+i);
	}


}
