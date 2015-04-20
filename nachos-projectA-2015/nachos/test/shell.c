#include "syscall.h"
#include "sys_libs.c"

	int
main()
{
	SpaceId newProc;
	OpenFileId input = ConsoleInput;
	OpenFileId output = ConsoleOutput;
	char prompt[2], ch, buffer[60], bytesbuf[10];
	int i, ret;
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
			if (strcomp(buffer, "exit") == 1) {
				Exit(0);
			} else {
				newProc = Exec(buffer, argv);
				Write("PID: ", 5, output);
				Write(itoa(newProc), strlen(itoa(newProc)), output);
				Write("\n", 1, output);
				ret = Join(newProc);
				Write("Process ", 8, output);
				Write(itoa(newProc), strlen(itoa(newProc)), output);
				Write(" returned: ", 11, output);
				Write(itoa(ret), strlen(itoa(ret)), output);
				Write("\n", 1, output);
			}
		}
	}
}
