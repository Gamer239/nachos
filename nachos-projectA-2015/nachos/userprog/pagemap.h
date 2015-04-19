#ifdef CHANGED
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
		int* inTime;
		PageMap(int numpages);
		~PageMap();
		void setInTime(int page, int time);
		void clearTime(int page);

	public:
		static PageMap* GetInstance();
		void Clear(int which);
		int Find();
		int GetNextVictim();
};

#endif
#endif
