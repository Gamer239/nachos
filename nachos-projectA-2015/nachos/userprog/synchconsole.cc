#ifdef CHANGED
// synchconsole.cc
//	Routines to simulate a synchronous serial port to a console device.
//	A console has input (a keyboard) and output (a display).
//	These are each simulated by operations on UNIX files.
//	The simulated device is synchronous,
//	so we have to invoke the interrupt handler (after a simulated
//	delay), to signal that a byte has arrived and/or that a written
//	byte has departed.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchconsole.h"
#include "system.h"

// These functions will be called when the interrupt finishes
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
// SynchConsole::SynchConsole
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
	DEBUG('c', "Created SynchConsole\n");
}

//----------------------------------------------------------------------
// SynchConsole::~SynchConsole
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
// SynchConsole::CheckCharAvail()
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
	DEBUG('c', "CheckCharAvailDone before semaphore_get->V()\n");
	semaphore_get->V();
	DEBUG('c', "CheckCharAvailDone after semaphore_get->V()\n");

}

//----------------------------------------------------------------------
// SynchConsole::WriteDone()
// 	Internal routine called when it is time to invoke the interrupt
//	handler to tell the Nachos kernel that the output character has
//	completed.
//----------------------------------------------------------------------

void
SynchConsole::WriteDoneDone()
{
	DEBUG('c', "WriteDoneDone before semaphore_put->V()\n");
	semaphore_put->V();
	DEBUG('c', "WriteDoneDone after semaphore_put->V()\n");
}

//----------------------------------------------------------------------
// SynchConsole::GetChar()
// 	Read a character from the input buffer, if there is any there.
//	Either return the character, or EOF if none buffered.
//----------------------------------------------------------------------

char
SynchConsole::GetChar() {

	DEBUG('c', "GetChar\n");
	char ch;

	lock->Acquire();
	semaphore_get->P();
	console->CheckCharAvail();
	ch = console->GetChar();
	lock->Release();
	return ch;
}

//----------------------------------------------------------------------
// SynchConsole::PutChar()
// 	Write a character to the simulated display, schedule an interrupt
//	to occur in the future, and return.
//----------------------------------------------------------------------

void
SynchConsole::PutChar(char ch)
{
	DEBUG('c', "PutChar\n");
	lock->Acquire();			// only one disk I/O at a time
	console->PutChar(ch);
	semaphore_put->P();			// wait for interrupt
	lock->Release();
}
#endif
