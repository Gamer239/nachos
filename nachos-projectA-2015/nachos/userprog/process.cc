#ifdef CHANGED
#include "process.h"

std::map<int, Process*>* Process::procmap = NULL;
//TODO: we should probably have some overarching comment at least to give some context
Process::Process(Thread* aThread, int aid) {
	// printf("Created new process %s(%d)\n", aThread->getName(), aid);
	children = new List;
	thread = aThread;
	id = aid;
	GetProcMap()->insert(std::pair<int, Process*>(id, this));
	finished = false;
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
	((Process *) arg)->GetThread()->setStatus(ZOMBIE);
	((Process *) arg)->SetParent(NULL);
}

void Process::SetReturnValue(int val) {
	returnVal = val;
}

int Process::GetReturnValue() {
	return returnVal;
}

bool Process::GetFinished() {
	return finished;
}

void Process::SetFinished(bool f) {
	finished = f;
}
#endif
