
#ifndef CHANGED
#define CHANGED
#endif
#ifdef CHANGED
#include "exception_impl.h"
#include <map>
#include "../machine/machine.h"
#include "process.h"
#include "../threads/thread.h"
#include "../threads/system.h"
#include "exception_utils.h"
#include "user_translate.h"
#include "addrspace.h"

extern Machine* machine;	// user program memory and registers
extern FileSystem  *fileSystem;


>>>>>>> 4820491beb9d1e02de3629028d012140c240c35e

void HandleSyscall(int type) {

	int addr, argv, ret, i = 0;
	char c, buf[BUFFER_SIZE], filename[BUFFER_SIZE];
	bzero(buf, BUFFER_SIZE);

	switch (type) {
		case SC_Halt:
			{
				printf("Shutdown, initiated by user program.\n");
				// currentThread->space->PrintPages();
				interrupt->Halt();
				break;
			}
		case SC_Exit:
			{
				DEBUG('s', "Call to Syscall Exit (SC_Exit).\n");
				exit(machine->ReadRegister(4));
				break;
			}
		case SC_Exec:
			{
				addr = machine->ReadRegister(4);
				ReadString(addr, filename);
				DEBUG('s', "Call to Syscall Exec (SC_Exec).\n");
				argv = machine->ReadRegister(5);
				ret = exec(filename, argv);
				machine->WriteRegister(2, ret);
				break;
			}

		case SC_Join:
			{
				DEBUG('s', "Call to Syscall Join (SC_Join) from %s.\n", currentThread->getName());
				int pid = machine->ReadRegister(4);
				ret = (pid > 0) ? join(pid) : -1;
				machine->WriteRegister(2, ret);
				break;
			}

		case SC_Create:
			{
				DEBUG('s', "Call to Syscall Create (SC_Create).\n");
				create((char *)machine->ReadRegister(4));
				// no return value for Create
				break;
			}

		case SC_Open:
			{
				DEBUG('s', "Call to Syscall Open (SC_Open).\n");
				addr =  machine->ReadRegister(4); // char* filename arg, we need to read this buf
				c = '1';
				i = 0;
				while (c != '\0' && i < BUFFER_SIZE) {
					UserTranslate::ReadMem(addr + i, 1, (int *) &c);
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
				machine->WriteRegister(2, mapped_id);
				break;
			}

		case SC_Read:
			{
				addr = machine->ReadRegister(4);
				int size = machine->ReadRegister(5);
				int mapped_id = machine->ReadRegister(6);
				ret = read(addr, size, mapped_id);
				machine->WriteRegister(2, ret);
				break;
			}

		case SC_Write:
			{
				DEBUG('s', "Call to Syscall Write (SC_Write).\n");
				addr = machine->ReadRegister(4);
				int size = (int) machine->ReadRegister(5);
				const int mapped_id = (int) machine->ReadRegister(6);

				int wrote = 0;
				int value;
				char buff;
				bool read;
				int write_count = 1;

				if (currentThread->fileHandlers->find(mapped_id) != currentThread->fileHandlers->end()) {
					OpenFile* fileId = currentThread->fileHandlers->at( mapped_id );
					while(wrote < size && buff != EOF && write_count > 0) {
						read = UserTranslate::ReadMem((int) (addr + wrote), 1, &value);
						buff = value;
						if (mapped_id == ConsoleOutput) {
							WriteFile(ConsoleOutput, &buff, 1);
						} else if (mapped_id != ConsoleInput){
							write_count = fileId->Write(&buff, 1);
						} else {
							break;
						}
						wrote++;
					}
				} else {
					wrote = -1;
				}

				// ret = write(addr, size, mapped_id);

				machine->WriteRegister(2, wrote);

				break;
			}

		case SC_Close:
			{
				DEBUG('s', "Call to Syscall Close (SC_Close).\n");
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
					//ignore the close if it doesnt exist
				}
				break;
			}

		case SC_Fork:
			DEBUG('s', "Call to Syscall Fork (SC_Fork).\n");
			break;

		case SC_Yield:
			DEBUG('s', "Call to Syscall Yield (SC_Yield).\n");
			break;

		case SC_Checkpoint:
			DEBUG('s', "Call to Syscall Checkpoint (SC_Checkpoint).\n");
			{
				//read the address that contains the string
				addr = machine->ReadRegister(4);

				//close the file
				delete fileId;
				//fetch the string
				ReadString(addr, filename);

				//run the checkpoint code
				int ret = checkpoint( filename );

				//write the return
				machine->WriteRegister(2, ret);
			}
			break;

		default:
			printf("Unexpected Syscall Code: %d\n", type);
			ASSERT(FALSE);
			break;
	}

}

void startProcess(int n) {
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	currentThread->space->LoadArguments();
	DEBUG('s', "[IN startProcess]: currentThread is: %s\n", currentThread->getName());
	machine->Run();
	ASSERT(false);
}

void exit(int ret) {

	std::map<int, Process*>* procMap = Process::GetProcMap();
	Process* currentProcess;
	Process* parent;
	Thread* parentThread;

	if (procMap->find(currentThread->GetId()) != procMap->end()) {
		currentProcess = procMap->at(currentThread->GetId());
	} else {
		ASSERT(false); // should never happen
	}

	parent = currentProcess->GetParent();
	parentThread = parent->GetThread();

	// if I have parent
	if (parent != NULL && currentThread->GetStatus() != ZOMBIE) {
		currentProcess->SetReturnValue(ret);
		if (parentThread->GetStatus() == JOINING) {
			parentThread->joinSem->V();
		}
	} else {
		// we know we have no parent, so we can remove our map entry
		procMap->erase(currentThread->GetId());
	}

	if (!currentProcess->GetChildren()->IsEmpty()) {
		currentProcess->GetChildren()->Mapcar(Process::SetZombie);
	}

	currentProcess->SetFinished(true);

	delete currentThread->space;
	currentThread->space = NULL;
	DEBUG('s', "SC_EXIT: currentThread is: %s\n", currentThread->getName());
	currentThread->Finish();

}

