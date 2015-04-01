#include "syscall.h"
#include "sys_libs.c"

int main() {

	SpaceId newProc[6];
	OpenFileId input = ConsoleInput;
	OpenFileId output = ConsoleOutput;

	char argv[1];
	argv[0] = "\0";

	int i;
	for (i = 1; i < 6; i++) {
		switch (i) {

			case 1:
				newProc[1] = Exec("test/simple1", argv);
				break;

			case 2:
				newProc[2] = Exec("test/simple2", argv);
				break;

			case 3:
				newProc[3] = Exec("test/simple3", argv);
				break;

			case 4:
				newProc[4] = Exec("test/simple4", argv);
				break;

			case 5:
				newProc[5] = Exec("test/simple5", argv);
				break;

		}
	}

	int retSum = 0;

	for (i = 1; i < 6; i++) {
		retSum += Join(newProc[i]);
	}

	Write("Got total of: ", 14, output);
	Write(itoa(retSum), strlen(itoa(retSum)), output);
	Write("\n", 1, output);

	Exit(0);	

}
