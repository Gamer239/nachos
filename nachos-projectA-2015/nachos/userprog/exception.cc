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

void ExceptionHandler(ExceptionType which) {
	int type = machine->ReadRegister(2);

	if (which == SyscallException) {
		switch (type) {
			case SC_Halt:
				printf("Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;

			case SC_Exit:
				printf("Call to Syscall Exit (SC_Exit).\n");
				currentThread->space->RestoreState();
				// need to make sure we actually set reg 2 to the return value	
				machine->WriteRegister(2, 0); 
				break;

			case SC_Exec:
				printf("Call to Syscall Exec (SC_Exec).\n");
				printf("machine->ReadRegister(4): %s\n", (char *) machine->ReadRegister(4));
				// StartProcess(machine->ReadRegister(4));
				break;

			case SC_Join:
				printf("Call to Syscall Join (SC_Join).\n");
				break;

			case SC_Create:
				printf("Call to Syscall Create (SC_Create).\n");
				break;

			case SC_Open:
				printf("Call to Syscall Open (SC_Open).\n");
				break;

			case SC_Read: {
				// printf("Call to Syscall Read (SC_Read).\n");
				int size = (int) machine->ReadRegister(5);
				OpenFileId fileid = (int) machine->ReadRegister(6);
				// printf("size: %d, id: %d\n", size, fileid);
				// OpenFile file = OpenFile(fileid);
				// printf("file opened\n");
				// char* buffer = (char *) machine->WriteMem(machine->ReadRegister(4));
				break;
						  }

			case SC_Write:
				printf("Call to Syscall Write (SC_Write).\n");
				break;

			case SC_Close:
				printf("Call to Syscall Close (SC_Close).\n");
				break;

			case SC_Fork:
				printf("Call to Syscall Fork (SC_Fork).\n");
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
