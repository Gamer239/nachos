
#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <stdio.h>
#include <strings.h>

#define  NUMTHREADS 20
#define  BUFSIZE    12				\
  
int      sharedCtr=0;
int      sharedmCtr=0;
Lock     *mutexLock;
char     buf[NUMTHREADS][12];


//-----------------------------------------------------------
// Toggle interrupts so that time advances
// If -rs is used, need time to advance to get a switch
//-----------------------------------------------------------
void Toggle() 
{
   int i;
   IntStatus oldLevel;

   for (i=0;i<500;i++) {           
       oldLevel=interrupt->SetLevel(IntOff);
       interrupt->SetLevel(oldLevel);
   }   

}

//-----------------------------------------------------------
// Expect context switch at Toggle();  
// Expect value of sharedCtr will not be zero 
//-----------------------------------------------------------
void
noMutex(int which)
{
    int localCtr=0;
    int num;
    for (num = 0; num < 10; num++) {                     
        sharedCtr++;
	Toggle();
        sharedCtr--;
        if (sharedCtr!=0)localCtr++;
    } 

    printf("*** Nonmutex thread %d has localCtr=%d\n", which, localCtr);
}


//-----------------------------------------------------------
//
//-----------------------------------------------------------
void
mutex(int which)
{
    int num;
    int localCtr=0;
    for (num = 0; num < 100; num++) {                     
        mutexLock->Acquire();     
        sharedmCtr++;
	Toggle();
        sharedmCtr--;
        if (sharedmCtr!=0)localCtr++;
        mutexLock->Release();	
    } 
    printf("*** Mutex thread %d has localCtr=%d\n", which, localCtr);

}

//----------------------------------------------------------------------
// 
// 
//
//----------------------------------------------------------------------

void
LockThreadTest()
{
    DEBUG('t', "Entering ThreadTest");
    Thread *t;
    mutexLock=new Lock("mutex");
    int index=NUMTHREADS/2;
    
    for (int i=0;i<index;i++) {	   

        bzero(buf[i],BUFSIZE);
        snprintf(buf[i],BUFSIZE,"%s %d","Thread",i);
        t = new Thread(buf[i]);
        t->Fork(mutex,i);

        bzero(buf[index+i],BUFSIZE);
        snprintf(buf[index+i],BUFSIZE,"%s %d","Thread",index+i);
        t = new Thread(buf[index+i]);
        t->Fork(noMutex,index+i);
    }    

}

