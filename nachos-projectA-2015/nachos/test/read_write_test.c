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
//#include <string.h>

#define BUFFER_SIZE 16

int
main()
{
	/*
	* This is a basic read write test to a file.
	*
	*/
	char buffer[BUFFER_SIZE];
	char* filename = "my_file_name";
	char * testString = "testing hello\n";
	Create( filename );
	OpenFileId fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Write(testString, 15, fileId );
	Close( fileId );
	fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Read( buffer, 15, fileId );
	Write( buffer, 15, ConsoleOutput );
	Close( fileId );

	//mess with the write buffer size
	Create( filename );
	fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Write(testString, 15+10, fileId );
	Close( fileId );
	fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Read( buffer, 15, fileId );
	Write( buffer, 15, ConsoleOutput );
	Close( fileId );

	//mess with the read buffer size
	Create( filename );
	fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Write(testString, 15, fileId );
	Close( fileId );
	fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Read( buffer, 15+10, fileId );
	Write( buffer, 15, ConsoleOutput );
	Close( fileId );

	//TODO: provide to small of a read buffer
	Create( filename );
	fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Write(testString, 15, fileId );
	Close( fileId );
	fileId = Open( filename );
	if ( fileId < 2 )
	{
		Write( "open failed\n", 13, ConsoleOutput );
	}
	Read( buffer, 15+10, fileId );
	Write( buffer, 15, ConsoleOutput );
	Close( fileId );

  Exit(0);
}
