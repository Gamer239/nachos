#ifdef CHANGED
#include "exception_impl.h"
#include <map>
#include "process.h"
#include "thread.h"
#include "system.h"
#include "exception_utils.h"

void startProcess(int n) {
	currentThread->space->RestoreState();
	currentThread->space->InitRegisters();
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

SpaceId exec(char *filename, int argc, int argv) {
	OpenFile *executable = fileSystem->Open(filename);
	AddrSpace *space;

	std::map<int, Process*>* procMap = Process::GetProcMap();

	printf("Call to exec with argc: %d and argv:\n", argc);

	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %s\n", i, ((char**) argv)[i]);
	}

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
		printf("EXEC[%s]: Parent of %s is %s\n", currentThread->getName(), thread->getName(), parent->GetThread()->getName());
	} else {
		printf("EXEC[%s]: we didn't find ourself in the procmap\n", currentThread->getName());
		parent = new Process(currentThread, parentId);
	}

	child->SetParent(parent);
	parent->AddChild(child);

	procMap->insert(std::pair<int, Process*>(parentId, parent));
	procMap->insert(std::pair<int, Process*>(thread->GetId(), child));
	// printf("inserted pid %d into procmap\n", thread->GetId());
	delete executable;

	// check if the returned address space failed to allocate
	// the number of pages we needed
	if (space->GetFull()) {
		printf("Error: Exec: Insufficient memory to start process\n");
		delete space;
		return -1;
	}

	printf("right before set arguments\n");
	space->SetArguments(argc, (char**) argv, filename);
	printf("right after\n");

	thread->space = space;
	// assign the new space to the thread
	// put the thread on the ready queue?

	thread->Fork(startProcess, 0);

	return child->GetThread()->GetId();

}

int join(SpaceId id) {
	printf("join(spaceid = %d)\n", machine->ReadRegister(4));
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
	fileSystem->Create(buf, 0);
}

#endif
