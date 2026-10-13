#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int mask = 0;
  char *s;

  if (argc < 3 || argv[1][0] == 0) {
    fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
    exit(1);
  }
  for (s = argv[1]; *s; s++) {
    if (*s < '0' || *s > '9' || mask > (0x7fffffff - 9) / 10) {
      fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
      exit(1);
    }
    mask = mask * 10 + (*s - '0');
  }
  if (trace(mask) < 0) {
    fprintf(2, "trace: trace failed\n");
    exit(1);
  }
  exec(argv[2], argv + 2);
  fprintf(2, "trace: exec %s failed\n", argv[2]);
  exit(1);
}
