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

int
main(int argc, char *argv[])
{
	// char * testString = "testing hello\n";
	// Write(testString, 14, ConsoleOutput);
	Write("argc: ", 6, ConsoleOutput);
	Write(itoa(argc), strlen(itoa(argc)), ConsoleOutput);
	Write("\n", 1, ConsoleOutput);
	Write("should be yolo: ", strlen("should be yolo: "), ConsoleOutput); 
	Write(argv[0], strlen(argv[0]), ConsoleOutput);
	Write("\n", 1, ConsoleOutput);
	int i = 0;
	// while (i < 150) {
	//	if (i % 10 == 0) {
	Write("working\n", 8, ConsoleOutput);
	//	}
	//	i++;
	//}
    Exit(99);
    /* not reached */
}
