#ifdef CHANGED
#include "system.h"
#include "user_translate.h"
#include "exception_utils.h"
#include "machine.h"

Process* getCurrentProcess() {
	std::map<int, Process*>* procMap = Process::GetProcMap();
	Process* currentProcess;
	if (procMap->find(currentThread->GetId()) != procMap->end()) {
		currentProcess = procMap->at(currentThread->GetId());
	} else {
		ASSERT(false);
	}
	return currentProcess;	
}

void ReadString(int addr, char* buf) {
	char c;
	buf[0] = '\0';
	int i = 0;
	do {
		UserTranslate::ReadMem(addr++, 1, (int*) &c);
		sprintf(buf + strlen(buf), "%c", c);
		i++;
	} while (c != '\0' && i < 64);
}

void WriteString(int addr, char* buf, int size) {
	int i = 0;
	do {
		UserTranslate::WriteMem(addr + i, 1, (int) buf);
	} while (i++ < size);
}


#endif
