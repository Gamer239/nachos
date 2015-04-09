#include "syscall.h"
#include "sys_libs.c"

int
main()
{
	// just do some stuff	
	int i, j, k;
	i = 10;
	j = 20;
	k = i * j;
	for (i = 0; i < k; i++) {
		j += i;
	}
	j = k / i;
	i += 5;
	j -= k;
	j++;
	k -= j;
	i *= 4;
	j += i;	
	Exit(99);
}
