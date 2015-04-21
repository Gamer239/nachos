// progtest.cc
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "addrspace.h"
#ifdef CHANGED
#include "synch.h"
#include "syscall.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
SpaceId exec(char* filename, int argv);

void
StartProcess(char *filename)
{
	exec(filename, 0);
	#endif
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.
#ifdef CHANGED
static SynchConsole *console;
#endif
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------
#ifdef CHANGED
void
ConsoleTest (char *in, char *out)
{
    char ch;
    console = new SynchConsole(in, out);
    for (;;) {
		ch = console->GetChar();
		console->PutChar(ch);	// echo it!
		if (ch == 'q') return;  // if q, quit
    }
}
#endif
