#ifdef CHANGED
//TODO: fix comments in this file
// user_translate.cc
//	Routines to translate virtual addresses to physical addresses.
//	Software sets up a table of legal translations.  We look up
//	in the table on every memory reference to find the true physical
//	memory location.
//
// Two types of translation are supported here.
//
//	Linear page table -- the virtual page # is used as an index
//	into the table, to find the physical page #.
//
//	Translation lookaside buffer -- associative lookup in the table
//	to find an entry with the same virtual page #.  If found,
//	this entry is used for the translation.
//	If not, it traps to software with an exception.
//
//	In practice, the TLB is much smaller than the amount of physical
//	memory (16 entries is common on a machine that has 1000's of
//	pages).  Thus, there must also be a backup translation scheme
//	(such as page tables), but the hardware doesn't need to know
//	anything at all about that.
//
//	Note that the contents of the TLB are specific to an address space.
//	If the address space changes, so does the contents of the TLB!
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "machine.h"
#include "addrspace.h"
#include "system.h"
#include "user_translate.h"

//----------------------------------------------------------------------
// UserTranslate::ReadMem
//      Read "size" (1) bytes of virtual memory at "addr" into
//	the location pointed to by "value".
//
//   	Returns FALSE if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to read from
//	"size" -- the number of bytes to read (1)
//	"value" -- the place to write the result
//----------------------------------------------------------------------

bool
UserTranslate::ReadMem(int addr, int size, int *value)
{
	int data;
	ExceptionType exception;
	int physicalAddress;

	//make sure that we are dealing with only size one translations
	if ( size != 1)
	{
		return FALSE;
	}

	DEBUG('a', "Reading VA 0x%x, size %d\n", addr, size);

	exception = UserTranslate::Translate(addr, &physicalAddress, size, FALSE);
	if (exception != NoException) {
		machine->RaiseException(exception, addr);
		return FALSE;
	}
	switch (size) {
		case 1:
		data = machine->mainMemory[physicalAddress];
		*value = data;
		break;

		default: ASSERT(FALSE);
	}

	DEBUG('a', "\tvalue read = %8.8x\n", *value);
	return (TRUE);
}

//----------------------------------------------------------------------
// UserTranslate::WriteMem
//      Write "size" (1) bytes of the contents of "value" into
//	virtual memory at location "addr".
//
//   	Returns FALSE if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to write to
//	"size" -- the number of bytes to be written (1)
//	"value" -- the data to be written
//----------------------------------------------------------------------

bool
UserTranslate::WriteMem(int addr, int size, int value)
{
	ExceptionType exception;
	int physicalAddress;

	//make sure that we are dealing with only size one translations
	if ( size != 1)
	{
		return FALSE;
	}

	DEBUG('a', "Writing VA 0x%x, size %d, value 0x%x\n", addr, size, value);

	exception = Translate(addr, &physicalAddress, size, TRUE);
	if (exception != NoException) {
		machine->RaiseException(exception, addr);
		return FALSE;
	}
	switch (size) {
		case 1:
		machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
		break;

		default: ASSERT(FALSE);
	}

	return TRUE;
}

//----------------------------------------------------------------------
// UserTranslate::Translate
// 	Translate a virtual address into a physical address, using
//	either a page table or a TLB.  Check for alignment and all sorts
//	of other errors, and if everything is ok, set the use/dirty bits in
//	the translation table entry, and store the translated physical
//	address in "physAddr".  If there was an error, returns the type
//	of the exception.
//
//	"virtAddr" -- the virtual address to translate
//	"physAddr" -- the place to store the physical address
//	"size" -- the amount of memory being read or written
// 	"writing" -- if TRUE, check the "read-only" bit in the TLB
//----------------------------------------------------------------------

ExceptionType
UserTranslate::Translate(int virtAddr, int* physAddr, int size, bool writing)
{
	int i;
	unsigned int vpn, offset;
	TranslationEntry *entry;
	unsigned int pageFrame;

	//make sure that we are dealing with only size one translations
	if ( size != 1)
	{
		return AddressErrorException;
	}

	DEBUG('a', "\tTranslate 0x%x, %s: ", virtAddr, writing ? "write" : "read");

	// we must have either a TLB or a page table, but not both!
	ASSERT(machine->tlb == NULL || machine->pageTable == NULL);
	ASSERT(machine->tlb != NULL || machine->pageTable != NULL);

	// calculate the virtual page number, and offset within the page,
	// from the virtual address
	vpn = (unsigned) virtAddr / PageSize;
	offset = (unsigned) virtAddr % PageSize;

	if (machine->tlb == NULL) {		// => page table => vpn is index into table
		if (vpn >= machine->pageTableSize) {
			DEBUG('a', "virtual page # %d too large for page table size %d!\n",
			virtAddr, machine->pageTableSize);
			return AddressErrorException;
		} else if (!machine->pageTable[vpn].valid) {
			DEBUG('a', "virtual page # %d too large for page table size %d!\n",
			virtAddr, machine->pageTableSize);
			return PageFaultException;
		}
		entry = &machine->pageTable[vpn];
	} else {
		for (entry = NULL, i = 0; i < TLBSize; i++)
		if (machine->tlb[i].valid && ((unsigned int) machine->tlb[i].virtualPage == vpn)) {
			entry = &machine->tlb[i];			// FOUND!
			break;
		}
		if (entry == NULL) {				// not found
			DEBUG('a', "*** no valid TLB entry found for this virtual page!\n");
			return PageFaultException;		// really, this is a TLB fault,
			// the page may be in memory,
			// but not in the TLB
		}
	}

	if (entry->readOnly && writing) {	// trying to write to a read-only page
		DEBUG('a', "%d mapped read-only at %d in TLB!\n", virtAddr, i);
		return ReadOnlyException;
	}
	pageFrame = entry->physicalPage;

	// if the pageFrame is too big, there is something really wrong!
	// An invalid translation was loaded into the page table or TLB.
	if (pageFrame >= NumPhysPages) {
		DEBUG('a', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
		return BusErrorException;
	}
	entry->use = TRUE;		// set the use, dirty bits
	if (writing)
	entry->dirty = TRUE;
	*physAddr = pageFrame * PageSize + offset;
	ASSERT((*physAddr >= 0) && ((*physAddr + size) <= MemorySize));
	DEBUG('a', "phys addr = 0x%x\n", *physAddr);
	return NoException;
}

#endif
