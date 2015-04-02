#ifdef CHANGED
//TODO: fix the comments in this file
// synchconsole.cc
//	Routines to simulate a serial port to a console device.
//	A console has input (a keyboard) and output (a display).
//	These are each simulated by operations on UNIX files.
//	The simulated device is asynchronous,
//	so we have to invoke the interrupt handler (after a simulated
//	delay), to signal that a byte has arrived and/or that a written
//	byte has departed.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchconsole.h"
#include "system.h"

// Dummy functions because C++ is weird about pointers to member functions
static void ConsoleReadPoll(int c)
{
	SynchConsole *console = (SynchConsole *)c;
	console->CheckCharAvailDone();
}

static void ConsoleWriteDone(int c)
{
	SynchConsole *console = (SynchConsole *)c;
	console->WriteDoneDone();
}

//----------------------------------------------------------------------
// Console::Console
// 	Initialize the simulation of a hardware console device.
//
//	"readFile" -- UNIX file simulating the keyboard (NULL -> use stdin)
//	"writeFile" -- UNIX file simulating the display (NULL -> use stdout)
//----------------------------------------------------------------------

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
	semaphore_put = new Semaphore("synch console_put", 0);
    semaphore_get = new Semaphore("synch console_get", 0);
	lock = new Lock("synch console lock");
	console = new Console(readFile, writeFile, ConsoleReadPoll, ConsoleWriteDone, (int) this);
}

//----------------------------------------------------------------------
// Console::~Console
// 	Clean up console emulation
//----------------------------------------------------------------------

SynchConsole::~SynchConsole()
{
	delete semaphore_put;
    delete semaphore_get;
	delete lock;
	delete console;
}

//----------------------------------------------------------------------
// Console::CheckCharAvail()
// 	Periodically called to check if a character is available for
//	input from the simulated keyboard (eg, has it been typed?).
//
//	Only read it in if there is buffer space for it (if the previous
//	character has been grabbed out of the buffer by the Nachos kernel).
//	Invoke the "read" interrupt handler, once the character has been
//	put into the buffer.
//----------------------------------------------------------------------
void
SynchConsole::CheckCharAvailDone()
{
    //printf("calledcheckavil\n");
	semaphore_get->V();
}

//----------------------------------------------------------------------
// Console::WriteDone()
// 	Internal routine called when it is time to invoke the interrupt
//	handler to tell the Nachos kernel that the output character has
//	completed.
//----------------------------------------------------------------------

void
SynchConsole::WriteDoneDone()
{
    //printf("called write done\n");
	semaphore_put->V();
}

//----------------------------------------------------------------------
// Console::GetChar()
// 	Read a character from the input buffer, if there is any there.
//	Either return the character, or EOF if none buffered.
//----------------------------------------------------------------------

char
SynchConsole::GetChar()
{
	lock->Acquire();
    //printf("lock aquired\n");
	console->CheckCharAvail();
    //printf("char avail\n");
	semaphore_get->P();
    //printf("release\n");
	lock->Release();
	return console->GetChar();
}

//----------------------------------------------------------------------
// Console::PutChar()
// 	Write a character to the simulated display, schedule an interrupt
//	to occur in the future, and return.
//----------------------------------------------------------------------

void
SynchConsole::PutChar(char ch)
{
    //printf("putchar\n");
	lock->Acquire();			// only one disk I/O at a time
    //printf("lock aquired\n");
	console->PutChar(ch);
    //printf("put the char\n");
	semaphore_put->P();			// wait for interrupt
    //printf("release\n");
	lock->Release();
}
#endif
