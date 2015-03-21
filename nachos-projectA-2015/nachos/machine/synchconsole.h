#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"

class SynchConsole {
	public:
		SynchConsole(char *readFile, char *writeFile, VoidFunctionPtr readAvail, 
				VoidFunctionPtr writeDone, int callArg);
		// initialize the hardware console device
		~SynchConsole();			// clean up console emulation

		// external interface -- Nachos kernel code can call these
		void PutChar(char ch);	// Write "ch" to the console display, 
		// and return immediately.  "writeHandler" 
		// is called when the I/O completes. 

		char GetChar();	   	// Poll the console input.  If a char is 
		// available, return it.  Otherwise, return EOF.
		// "readHandler" is called whenever there is 
		// a char to be gotten

		// internal emulation routines -- DO NOT call these. 
		void WriteDone();	 	// internal routines to signal I/O completion
		void CheckCharAvail();

	private:
		int readFileNo;			// UNIX file emulating the keyboard 
		int writeFileNo;			// UNIX file emulating the display
		VoidFunctionPtr writeHandler; 	// Interrupt handler to call when 
		// the PutChar I/O completes
		VoidFunctionPtr readHandler; 	// Interrupt handler to call when 
		// a character arrives from the keyboard
		int handlerArg;			// argument to be passed to the 
		// interrupt handlers
		bool putBusy;    			// Is a PutChar operation in progress?
		// If so, you can't do another one!
		char incoming;    			// Contains the character to be read,
		// if there is one available. 
		// Otherwise contains EOF.
		Semaphore readAvail;
		Semaphore writeDone;
};


