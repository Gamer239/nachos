#include "synchconsole.h"

static void ReadAvail() { readAvail->V(); }
static void WriteDone() { writeDone->V(); }

SynchConsole(char *readFile, char *writeFile) {
	Console::Console(readFile, writeFile, ReadAvail, WriteDone, 0);	
}

~SynchConsole() {
	Console::~Console();
}

void PutChar(char ch) {
	Console::PutChar(ch);
	writeDone->P();	
}

char GetChar() {
	readAvail->P();
	Console::GetChar();
}

