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
}

void PageMap::setInTime(int page, int time) {
	inTime[page] = time;
}

void PageMap::clearTime(int page) {
	inTime[page] = -1;
}

void PageMap::Clear(int which) {
	inTime[which] = -1;
	BitMap::Clear(which);
}

int PageMap::Find() {
	int bit = BitMap::Find();
	inTime[bit] = stats->totalTicks;
	return bit;
}

int PageMap::GetNextVictim() {

	int oldest = 0;
	for (int i = 0; i < NumPhysPages; i++) {
		if (inTime[i] < inTime[oldest]) {
			oldest = i;
		}
	}
	Mark(oldest);
	inTime[oldest] = stats->totalTicks;
	return oldest;
}
#endif
