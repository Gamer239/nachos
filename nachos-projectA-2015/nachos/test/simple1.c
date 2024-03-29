/* argtest.c
 *
 * Parent in simple argument test.
 *
 */

#include "syscall.h"

int
main()
{

  SpaceId kid;
  int joinval;
  char *args[3];

  args[0] = "argument1";
  args[1] = "argument2";
  args[2] = (char *)0;

  kid = Exec("test/simple2",args);
  joinval = Join(kid);

  Halt();
  /* not reached */
}

/* Print a null-terminated string "s" on open file
   descriptor "file". */

prints(s,file)
char *s;
OpenFileId file;

{
  while (*s != '\0') {
    Write(s,1,file);
    s++;
  }
}

/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i;
  char c;

  if (n < 0) {
    Write("-",1,file);
    n = -n;
  }
  if ((i = n/10) != 0)
    printd(i,file);
  c = (char) (n % 10) + '0';
  Write(&c,1,file);
}
