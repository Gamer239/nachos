#ifdef CHANGED
#include "synch.h"
#include "copyright.h"
#include "system.h"

#define	NUMTHREADS 20

Condition* crossingCV;
Lock* monitor;
Lock* travel;
Lock* dirLock[2];
int onBridge;
int dirCount[2];

/**
 * A car arrives at the bridge.
 */
void ArriveBridge(int direction) {


	dirLock[direction]->Acquire();
	dirCount[direction]++;
	//if we're the first to arrive then wait for the travel direction
	if (dirCount[direction] == 1) {
		travel->Acquire();
	}
	dirLock[direction]->Release();
	
	// wait to cross the bridge
	monitor->Acquire();
	//since we're using mesa style we need a while loop check
	while (onBridge >= 3)
	{
		// give up access to our monitor while we wait
		crossingCV->Wait(monitor);
	}
	//update the bridge count
	onBridge++;
	monitor->Release();	
}

/**
 * Simply prints out the direction the car is going, as well
 * as the number of cars currently on the bridge.
 */
void CrossBridge(int direction) {
	monitor->Acquire();
	printf("Currently on the bridge going %d, onBridge: %d\n", direction, onBridge);
	monitor->Release();
}

void ExitBridge(int direction) {

	//reacuire the monitor and decrement our bridge count
	monitor->Acquire();
	onBridge--;
	
	//signal for more cars to pass
	crossingCV->Signal(monitor);
	monitor->Release();
	
	//grab our side's lock then decrement access 
	// and possibly release the signal flag
	dirLock[direction]->Acquire();
	dirCount[direction]--;
	if (dirCount[direction] == 0) {
		travel->Release();
	}
	dirLock[direction]->Release();
}	

void OneVehicle(int direction) {
	ArriveBridge(direction);
	CrossBridge(direction);
	ExitBridge(direction);
}

void ThreadTest() {

	Thread *t;

	//init our variables
	crossingCV = new Condition("condition variable");

	monitor = new Lock("monitor lock");
	travel = new Lock("travel lock");

	dirLock[0] = new Lock("direction 0 lock");
	dirLock[1] = new Lock("direction 1 lock");

	dirCount[0] = 0;
	dirCount[1] = 0;

	onBridge = 0;

	for (int i = 0; i < NUMTHREADS; i++) {
		t = new Thread("car direction 0");
		t->Fork(OneVehicle, 0);
		t = new Thread("car direction 1");
		t->Fork(OneVehicle, 1);
	}

}
#endif
