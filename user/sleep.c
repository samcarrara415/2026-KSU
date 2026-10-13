#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  // Need exactly one argument.
  if (argc != 2) {
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }

  // Every character must be a digit (this also rejects "-1" and "").
  if (argv[1][0] == 0) {
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  for (i = 0; argv[1][i] != 0; i++) {
    if (argv[1][i] < '0' || argv[1][i] > '9') {
      fprintf(2, "usage: sleep <ticks>\n");
      exit(1);
    }
  }

  pause(atoi(argv[1]));
  exit(0);
}
