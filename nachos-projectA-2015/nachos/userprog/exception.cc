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
	interrupt->SetLevel(IntOff);
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL) {
		printf("SC_Exec Error: Unable to open file %s\n", filename);
		return -1;
	}

	(void) interrupt->SetLevel(IntOff);
	Thread *thread = new Thread(filename, 0);
	space = new AddrSpace(executable);
	int parentId = currentThread->GetId();

	Process *child = new Process(thread, thread->GetId());
	Process *parent;

	// in case our parent thread somehow was not put into the procmap
	if (Process::GetProcMap()->find(parentId) != Process::GetProcMap()->end()) {
		parent = Process::GetProcMap()->at(parentId);
		printf("EXEC[%s]: Parent of %s is %s\n", currentThread->getName(), thread->getName(), parent->GetThread()->getName());
	} else {
		printf("EXEC[%s]: we didn't find ourself in the procmap\n", currentThread->getName());
		parent = new Process(currentThread, parentId);
		Process::GetProcMap()->insert(std::pair<int, Process*>(parentId, parent));
	}
	parent->AddChild(child);

	child->SetParent(parent);

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

	interrupt->Enable();
	return child->GetThread()->GetId();

}

#define BUFFER_SIZE 256

void ExceptionHandler(ExceptionType which) {
	int type = machine->ReadRegister(2);

	char c, buf[BUFFER_SIZE], filename[BUFFER_SIZE];
	bzero(buf, BUFFER_SIZE);
	int addr, i = 0;

	if (which == SyscallException) {
		switch (type) {
			case SC_Halt:
				printf("Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;

			case SC_Exit:
				{
					printf("Call to Syscall Exit (SC_Exit).\n");
					// I don't think we need to care about the value passed to use
					// by the user program's Exit call.

					Process * currentProcess;
					if (Process::GetProcMap()->find(currentThread->GetId()) !=
							Process::GetProcMap()->end()) {
						printf("current process found in procmap\n");
						currentProcess = Process::GetProcMap()->at(currentThread->GetId());
					} else {
						ASSERT(false); // should never happen
					}

					Process * parent = currentProcess->GetParent();
					Thread * parentThread = parent->GetThread();
					if (parent != NULL && currentThread->GetStatus() != ZOMBIE) { // I have parent
						printf("I have a parent\n");
						currentProcess->SetReturnValue(machine->ReadRegister(4));
						if (parentThread->GetStatus() == JOINING) {
							printf("my(%d) parent(%d) is waiting for me\n", currentThread->GetId(), parentThread->GetId());
							parentThread->joinSem->V();
						 } else {
							printf("my(%d) parent(%d) is not blocked(%d)\n", currentThread->GetId(), parentThread->GetId(), parentThread->GetStatus());
						 }

					}

					printf("gonna check for children\n");
					if (!currentProcess->GetChildren()->IsEmpty()) {
						printf("I have children\n");
						currentProcess->GetChildren()->Mapcar(Process::SetZombie);
					}

					Process::GetProcMap()->erase(currentThread->GetId());

					delete currentProcess;
					delete currentThread->space;
					currentThread->space = NULL;
					printf("SC_EXIT: currentThread is: %s\n", currentThread->getName());
					currentThread->Finish();
					machine->WriteRegister(2, 0);
					break;
				}
			case SC_Exec:
				{
					addr = machine->ReadRegister(4);
					do {
						machine->ReadMem(addr + i, 1, (int *) &c);
						sprintf(buf + strlen(buf), "%c", c);
						i++;
					} while (c != '\0');

					printf("Call to Syscall Exec (SC_Exec).\n");
					int ret = exec(buf);
					printf("ret: %d\n", ret);
					machine->WriteRegister(2, ret);
					break;
				}

			case SC_Join:
				{
					printf("Call to Syscall Join (SC_Join) from %s.\n", currentThread->getName());
					scheduler->Print();
					printf("join(spaceid = %d)\n", machine->ReadRegister(4));
					int pid = machine->ReadRegister(4);
					int retVal;

					IntStatus oldLevel = interrupt->SetLevel(IntOff);
					Process* target;
					Thread* targetParent;
					// target process is in procmap
	
					if (Process::GetProcMap()->find(pid) != Process::GetProcMap()->end()) {
<<<<<<< HEAD
						printf("JOIN - found pid in procmap!\n");
						target = Process::GetProcMap()->at(pid);
						targetParent = target->GetParent()->GetThread();
						// targets parent is us
						if (targetParent->GetId() == currentThread->GetId()) {
							printf("JOIN - target(%d)s parent(%d) is us(%d)\n", pid,
									targetParent->GetId(), currentThread->GetId());
							if (target->GetThread()->GetId() != FINISHED) {
								printf("JOIN - target is not finished, waiting\n");
								currentThread->setStatus(JOINING);
								currentThread->WaitOnReturn();
							}
							printf("JOIN - target is done, getting return value\n");
							retVal = target->GetReturnValue();
						} else {
							printf("JOIN - target(%d)s parent(%d) is not us(%d)\n", pid,
									targetParent->GetId(), currentThread->GetId());
							retVal = -1;
						}
					} else {
						printf("JOIN - did not find pid in procmap\n");
						retVal = -1;
=======
						// this process
>>>>>>> 1d15203f2ec859224be2e9a4bcc734f91f765a72
					}
					interrupt->SetLevel(oldLevel);
					printf("JOIN - retVal: %d\n", retVal);
					machine->WriteRegister(2, retVal);
					break;
				}

			case SC_Create:
				{
					printf("Call to Syscall Create (SC_Create).\n");
<<<<<<< HEAD
					// char* filename arg, we need to read this buf

					addr =  machine->ReadRegister(4);
					printf("the address is %d\n", addr);
=======
					addr =  machine->ReadRegister(4); // char* filename arg, we need to read this buf
					//printf("the address is %d\n", addr);
>>>>>>> 1d15203f2ec859224be2e9a4bcc734f91f765a72
					bzero( buf, BUFFER_SIZE);
					i=0;
					c='1';
					while (c != '\0' && i < BUFFER_SIZE) {
						machine->ReadMem(addr + i, 1, (int *) &c);
						//sprintf(buf + strlen(buf), "%c", c);
						//printf("%c", c);
						buf[i]=c;
						i++;
					}

					//printf("total i's %d\n", i);

					//TODO: remove this line below
					//printf("\ncreating file %s|\n", buf);

					// a file size of zero is fine for now
					// we'll add data once we open it
					fileSystem->Create(buf, 0);

					// no return value for Create
					break;
				}

			case SC_Open: {
							  printf("Call to Syscall Open (SC_Open).\n");
							  addr =  machine->ReadRegister(4); // char* filename arg, we need to read this buf
							  c = '1';
							  i = 0;
							  while (c != '\0' && i < BUFFER_SIZE) {
								  machine->ReadMem(addr + i, 1, (int *) &c);
								  //sprintf(buf + strlen(buf), "%c", c);
								  buf[i]=c;
								  i++;
							  }

							  OpenFile* fileId = fileSystem->Open(buf);
							  //find the first available key
							  int mapped_id = 0;
							  while( currentThread->fileHandlers->find( mapped_id ) != currentThread->fileHandlers->end( ) && fileId != NULL )
							  {
								  mapped_id++;
							  }
							  if ( fileId != NULL )
							  {
								  currentThread->fileHandlers->insert( std::pair<int, OpenFile*>( mapped_id, fileId ) );
							  }
							  else
							  {
								  mapped_id = -1;
							  }
							  //printf("open id is %d\n", mapped_id);
							  machine->WriteRegister(2, mapped_id);
							  break;
						  }

			case SC_Read: {
							  addr = machine->ReadRegister(4);
							  int size = machine->ReadRegister(5);
							  int mapped_id = machine->ReadRegister(6);

							  int read = 0;
							  char buff;
							  bool write;
								int bytes_read = 1;
							  //check to make sure that our file handler exists
							  if ( currentThread->fileHandlers->find( mapped_id ) != currentThread->fileHandlers->end( ) )
							  {
								  OpenFile* fileId = currentThread->fileHandlers->at( mapped_id );
								  while (read < size && bytes_read > 0)
								  {
									  if ( mapped_id == ConsoleInput )
									  {
										  Read( ConsoleInput, &buff, 1 );
									  }
									  else if ( mapped_id != ConsoleOutput )
									  {
										  bytes_read = fileId->Read(&buff, 1);
									  }
									  else
									  {
										  break;
									  }
									  //printf("%c", buff);
									  write = machine->WriteMem( addr + read, 1, ( int ) buff );
									  read++;
								  }
							  }
							  else
							  {
								  //TODO: we never read anything because that file isn't open, handle this?
								  printf( "Error reading file %d\n", mapped_id );
							  }
							  //printf("\n");

							  machine->WriteRegister(2, read);
							  break;
						  }

<<<<<<< HEAD
			case SC_Write:
						  {
							  char * addr = (char *) machine->ReadRegister(4);
							  int size = (int) machine->ReadRegister(5);
							  const int mapped_id = (int) machine->ReadRegister(6);

							  //printf( "addr %d size %d mapped_id %d\n", addr, size, mapped_id);

							  int wrote = 0;
							  int value;
							  char buff;
							  bool read;
							  //printf( "addr %d size %d mapped_id %d\n", addr, size, mapped_id);
							  if ( currentThread->fileHandlers->find( mapped_id ) != currentThread->fileHandlers->end( ) )
							  {
								  OpenFile* fileId = currentThread->fileHandlers->at( mapped_id );
								  //printf( "addr %d size %d mapped_id %d 256\n", addr, size, mapped_id);
								  while( wrote < size && buff != EOF )
								  {
									  //printf( "addr %d size %d mapped_id %d 259\n", addr, size, mapped_id);
									  read = machine->ReadMem((int) (addr + wrote), 1, &value);
									  buff = value;
									  //printf( "addr %d size %d mapped_id %d 261\n", addr, size, mapped_id);
									  //printf("%c", buff);
									  if ( mapped_id == ConsoleOutput )
									  {
										  WriteFile( ConsoleOutput, &buff, 1 );
									  }
									  else if ( mapped_id != ConsoleInput )
									  {
										  fileId->Write(&buff, 1);
									  }
									  else
									  {
										  //printf("mapped_id %d consoleinput %d\n", mapped_id, ConsoleInput);
										  break;
									  }
									  wrote++;
=======
			case SC_Write: {

							   char * addr = (char *) machine->ReadRegister(4);
							   int size = (int) machine->ReadRegister(5);
							   const int mapped_id = (int) machine->ReadRegister(6);

							   //printf( "addr %d size %d mapped_id %d\n", addr, size, mapped_id);

							   int wrote = 0;
							   int value;
							   char buff;
							   bool read;
								int bytes_written = 1;
							   //printf( "addr %d size %d mapped_id %d\n", addr, size, mapped_id);
							   if ( currentThread->fileHandlers->find( mapped_id ) != currentThread->fileHandlers->end( ) )
							   {
								   OpenFile* fileId = currentThread->fileHandlers->at( mapped_id );
								   //printf( "addr %d size %d mapped_id %d 256\n", addr, size, mapped_id);
								   while( wrote < size && buff != EOF && bytes_written > 0 )
								   {
									   //printf( "addr %d size %d mapped_id %d 259\n", addr, size, mapped_id);
									   read = machine->ReadMem((int) (addr + wrote), 1, &value);
									   buff = value;
									   //printf( "addr %d size %d mapped_id %d 261\n", addr, size, mapped_id);
									   //printf("%c", buff);
									   if ( mapped_id == ConsoleOutput )
									   {
										   WriteFile( ConsoleOutput, &buff, 1 );
									   }
									   else if ( mapped_id != ConsoleInput )
									   {
										   bytes_written = fileId->Write(&buff, 1);
									   }
									   else
									   {
										   //printf("mapped_id %d consoleinput %d\n", mapped_id, ConsoleInput);
										   break;
									   }
									   wrote++;
								   }
							   }
							   else
							   {
								   //TODO: we never write anything because the file isnt open, handle this?
								   printf( "Error writing file %d\n", mapped_id );
							   }
							   // printf("\nWrote %d bytes\n", wrote);
							   /*
								  while (wrote < size && buf != EOF) {
								  read = machine->ReadMem((int) (addr + wrote), 1, (int*) &buf);
								  WriteFile(fileid, &buf, 1);
								  wrote++;
>>>>>>> 1d15203f2ec859224be2e9a4bcc734f91f765a72
								  }
							  }
							  else
							  {
								  //TODO: we never write anything because the file isnt open, handle this?
								  printf( "Error writing file %d\n", mapped_id );
							  }
							  // printf("\nWrote %d bytes\n", wrote);
							  /*
								 while (wrote < size && buf != EOF) {
								 read = machine->ReadMem((int) (addr + wrote), 1, (int*) &buf);
								 WriteFile(fileid, &buf, 1);
								 wrote++;
								 }
								 */

							  machine->WriteRegister(2, wrote);

							  break;
						  }

			case SC_Close:
						  {
							  printf("Call to Syscall Close (SC_Close).\n");
							  int mapped_id = (int) machine->ReadRegister(4);
							  if ( currentThread->fileHandlers->find( mapped_id ) != currentThread->fileHandlers->end( )
									  && ( mapped_id != 0 || mapped_id != 1 ) )
							  {
								  OpenFile* fileId = currentThread->fileHandlers->at( mapped_id );
								  currentThread->fileHandlers->erase( mapped_id );
								  delete fileId;
							  }
							  else
							  {
								  //TODO: we are trying to close a file handler that doesnt exist?
								  printf( "Error closing file %d\n", mapped_id );
							  }
						  }
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
