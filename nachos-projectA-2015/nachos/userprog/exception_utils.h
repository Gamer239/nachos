#ifdef CHANGED
#ifndef EXCEPTION_UTILS_H
#define EXCEPTION_UTILS_H

#include "process.h"
class Process;

void ReadString(int addr, char* buf);
void WriteString(int addr, char* buf, int size);
Process* getCurrentProcess();
void intToChar(int value, char* buf);

#endif
#endif
