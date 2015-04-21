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
#include "exception_impl.h"
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
	char file_name[200];
	executable = execFile;
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
			(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	sprintf(file_name, "/tmp/swap.%d", (int) this);
	ASSERT(fileSystem->Create(file_name, 0));
	swap = fileSystem->Open(file_name);

	// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
		+ UserStackSize;	// we need to increase the size
	// to leave room for the stack
	numPages = divRoundUp(size, PageSize);
	size = numPages * PageSize;

	memFull = false;
	pageMap = PageMap::GetInstance();

	DEBUG('a', "Initializing address space, num pages %d, size %d\n",
			numPages, size);

	// first, set up the translation
	pageTable = new TranslationEntry[numPages];
	if (machine->pageTable == NULL) machine->pageTable = pageTable;
	if (!memFull) {
		for (i = 0; i < numPages; i++) {
			pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
			// find first open pages
			pageTable[i].physicalPage = NOT_LOADED;
			pageTable[i].valid = FALSE;
			pageTable[i].use = FALSE;
			pageTable[i].dirty = FALSE;
			pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
			// a separate page, we could set its
			// pages to be read-only
		}
	}
	
}

void AddrSpace::LoadMem(int addr, int fileAddr, int size) {
	int pageNum;
	int cAddr;
	int offset;
	for (int i = 0; i < size; i++) {
		pageNum = (addr + i) / PageSize;
		offset = (addr + i) - (pageNum * PageSize);
		cAddr = pageTable[pageNum].physicalPage * PageSize + offset;
		executable->ReadAt(&(machine->mainMemory[cAddr]), 1, fileAddr + i);
	}
}

void AddrSpace::SaveToSwap(int vPage) {
	swap->WriteAt(&(machine->mainMemory[pageTable[vPage].physicalPage * PageSize]), 
			PageSize, vPage * PageSize);
	pageTable[vPage].physicalPage = IN_SWAP;
	pageTable[vPage].valid = false;
}

