// pagemap.h
//  A singleton wrapper for our bitmap class.
//  This way, a user program's address space can just hold a pointer
//  to this singleton and we can have a single bitmap to keep track
//  of all of our free pages

#include "machine.h"
#include "bitmap.h"

#ifndef PAGEMAP_H
#define	PAGEMAP_H

class PageMap : public BitMap {

	private:
		static PageMap* pagemap;
		PageMap(int numpages);
		~PageMap();

	public:
		static PageMap* GetInstance();

};

#endif
