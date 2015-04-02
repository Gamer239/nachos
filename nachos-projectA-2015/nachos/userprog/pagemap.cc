#ifdef CHANGED
#include "pagemap.h"
#include "machine.h"

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
}

#endif