void AddrSpace::GetFromSwap(int vpage) {
	char page[PageSize];
	// ASSERT(coreMap->Check(vpage, pageTable[vpage].physicalPage));
	swap->ReadAt(page, PageSize, vpage*PageSize);
	for (int i = 0; i < PageSize; i++) {
		machine->mainMemory[pageTable[vpage].physicalPage * PageSize + i] = page[i];
	}
	DEBUG('v', "[VMEM] Loaded: vitrPage %d of thread %s to phys page %d\n", 
			vpage, currentThread->getName(), pageTable[vpage].physicalPage);
	pageTable[vpage].valid = true;
	pageTable[vpage].readOnly = false;
	pageTable[vpage].use = false;
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
			if (pageTable[i].physicalPage >= 0) {
				pageMap->Clear(pageTable[i].physicalPage);
			}
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
void AddrSpace::SetArguments(int argVec, char* filename) {
	if (argVec == 0) return;
	argc = 0;
	char* ptr;
	char** args = (char**) argVec;
	char temp[128];
	int arg_ptr;
	
	while (true) {
		machine->ReadMem(argVec + 4 * argc, 4, &arg_ptr);
		if (arg_ptr == 0) break;
		else argc++;
	}
	argc++;
	// printf("total of %d args\n", argc);

	argv = new char*[argc];

	argv[0] = new char[strlen(filename) + 1];
	strcpy(argv[0], filename);

	for (int i = 0; i < argc - 1; i++) {
		machine->ReadMem(argVec + 4 * i, 4, &arg_ptr);		
		ReadString(arg_ptr, temp);
		// printf("got temp of %s\n", temp);
		// printf("i + 1: %d\n", i + 1);
		argv[i + 1] = new char[strlen(temp) + 1];
		// printf("made the thing\n");
		strcpy(argv[i + 1], temp);
		// printf("its this!: %s\n", argv[i + 1]);
	}

}

/**
 * Should load the arguments into their correct places in memory, just before
 * the code section, so the user program can access them.
 */
void AddrSpace::LoadArguments(){
	
	int args[argc];
	int strLen;

	int sp = machine->ReadRegister(StackReg);

	for (int i = 0; i < argc; i++) {
		strLen = strlen(argv[i]) + 1;
		sp -= strLen;
		WriteString(sp, argv[i], strLen);
		args[i] = sp;
	}

	sp = sp & ~3;

	for (int i = 0; i < argc; i++) {
		machine->WriteMem(sp, 4, args[i]);
		sp += 4;
	}

	sp -= argc * sizeof(int);

	machine->WriteRegister(5, sp);
	machine->WriteRegister(4, argc);
	machine->WriteRegister(StackReg, sp - 4 * argc);
	
}

int AddrSpace::GetNumPages() {
	return numPages;
}
#endif

void AddrSpace::LoadPage(int vAddr) {

	// check physical page bitmap
	int vPage = vAddr / PageSize;
	int newPage;
	
	// are there open frames?
	if (pageMap->NumClear() <= 0) {
		
		// yes, find victim page
		// get frame number, and vpn of the page for the
		// process that holds it.
		newPage = pageMap->GetNextVictim();

		// we now have the frame # that will be replaced.
		DEBUG('v', "[VMEM] memory full, selecting %d as victim.\n", newPage);
		
		// we need to check the victim page's thread's pagetable
		// what thread is that?
		// what vpn is that frame to it?
		Thread* victimThread = pageMap->curThreads[newPage];
		int victimVPage = pageMap->vPage[newPage];
		pageMap->curThreads[newPage] = currentThread;
		pageMap->vPage[newPage] = vPage;

		DEBUG('v', "[VMEM] got victim thread, victim vpn %d\n", victimVPage);

		// is victim page dirty?
		if (victimThread->space->pageTable[victimVPage].dirty) {
			// yes, store it.
			DEBUG('r', "[VMEM] victim page %d is dirty, saving to swap\n", victimVPage);
			victimThread->space->SaveToSwap(victimVPage);
			victimThread->space->pageTable[victimVPage].physicalPage = IN_SWAP;	
		} else {
			// no. just mark it as not loaded
			DEBUG('r', "[VMEM] victim page %d is not dirty\n", victimVPage);
			victimThread->space->pageTable[victimVPage].physicalPage = NOT_LOADED;
		}
		victimThread->space->pageTable[victimVPage].valid = false;

	} else {
		// no, grab first open one and set intime, curthread, and 
		// the vpn that we're using for that frame.
		newPage = pageMap->Find(vPage);
		DEBUG('v', "[VMEM] loading into frame %d\n", newPage);
	}

	if (pageTable[vPage].physicalPage == IN_SWAP) {
		DEBUG('v', "[VMEM] Virtual page stored in swap.\n");
		pageTable[vPage].physicalPage = newPage;
		GetFromSwap(vPage);
	} else if (pageTable[vPage].physicalPage == NOT_LOADED) {
		DEBUG('v', "[VMEM] Virtual page not loaded.\n");
		pageTable[vPage].physicalPage = newPage;
		LoadPageFromExecutable(vPage);
	} else {
		printf("physical page %d, vPage %d\n", pageTable[vPage].physicalPage, vPage);
		ASSERT(false); // shouldn't happen
	}
	
	// mark page table entry as valid
	pageTable[vPage].valid = TRUE;
	DEBUG('v', "[VMEM] vPage %d now loaded in phys frame %d\n", vPage, newPage);
}

void AddrSpace::InvalidatePage(int vpn) {
	DEBUG('v', "[VMEM] invalidating virt page %d\n", vpn);
	this->pageTable[vpn].valid = false;
}

void AddrSpace::LoadPageFromExecutable(int vPage) {
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
}

void AddrSpace::PrintPages() {
	// printf("were in printpages, numpages: %d\n", numPages);
	for (unsigned int i = 0; i < NumPhysPages; i++) {
		// if (pageTable[i].physicalPage >= 0) {
			printf("Page %d: ", i);
			for (int j = 0; j < PageSize; j++) {
				char ch = machine->mainMemory[(i * PageSize) + j];
				if (ch < 32 || ch > 127) {
					// ch = ' ';
					printf("%d", (int)ch);
				} else {
					printf("%c", ch);
				}
			}
			printf("\n");
		//}
	}

bool AddrSpace::readAddrState( OpenFile* fileId )
{
	char value;
	char small_buf[4];
	int result = 0;

	//read in the number of pages that we will need to add
	result = fileId->Read(small_buf, 4);
	int newNumPages = charToInt(small_buf);

	//we should have already instantiated our address space with the code
	//so the number of pages should already be mapped and now we can load
	//our state back into place
	if (newNumPages <= 0 || numPages != newNumPages)
	{
		printf("ERROR - newNumPages <=0 || numPages != newNumPage, %d, %d\n", newNumPages, numPages);
		return false;
	}

	//load each page and check that there is still free space left
	for ( int i = 0; i < numPages && pageMap->NumClear() > 0; i++ )
	{
		//load the current info
		TranslationEntry entry = pageTable[i];

		//read the dirty bit
		result = fileId->Read(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - read the dirty bit, %d\n", result);
			return false;
		}
		entry.dirty = value;

		//read the readOnly status
		result = fileId->Read(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - read the readonly status, %d\n", result);
			return false;
		}
		entry.readOnly = value;

		//read the usebool
		result = fileId->Read(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - read the usebool, %d\n", result);
			return false;
		}
		entry.use = value;

		//read the valid bit
		result = fileId->Read(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - read the valid bit, %d\n", result);
			return false;
		}
		entry.valid = value;

		//read the virtualpage number
		result = fileId->Read(small_buf, 4);
		if ( result < 4 )
		{
			printf("ERROR - write the virtualpage number, %d\n", result);
			return false;
		}
		entry.virtualPage = charToInt(small_buf);

		//dont take a physical page if we weren't using it before
		if (entry.valid == false)
		{
			if (entry.physicalPage >= 0)
			{
				pageMap->Clear(entry.physicalPage);
			}
		}
		else
		{
			//find an unused page
			if (entry.physicalPage == NOT_LOADED)
			{
				//TODO: could we ever have a victim page that was just one that was loaded which then corrupts our system state?
				if (pageMap->NumClear() > 0)
				{
					entry.physicalPage = pageMap->Find(entry.virtualPage);
				}
				else
				{
					// yes, find victim page
					// get frame number, and vpn of the page for the
					// process that holds it.
					entry.physicalPage = pageMap->GetNextVictim();

					// we now have the frame # that will be replaced.
					DEBUG('v', "[VMEM] memory full, selecting %d as victim.\n", entry.physicalPage);

					// we need to check the victim page's thread's pagetable
					// what thread is that?
					// what vpn is that frame to it?
					Thread* victimThread = pageMap->curThreads[entry.physicalPage];
					int victimVPage = pageMap->vPage[entry.physicalPage];
					pageMap->curThreads[entry.physicalPage] = currentThread;
					pageMap->vPage[entry.physicalPage] = entry.virtualPage;

					DEBUG('v', "[VMEM] got victim thread, victim vpn %d\n", victimVPage);

					// is victim page dirty?
					if (victimThread->space->pageTable[victimVPage].dirty) {
						// yes, store it.
						DEBUG('r', "[VMEM] victim page %d is dirty, saving to swap\n", victimVPage);
						victimThread->space->SaveToSwap(victimVPage);
						victimThread->space->pageTable[victimVPage].physicalPage = IN_SWAP;
					} else {
						// no. just mark it as not loaded
						DEBUG('r', "[VMEM] victim page %d is not dirty\n", victimVPage);
						victimThread->space->pageTable[victimVPage].physicalPage = NOT_LOADED;
					}
					victimThread->space->pageTable[victimVPage].valid = false;
				}
			}
			//its in swap
			else if ( entry.physicalPage == IN_SWAP )
			{
				GetFromSwap(entry.virtualPage);
			}
			//TODO: Don't assume that it's valid if its not loaded or in swap
			else
			{
				//its loaded for us let's just replace the data with our checkpoint
				//TODO: we shouldn't need to do anything here for now
			}
		}

		//save the current info
		pageTable[i] = entry;

		//read the contents of the page to the file
		result = fileId->Read(&(machine->mainMemory[entry.physicalPage * PageSize]), PageSize);
		if ( result < PageSize )
		{
			printf("ERROR - read the contents of the page to the file, %d\n", result);
			return false;
		}

	}

	//check to make sure that we have some free pages open still
	if ( pageMap->NumClear() <= 0 )
	{
		printf("ERROR - check to make sure that we have some free pages open still, %d\n", pageMap->NumClear());
		return false;
	}

	return true;
}

bool AddrSpace::writeAddrState( OpenFile* fileId )
{
	char value;
	char small_buf[4];
	int result = 0;

	//write the number of pages that we have to the file
	intToChar(numPages, small_buf);
	result = fileId->Write(small_buf, 4);
	if ( result < 4 )
	{
		printf("ERROR - write the number of pages that we have to the file, result\n", result);
		return false;
	}

	for ( int i = 0; i < numPages; i++ )
	{
		TranslationEntry entry = pageTable[i];

		//write the dirty bit
		value = entry.dirty;
		result = fileId->Write(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - write the dirty bit, %d\n", result);
			return false;
		}

		//write the readOnly status
		value = entry.readOnly;
		result = fileId->Write(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - write the readonly status, %d\n", result);
			return false;
		}

		//write the usebool
		value = entry.use;
		result = fileId->Write(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - write the usebool, %d\n", result);
			return false;
		}

		//write the valid bit
		value = entry.valid;
		result = fileId->Write(&value, 1);
		if ( result < 1 )
		{
			printf("ERROR - write the valid bit, %d\n", result);
			return false;
		}

		//write the virtualpage number
		intToChar(entry.virtualPage, small_buf);
		result = fileId->Write(small_buf, 4);
		if ( result < 4 )
		{
			printf("ERROR - write the virtualpage number, %d\n", result);
			return false;
		}
		printf("writing virtual page %d %d %d\n", entry.virtualPage, entry.dirty, entry.physicalPage);

		//write the contents of the page to the file
		result = fileId->Write(&(machine->mainMemory[entry.physicalPage * PageSize]), PageSize);
		if ( result < PageSize )
		{
			printf("ERROR - write the contents of the page to the file, %d\n", result);
			return false;
		}

	}

	return true;
}
