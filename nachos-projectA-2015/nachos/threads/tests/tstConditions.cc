
#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <stdio.h>
#include <strings.h>


#define  NUMTHREADS 10

int       sharedCtr=0;
int       sharedmCtr=0;
Lock      *mutexLock;
Condition *aCondition;
char      buf[NUMTHREADS][10];

//=============================
// Run with -rs option
//=============================
//-----------------------------------------------------------
//
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
//
//-----------------------------------------------------------
void Count(int which)
{
    mutexLock->Acquire();
    sharedCtr++;
    if (sharedCtr==NUMTHREADS){
        printf("Thread %d signals, sharedCtr = %d\n",which,sharedCtr);
        fflush(stdout);
        Toggle();  /* Opportunity for context-switch using -rs option */
                   /* This thread should keep the lock */
        aCondition->Signal(mutexLock);  /* Last one in start waking others */
    } else {
        /* First NUMTHREADS threads should wait on the condition */
        printf("Thread %d waits, sharedCtr = %d\n",which,sharedCtr);
        fflush(stdout);
        Toggle(); /* This thread should keep the lock.*/
        aCondition->Wait(mutexLock);
        printf("Thread %d off the wait, sharedCtr = %d\n",which,sharedCtr);
        fflush(stdout);
        aCondition->Signal(mutexLock);
    }
    //
    //---- Signaller should keep the lock.  This will execute before any
    //     awoken thread will.
    //
    Toggle();  /* No awoken thread should run here! */
    sharedCtr--;
    printf("Thread %d exits with sharedCtr = %d\n",which,sharedCtr);
    fflush(stdout);
    mutexLock->Release();  //--- Awoken thread may run now.
}


void
ThreadTest()
{

    DEBUG('t', "Entering ThreadTest");
    Thread *t;
    mutexLock=new Lock("mutex");
    aCondition=new Condition("aCondition");
      
    for (int i=0;i<NUMTHREADS;i++) {	   

        bzero(buf[i],10);
        snprintf(buf[i],10,"%s %d","Thread",i);

        t = new Thread(buf[i]);
        t->Fork(Count,i);
    }    


}

