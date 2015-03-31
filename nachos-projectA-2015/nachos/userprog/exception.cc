// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "process.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void startProcess(int n) {
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	// currentThread->space->LoadArguments();
	printf("[IN startProcess]: currentThread is: %s\n", currentThread->getName());
	machine->Run();
	// ASSERT(false);
}

SpaceId exec(char *filename) {
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL) {
		printf("SC_Exec Error: Unable to open file %s\n", filename);
		return -1;
	}

	Thread *thread = new Thread(filename, 0);
	space = new AddrSpace(executable);
	Process *process = new Process(thread, space->GetId());

	delete executable;

	// check if the returned address space failed to allocate
	// the number of pages we needed
	if (space->GetFull()) {
		printf("Error: Exec: Insufficient memory to start process\n");
		delete space;
		return -1;
	}

	thread->space = space;
	// assign the new space to the thread
	// put the thread on the ready queue?

	thread->Fork(startProcess, 0);
	currentThread->Yield();

	return space->GetId();


}

void ExceptionHandler(ExceptionType which) {
	int type = machine->ReadRegister(2);

	char c, buf[64], filename[64];
	bzero(buf, 64);
	int addr, i = 0;

	if (which == SyscallException) {
		switch (type) {
			case SC_Halt:
				printf("Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;

			case SC_Exit:
				printf("Call to Syscall Exit (SC_Exit).\n");
				// I don't think we need to care about the value passed to use
				// by the user program's Exit call.
				delete currentThread->space;
				currentThread->space = NULL;
				printf("SC_EXIT: currentThread is: %s\n", currentThread->getName());
				currentThread->Finish();
				machine->WriteRegister(2, 0); 
				break;

			case SC_Exec: 
				{
					addr = machine->ReadRegister(4);
					do {
						machine->ReadMem(addr + i, 1, (int *) &c);
						sprintf(buf + strlen(buf), "%c", c);
						i++;
					} while (c != '\0');

					printf("Call to Syscall Exec (SC_Exec).\n");
					printf("Execing: %s\n", buf); 
					int ret = exec(buf);

					machine->WriteRegister(2, ret);
					break;
				}

			case SC_Join: {
							  printf("Call to Syscall Join (SC_Join).\n");
							  printf("join(spaceid = %d)\n", machine->ReadRegister(4));
							  break;
						  }

			case SC_Create: {
								printf("Call to Syscall Create (SC_Create).\n");
								addr =  machine->ReadRegister(4); // char* filename arg, we need to read this buf
								while (c != '\0' && i < 64) {
									machine->ReadMem(addr + i, 1, (int *) &c);
									sprintf(buf + strlen(buf), "%c", c);
									i++;
								}

								// not sure how big the file should be.
								// works for now thought
								fileSystem->Create(buf, 128);

								// no return value for Create
								break;
							}

			case SC_Open: {
							  printf("Call to Syscall Open (SC_Open).\n");
							  addr =  machine->ReadRegister(4); // char* filename arg, we need to read this buf
							  while (c != '\0' && i < 64) {
								  machine->ReadMem(addr + i, 1, (int *) &c);
								  sprintf(buf + strlen(buf), "%c", c);
								  i++;
							  }

							  // this is not right
							  int fileId = (int) fileSystem->Open(buf);
							  printf("got file: %d\n", fileId);
							  machine->WriteRegister(2, fileId);
							  break;
						  }

			case SC_Read: {
							  addr = machine->ReadRegister(4);
							  int size = machine->ReadRegister(5);
							  OpenFileId fileid = machine->ReadRegister(6);

							  int read = 0;
							  char buf;
							  bool write;
							  while (read < size) {
								  Read(fileid, &buf, 1);
								  write = machine->WriteMem(addr + read, 1, (int) buf);
								  read++;
							  }

							  machine->WriteRegister(2, read);
							  break;
						  }

			case SC_Write: {

							   char * addr = (char *) machine->ReadRegister(4);
							   int size = (int) machine->ReadRegister(5);
							   OpenFileId fileid = (int) machine->ReadRegister(6);

							   int wrote = 0;
							   char buf;
							   bool read;
							   while (wrote < size && buf != EOF) {
								   read = machine->ReadMem((int) (addr + wrote), 1, (int*) &buf);
								   WriteFile(fileid, &buf, 1);
								   wrote++;
							   }

							   machine->WriteRegister(2, read);

							   break;
						   }

			case SC_Close:
						   printf("Call to Syscall Close (SC_Close).\n");
						   break;

			case SC_Fork:
						   printf("Call to Syscall Fork (SC_Fork).\n");
						   printf("fork(%d)\n", machine->ReadRegister(4));
						   break;

			case SC_Yield:
						   printf("Call to Syscall Yield (SC_Yield).\n");
						   break;

			default:
						   printf("Unexpected user mode exception %d %d\n", which, type);
						   ASSERT(FALSE);
						   break;
		}
	}

	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PrevPCReg) + 4);
	machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
	machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);

}


