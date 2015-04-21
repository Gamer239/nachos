/* maxfork.c
 *
 * Parent in big net of processes ... stress test
 *
 */

#define NUMKIDS 15

#include "syscall.h"

int
main()
{

  SpaceId kid[NUMKIDS];
  int i, joinval;
  char *args[2];

  print("PARENT exists\n");

  args[0] = "kid";
  args[1] = (char *)0;

  for (i=0; i<NUMKIDS; i++) {
    kid[i] = Exec("kid", args);
    if ( kid[i] < 0) {
      print("Exec failed at i=");
      printd(i, ConsoleOutput);
      print("\n");
    }
    else {
      print("Kid ");
      printd(i, ConsoleOutput);
      print(" created.\n");
    }
  }

  print("PARENT about to Join kids\n");

  for (i=0; i<NUMKIDS; i++) {
    joinval = Join(kid[i]);
    print("PARENT off Join with value of ");
    printd(joinval, ConsoleOutput);
    print("\n");
  }

  Exit(0);

  /* not reached */
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

/* Print a null-terminated string "s" on ConsoleOutput. */

print(s)
char *s;

{
  prints(s, ConsoleOutput);
}

