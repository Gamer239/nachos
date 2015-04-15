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
#include "thread.h"
#ifdef CHANGED
#include "exception_impl.h"
#include "exception_utils.h"
#include "synchconsole.h"
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

	IntStatus oldLevel = interrupt->SetLevel(IntOff); // yolo
	int type = machine->ReadRegister(2);
	bool sysCall = false;


	switch (which) {
		case SyscallException:
			{
				sysCall = true;
				HandleSyscall(type);
				break;
			}

		case PageFaultException: 
			{
				Process* currentProcess = getCurrentProcess();
				if (machine->registers[BadVAddrReg] / PageSize >=
						currentThread->space->GetNumPages()) {
					// printf("Page fault: virtual address out of range: %d/%d - exiting process\n",
					//	   	machine->registers[BadVAddrReg], currentThread->space->GetNumPages());
					exit(-1);
				} else {
					// printf("Page fault: page not loaded into main memory - loading page now\n");
					currentThread->space->LoadPage(machine->registers[BadVAddrReg]);
				}
				break;
			}

		case ReadOnlyException:
			{
				printf("Write attempted on read-only page - Exiting\n");
				exit(-1);
				break;
			}

		case BusErrorException:
			{
				printf("Translation resulted in invalid physical address - Exiting\n");
				exit(-1);
				break;
			}
		
		case AddressErrorException:
			{
				printf("Exception: Unaligned/Out of address space reference - Exiting\n");
				exit(-1);
				break;
			}
		
		case OverflowException:
			{
				printf("WARNING: Integer overflow in add or sub\n");
				break;
			}	
		
		case IllegalInstrException:
			{
				printf("Illegal instruction - Exiting\n");
				exit(-1);
				break;
			}

	}

	if (sysCall) {
		machine->WriteRegister(PrevPCReg, machine->ReadRegister(PrevPCReg) + 4);
		machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
		machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
	}
	interrupt->SetLevel(oldLevel);
}

#endif
