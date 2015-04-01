#ifndef EXCEPTION_IMPL_H
#define EXCEPTION_IMPL_H

#include "syscall.h"

void exit(int ret);
SpaceId exec(char* filename, int argc, int argv);
int join(SpaceId id);
void create(char* filename);

#endif
