#ifdef CHANGED
//TODO: fix comments in this file
// machine.h
//	Data structures for simulating the execution of user programs
//	running on top of Nachos.
//
//	User programs are loaded into "mainMemory"; to Nachos,
//	this looks just like an array of bytes.  Of course, the Nachos
//	kernel is in memory too -- but as in most machines these days,
//	the kernel is loaded into a separate memory region from user
//	programs, and accesses to kernel memory are not translated or paged.
//
//	In Nachos, user programs are executed one instruction at a time,
//	by the simulator.  Each memory reference is translated, checked
//	for errors, etc.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef USERTRANSLATE_H
#define USERTRANSLATE_H

#include "copyright.h"
#include "utility.h"
#include "translate.h"

// The following class defines the simulated host workstation hardware, as
// seen by user programs -- the CPU registers, main memory, etc.
// User programs shouldn't be able to tell that they are running on our
// simulator or on the real hardware, except
//	we don't support floating point instructions
//	the system call interface to Nachos is not the same as UNIX
//	  (10 system calls in Nachos vs. 200 in UNIX!)
// If we were to implement more of the UNIX system calls, we ought to be
// able to run Nachos on top of Nachos!
//
// The procedures in this class are defined in machine.cc, mipssim.cc, and
// translate.cc.

class UserTranslate {
  public:

    static bool ReadMem(int addr, int size, int* value);
    static bool WriteMem(int addr, int size, int value);
    				// Read or write 1, 2, or 4 bytes of virtual
				// memory (at addr).  Return FALSE if a
				// correct translation couldn't be found.

    static ExceptionType Translate(int virtAddr, int* physAddr, int size,bool writing);
    				// Translate an address, and check for
				// alignment.  Set the use and dirty bits in
				// the translation entry appropriately,
    				// and return an exception code if the
				// translation couldn't be completed.
};

#endif // MACHINE_H
#endif
