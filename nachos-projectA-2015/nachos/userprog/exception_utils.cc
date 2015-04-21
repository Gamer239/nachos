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
void intToChar(int value, char* buf)
{
	buf[0] = ( value & 0xFF000000 ) >> 24;
	buf[1] = ( value & 0x00FF0000 ) >> 16;
	buf[2] = ( value & 0x0000FF00 ) >> 8;
	buf[3] = ( value & 0x000000FF );
}

int charToInt( char* buf )
{
	int result = 0;
	int num0 = buf[0];
	int num1 = buf[1];
	int num2 = buf[2];
	int num3 = buf[3];

	result = ( num0 << 24 ) | ( num1 << 16 ) | ( num2 << 8 ) | num3;

	return result;
}

#endif
