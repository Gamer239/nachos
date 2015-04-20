#ifdef CHANGED
#include "pagemap.h"
#include "machine.h"
#include "system.h"
#include <climits>

PageMap* PageMap::pagemap = NULL;

PageMap* PageMap::GetInstance() {
	if (pagemap == NULL) {
		pagemap = new PageMap(NumPhysPages);
	}
	return pagemap;
}

PageMap::PageMap(int numpages) :
	BitMap(numpages)
{
	inTime = new int[NumPhysPages];
	for (int i = 0; i < NumPhysPages; i++) {
		inTime[i] = -1;
	}
	vPage = new int[NumPhysPages];
	for (int i = 0; i < NumPhysPages; i++) {
		vPage[i] = -1;
	}
	curThreads = new Thread*[NumPhysPages];
}

void PageMap::setInTime(int page, int time) {
	inTime[page] = time;
}

void PageMap::clearTime(int page) {
	inTime[page] = -1;
}

void PageMap::Clear(int which) {
	BitMap::Clear(which);
	inTime[which] = -1;
	curThreads[which] = NULL;
	vPage[which] = -1;
}

int PageMap::Find(int vpn) {
	int bit = BitMap::Find();
	inTime[bit] = stats->totalTicks;
	curThreads[bit] = currentThread;
	vPage[bit] = vpn; 
	return bit;
}

int PageMap::GetNextVictim() {
	unsigned long long oldestTime = ULLONG_MAX;
	unsigned long long secondTime = ULLONG_MAX;
	int oldest = 0;
	int second = 0;
	for (int i = 0; i < NumPhysPages; i++) {
		if (inTime[i] < secondTime) {
			if (inTime[i] < oldestTime) {
				secondTime = oldestTime;
				second = oldest;
				oldestTime = inTime[i];
				oldest = i;	
			} else {
				secondTime = inTime[i];
				second = i;
			}
		}	
	}
	ASSERT(oldest >= 0);
	/*
	if (oldest == curFrame) {
		DEBUG('v', "oldest frame is the one were running off of right now :)\n");
		oldest = second;
	}
	*/
	// Mark(oldest);
	inTime[oldest] = stats->totalTicks;
	// curThreads[oldest]->space->InvalidatePage(oldest);
	return oldest;
}
#endif
