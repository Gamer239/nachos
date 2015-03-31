#include "process.h"

std::map<int, Process*>* Process::procmap = NULL;

Process::Process(Thread* aThread, int aid) {
	printf("Created new process %s(%d)\n", aThread->getName(), aid);
	children = new List;
	thread = aThread;
	id = aid;
	GetProcMap()->insert(std::pair<int, Process*>(id, this));
}

Process::~Process() {
	delete children;
}

void Process::SetParent(Process* tParent) {
	parent = tParent;
}

Process * Process::GetParent() {
	return parent;
}

void Process::AddChild(Process* child) {
	children->Append(child);
}

List * Process::GetChildren() {
	return children;
}

std::map<int, Process*>* Process::GetProcMap() {
	if (procmap == NULL) {
		procmap = new std::map<int, Process*>;
	}
	return procmap;
}

Thread * Process::GetThread() {
	return thread;
}

void Process::SetZombie(int arg) {
	((Thread *) arg)->setStatus(ZOMBIE);
}

void Process::SetReturnValue(int val) {
	returnVal = val;
}