SpaceId exec(char *filename, int argv) {
	
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	std::map<int, Process*>* procMap = Process::GetProcMap();

	//printf("argv[%d]: %s (%d)\n", 0, ((char**) argv)[0], ((int)((char **)argv)[0]));

	if (executable == NULL) {
		printf("SC_Exec Error: Unable to open file %s\n", filename);
		return -1;
	}

	Thread *thread = new Thread(filename, 0);
	space = new AddrSpace(executable);
	int parentId = currentThread->GetId();

	Process *child = new Process(thread, thread->GetId());
	Process *parent;

	// in case our parent thread somehow was not put into the procmap
	if (procMap->find(parentId) != procMap->end()) {
		parent = procMap->at(parentId);
	} else {
		parent = new Process(currentThread, parentId);
	}

	child->SetParent(parent);
	parent->AddChild(child);

	procMap->insert(std::pair<int, Process*>(parentId, parent));
	procMap->insert(std::pair<int, Process*>(thread->GetId(), child));
	// printf("inserted pid %d into procmap\n", thread->GetId());
	// delete executable;

	// check if the returned address space failed to allocate
	// the number of pages we needed
	if (space->GetFull()) {
		printf("Error: Exec: Insufficient memory to start process\n");
		delete space;
		return -1;
	}

	space->SetArguments(argv, filename);

	thread->space = space;
	// assign the new space to the thread
	// put the thread on the ready queue?

	thread->Fork(startProcess, 0);

	return child->GetThread()->GetId();

}

int join(SpaceId id) {
	int pid = machine->ReadRegister(4);
	int retVal;

	Process* target;
	Thread* targetParent;
	std::map<int, Process*>* procMap = Process::GetProcMap();

	if (procMap->find(pid) != procMap->end()) {
		target = procMap->at(pid);
		targetParent = target->GetParent()->GetThread();
		if (targetParent->GetId() == currentThread->GetId()) {
			if (!target->GetFinished()) {
				currentThread->setStatus(JOINING);
				currentThread->WaitOnReturn();
			}
			retVal = target->GetReturnValue();
			procMap->erase(pid);
		} else {
			retVal = -1;
		}
	} else {
		retVal = -1;
	}
	return retVal;
}

void create(char* filename) {
	char buf[128];
	ReadString((int) filename, buf);
	// a file size of zero is fine for now
	// we'll add data once we open it
	printf("gonna create %s\n", buf);
	fileSystem->Create(buf, 0);
}



int read(int addr, int size, OpenFileId mapped_id) {
	int read = 0;
	char buff;
	bool write;
	int read_count = 1;

	if (currentThread->fileHandlers->find(mapped_id) != currentThread->fileHandlers->end()) {
		// its a valid file for us to use
		OpenFile* fileId = currentThread->fileHandlers->at(mapped_id);
			// printf("in the else\n");
			while (read < size && read_count > 0) {
				if (mapped_id == ConsoleInput) {
					Read(ConsoleInput, &buff, 1);
				} else if (mapped_id != ConsoleOutput) {
					read_count = fileId->Read(&buff, 1);
				} else {
					break;
				}
				write = UserTranslate::WriteMem(addr + read, 1, (int) buff);
				read++;
			}
	} else {
		read = -1;
	}

	return read;

}

/*
   int write(int addr, int size, OpenFileId mapped_id) {
   int wrote = 0;
   int value;
   char buff;
   bool read;
   int write_count = 1;

   if (currentThread->fileHandlers->find(mapped_id) != currentThread->fileHandlers->end()) {
   OpenFile* fileId = currentThread->fileHandlers->at( mapped_id );
   while(wrote < size && buff != EOF && write_count > 0) {
   read = UserTranslate::ReadMem((int) (addr + wrote), 1, &value);
   buff = value;
   if (mapped_id == ConsoleOutput) {
   WriteFile(ConsoleOutput, &buff, 1);
   } else if (mapped_id != ConsoleInput){
   write_count = fileId->Write(&buff, 1);
   } else {
   break;
   }
   wrote++;
   }
   } else {
   wrote = -1;
   }
   return wrote;
   }
   */
int checkpoint(char* filename)
{
	bool res = true;
	bool exists = true;
	OpenFile* fileId = fileSystem->Open(filename);

	if (strlen(filename) <= 0)
	{
		printf("filename <= 0 %s\n", filename);
		return -1;
	}

	if ( fileId == NULL )
	{
		//the file doesn't exist create the checkpoint
		printf("the file doesn't exist\n");
		fileSystem->Create(filename, 0);
		OpenFile* fileId = fileSystem->Open(filename);
		res = currentThread->writeThreadContents( fileId );
		exists = false;
	}
	else
	{
		//the file exists, load the snapshot
		printf("the file exists\n");

		//TODO: when we read on old checkpoint do we need to update the PC since we need to move past this instruction?
		res = currentThread->readThreadContents( fileId );
	}

	//close the file
	if ( fileId != NULL )
	{
		delete fileId;
	}

	if ( res == false )
	{
		return -1;
	}
	else if ( exists == true )
	{
		return 1;
	}



	return 0;
}

#endif
