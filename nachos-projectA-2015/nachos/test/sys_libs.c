#define CHANGED true
#ifdef CHANGED
// system.h
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

//#ifndef SYSLIBS_H
//#define SYSLIBS_H

int strcomp(char* str1, char* str2) {

	int i;
	for (i = 0; i < 16; i++) {
		if (str1[i] != str2[i]) {
			return 0;
		}
		if (str1[i] == '\0' || str2[i] == '\0') break;
	}
	return 1;

}


int strlen( char* str )
{
	int i = 0;
	while( str[i] != '\0' )
	{
		i++;
	}
	return i;
}


//#endif // SYSTEM_H
#endif
