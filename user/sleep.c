#include "kernel/types.h"
#include "user/user.h"

// Parse a non-empty all-digit string; saturates instead of overflowing.
static int
parsenum(char *s)
{
  int n = 0;

  if (*s == 0)
    return -1;
  for (; *s; s++) {
    if (*s < '0' || *s > '9')
      return -1;
    if (n > (0x7fffffff - 9) / 10)
      n = 0x7fffffff;
    else
      n = n * 10 + (*s - '0');
  }
  return n;
}

int
main(int argc, char *argv[])
{
  int ticks;

  if (argc != 2 || (ticks = parsenum(argv[1])) < 0) {
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  pause(ticks);
  exit(0);
}
