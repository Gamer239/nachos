#ifdef CHANGED
#ifndef EXCEPTION_IMPL_H
#define EXCEPTION_IMPL_H

#include "syscall.h"

#define BUFFER_SIZE 256

void exit(int ret);
SpaceId exec(char* filename, int argv);
int join(SpaceId id);
void create(char* filename);
void HandleSyscall(int type);
int read(int addr, int size, OpenFileId mapped_id);
int write(int addr, int size, OpenFileId mapped_id);
int checkpoint(char* filename);

#endif
#endif
