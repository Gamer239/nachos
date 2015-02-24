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
	if (dirCount[direction] == 1) {
		travel->Acquire();
	}
	dirLock[direction]->Release();
	monitor->Acquire();
	while (onBridge >= 3)
		crossingCV->Wait(monitor);
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
	monitor->Acquire();
	onBridge--;
	crossingCV->Signal(monitor);
	monitor->Release();
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
