#include "syscall.h"

	int
main()
{
	SpaceId newProc;
	OpenFileId input = ConsoleInput;
	OpenFileId output = ConsoleOutput;
	char prompt[2], ch, buffer[60], bytesbuf[10];
	int i;
	int bytes;
	char *argv[1];

	argv[0] = "\0";

	prompt[0] = '-';
	prompt[1] = '-';

	while(1) {
		Write(prompt, 2, output);
		i = 0;
		do {
			bytes = Read(&buffer[i], 1, input); 
		} while( buffer[i++] != '\n' );

		buffer[--i] = '\0';
		Write(bytesbuf, 2, output);
		if(i > 0) {
			newProc = Exec(buffer, argv);
			Write("Got PID: ", 9, output);
		    Write((char)(((int) '0') + newProc), 1, output);
			Write("\n", 1, output);	
			Join(newProc);
		}
	}
}

