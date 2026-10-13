#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  // Need a mask and a command.
  if (argc < 3 || argv[1][0] == 0) {
    fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
    exit(1);
  }

  // The mask must be all digits.
  for (i = 0; argv[1][i] != 0; i++) {
    if (argv[1][i] < '0' || argv[1][i] > '9') {
      fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
      exit(1);
    }
  }

  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "trace: trace failed\n");
    exit(1);
  }

  // exec keeps the trace mask, so the command is traced.
  exec(argv[2], argv + 2);
  fprintf(2, "trace: exec %s failed\n", argv[2]);
  exit(1);
}
