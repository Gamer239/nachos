/* halt.c
 *	Simple program to test whether running a user program works.
 *
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"
#include "sys_libs.c"
//#include <string.h>

#define BUFFER_SIZE 256

int
main()
{
	/*
	* This is a basic checkpoint test to a file.
	*
	*/
	char* text = "test_file";
	char buffer[256];
	int res = 0;
	CheckPoint(text);
	char* num = itoa(res);
	Write( num, strlen(num), ConsoleOutput );
	//CheckPoint(text);

  Exit(0);
}
