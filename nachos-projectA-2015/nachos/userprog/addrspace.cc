// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#ifndef CHANGED
#include "noff.h"
#endif
#ifdef CHANGED
#include "exception_utils.h"
#endif
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

	static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------
#ifdef CHANGED
AddrSpace::AddrSpace(OpenFile *execFile)
{
	unsigned int i, size;
	executable = execFile;
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
			(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
		+ UserStackSize;	// we need to increase the size
	// to leave room for the stack
	numPages = divRoundUp(size, PageSize);
	size = numPages * PageSize;

	memFull = false;
	pageMap = PageMap::GetInstance();
	/*
	if (numPages > (unsigned int) pageMap->NumClear()) {
		// printf("numPages(%d) > pageMap-NumClear()(%d)", numPages, pageMap->NumClear());
		memFull = true;
	}		// check we're not trying
	*/
	// to run anything too big --
	// at least until we have
	// virtual memory


	DEBUG('a', "Initializing address space, num pages %d, size %d\n",
			numPages, size);

	/*
	printf("Current status of pageMap before allocate:\n");
	for (i = 0; i < NumPhysPages; i++) {
		printf("%c%c", pageMap->Test(i) ? 'X' : 'O', (i != 0 && i % 10 == 0) ? '\n' : ' ');
	}
	printf("\n");
	*/
	// first, set up the translation
	pageTable = new TranslationEntry[numPages];
	if (machine->pageTable == NULL) machine->pageTable = pageTable;
	if (!memFull) {
		for (i = 0; i < numPages; i++) {
			pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
			// find first open pages
			pageTable[i].physicalPage = 0;
			// printf("Allocated physical page %d as virt page %d\n", pageTable[i].physicalPage,
			// pageTable[i].virtualPage);
			if (pageTable[i].physicalPage == -1) {
				memFull = true;
				break;
			}
			pageTable[i].valid = FALSE;
			pageTable[i].use = FALSE;
			pageTable[i].dirty = FALSE;
			pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
			// a separate page, we could set its
			// pages to be read-only
		}
	}

	printf("Current status of pageMap after allocate:\n");
	i = 0;
	while (i < NumPhysPages) {
		if (i % 8 == 0) printf("\n");
		printf("%c ", pageMap->Test(i) ? 'X' : 'O');
		i++;
	}
	printf("\n\n");
	// zero out the entire address space, to zero the unitialized data segment
	// and the stack segment
	// bzero(machine->mainMemory, size);

	// then, copy in the code and data segments into memory
	/*
	if (!memFull) {
		LoadMem(noffH.code.virtualAddr, noffH.code.size, noffH.code.inFileAddr, executable);
		LoadMem(noffH.initData.virtualAddr, noffH.initData.size, noffH.initData.inFileAddr,
				executable);
	}
	*/
}

void AddrSpace::LoadMem(int addr, int fileAddr, int size) {
	int pageNum;
	int cAddr;
	int offset;
	DEBUG('a', "writing %d bytes from 0x%x to 0x%x\n", size, fileAddr, addr); 
	for (int i = 0; i < size; i++) {
		pageNum = (addr + i) / PageSize;
		offset = (addr + i) - (pageNum * PageSize);
		cAddr = pageTable[pageNum].physicalPage * PageSize + offset;
		DEBUG('a', "pageNum: %d, offset: %d, cAddr: 0x%x\n", pageNum, offset, cAddr);
		executable->ReadAt(&(machine->mainMemory[cAddr]), 1, fileAddr + i);
	}
}

#endif
//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
	#ifdef CHANGED
	// return the pages we were using
	for (unsigned int i = 0; i < numPages; i++) {
		if (pageTable[i].physicalPage < NumPhysPages) {
			pageMap->Clear(pageTable[i].physicalPage);
		}
	}
	delete pageTable;
	delete executable;
	#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

	void
AddrSpace::InitRegisters()
{
	int i;

	for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister(i, 0);

	// Initial program counter -- must be location of "Start"
	machine->WriteRegister(PCReg, 0);

	// Need to also tell MIPS where next instruction is, because
	// of branch delay possibility
	machine->WriteRegister(NextPCReg, 4);

	// Set the stack register to the end of the address space, where we
	// allocated the stack; but subtract off a bit, to make sure we don't
	// accidentally reference off the end!
	machine->WriteRegister(StackReg, numPages * PageSize - 16);
	DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
	#ifdef CHANGED
	pageTable = machine->pageTable;
	numPages = machine->pageTableSize;
	#endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------
#ifdef CHANGED
void AddrSpace::RestoreState()
{
	machine->pageTable = pageTable;
	machine->pageTableSize = numPages;
}

/**
 * Check if the memFull bit was set on this AddrSpace instance,
 * indicating that there are not enough pages to be able to completely
 * load the program into memory.
 */
bool AddrSpace::GetFull() {
	return memFull;
}

/**
 * Sets the argument fields "argc" and "argv" for the this AddrSpace instance,
 * we load them into their correct places in memory in AddrSpace::LoadArguments.
 */
/*
void AddrSpace::SetArguments(int argc, char* argv[], char* filename) {

	char temp[128];
	this->argc = argc + 1;
	this->argv = new char*[argc];

	this->argv[0] = new char[strlen(filename) + 1];
	strcpy(this->argv[0], filename);

	printf("set arguments, argc: %d\n", argc);
	printf("copied filename: %s\n", this->argv[0]);

	for (int i = 0; i < argc; i++) {
		printf("argv[i]: %s\n", argv[i]);
		int arg_ptr;
		bzero(temp, 128);
		UserTranslate::ReadMem((int) argv + 4 * i, 4, &arg_ptr);
		ReadString(arg_ptr, temp);
		this->argv[i + 1] = new char[strlen(temp) + 1];
		strcpy(this->argv[i + 1], temp);
		printf("got argument %d: %s\n", i, temp);
	}

}
*/
/**
 * Should load the arguments into their correct places in memory, just before
 * the code section, so the user program can access them.
void AddrSpace::LoadArguments(){
	int args[argc];
	int strLen;

	int sp = numPages * PageSize;
	int cantPag = 0;
	int tmpSize = 4 * argc;

	for (int i = 0; i < argc; i++) {
		strLen = strlen(argv[i]) + 1;
		tmpSize += strLen;
		if (tmpSize > PageSize * cantPag ){
			cantPag += 1;
			pageTable[numPages - cantPag].valid =  true;
			bzero((machine->mainMemory) + (pageTable[numPages - cantPag]
						.physicalPage * PageSize), PageSize);
		}
		sp = sp - strLen;
		WriteString(sp, argv[i], strLen);
		args[i] = sp;
	}

	sp = sp - argc * 4;
	sp = sp - sp % 4;
	machine->WriteRegister(StackReg, sp - 16);
	machine->WriteRegister(4, argc);
	machine->WriteRegister(5, sp);

	for (int i = 0; i < argc; i++) {
		machine->WriteMem(sp, 4, args[i]);
		sp = sp + 4;
	};

}
*/

int AddrSpace::GetNumPages() {
	return numPages;
}
#endif

void AddrSpace::LoadPage(int vAddr) {

	// check physical page bitmap
	int vPage = vAddr / PageSize;
	int newPage;

	// We need that core map that maps back from a
	// physical page to a virtual page. I think I
	// got this. each virtual address is specifc
	// to the program. We need not only a map from
	// frame number to virtual page number, but
	// a map from the frame number to the thread as
	// well as the corresponding virtual page number.

	if (pageMap->NumClear() <= 0) {
		newPage = pageMap->GetNextVictim();
		DEBUG('v', "memory full, selecting victim for replacement: %d\n", newPage);
	} else {
		// else grab first open one
		newPage = pageMap->Find();
		DEBUG('v', "going to load page into main mem frame: %d\n", newPage);
	}

	// set frame number
	pageTable[vPage].physicalPage = newPage;
	
	int startAddr = vPage * PageSize;
	int addr;
	for (int i = 0; i < PageSize; i++) {
		addr = startAddr + i;
		if (addr >= noffH.code.virtualAddr &&
				addr < noffH.code.virtualAddr + noffH.code.size) {
			LoadMem(addr, noffH.code.inFileAddr + (startAddr - noffH.code.virtualAddr + i), 1);
		} else if (addr >= noffH.initData.virtualAddr &&
				addr < noffH.initData.virtualAddr + noffH.initData.size) {
			LoadMem(addr, noffH.initData.inFileAddr + (startAddr - noffH.initData.virtualAddr + i), 1);
		}
	}

	// mark page table entry as valid
	pageTable[vPage].valid = TRUE;
	DEBUG('v', "vpage %d should now be loaded in phys frame %d..\n", vPage, newPage);
}
