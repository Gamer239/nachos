// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#ifdef CHANGED
#include "pagemap.h"
#include "noff.h"
#endif

#define UserStackSize		1024 	// increase this as necessary!
#define IN_SWAP				-3
#define NOT_LOADED			-4

class PageMap;

class AddrSpace {
	public:
		AddrSpace(OpenFile *executable);	// Create an address space,
		// initializing it with the program
		// stored in the file "executable"
		~AddrSpace();			// De-allocate an address space

		void InitRegisters();		// Initialize user-level CPU registers,
		// before jumping to user code

		void SaveState();			// Save/restore address space-specific
		void RestoreState();		// info on a context switch
#ifdef CHANGED
		bool GetFull();			// so we can check if space creation failed
		void SetArguments(int argVec, char* filename);
		void LoadArguments();
		void LoadPage(int vPage);
		void LoadMem(int addr, int fileAddr, int size);
		int GetNumPages();
		void InvalidatePage(int vpn);
		void LoadPageFromExecutable(int vPage);
		TranslationEntry *pageTable;	// Assume linear page table translation
		void PrintPages();
#endif

	private:
		OpenFile* executable;
		OpenFile* swap;
		NoffHeader noffH;
		// for now!
		unsigned int numPages;		// Number of pages in the virtual
		// address space
#ifdef CHANGED
		PageMap* pageMap;
		bool memFull;
		int argc;
		char** argv;
		void SaveToSwap(int vPage);
		void GetFromSwap(int vpage);
		// void LoadMem(int virtualAddr, int size, int inFileAddr);
#endif
};

#endif // ADDRSPACE_H
