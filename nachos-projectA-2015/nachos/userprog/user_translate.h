#ifdef CHANGED
// user_translate.h
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef USERTRANSLATE_H
#define USERTRANSLATE_H

#include "copyright.h"
#include "utility.h"
#include "translate.h"

/*
* The purpose of this class is to serve as a separate implementation from the
* the machine's address translation functions. This still hooks into
* the machine to get the correct mappings.
*/
class UserTranslate {
  public:

    static bool ReadMem(int addr, int size, int* value);
    static bool WriteMem(int addr, int size, int value);
    				// Read or write 1 byte of virtual
				// memory (at addr).  Return FALSE if a
				// correct translation couldn't be found.

    static ExceptionType Translate(int virtAddr, int* physAddr, int size,bool writing);
    				// Translate an address, and check for
				// alignment.  Set the use and dirty bits in
				// the translation entry appropriately,
    				// and return an exception code if the
				// translation couldn't be completed.
};

#endif // USERTRANSLATE_H
#endif
