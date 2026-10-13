#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

static struct uproc procs[NPROC];

int
main(void)
{
  int i, j, n, pid, bad = 0;

  for (i = 0; i < 20; i++) {
    pid = fork();
    if (pid == 0) {
      for (j = 0; j < 30; j++) {
        int c = fork();
        if (c == 0)
          exit(0);
        if (c > 0)
          wait(0);
      }
      exit(0);
    }
  }
  for (i = 0; i < 300; i++) {
    n = getprocs(procs, NPROC);
    if (n < 1)
      bad++;
    for (j = 0; j < n; j++)
      if (procs[j].pid <= 0 || procs[j].state < 1 || procs[j].state > 5)
        bad++;
    if (getprocs(procs, 2) != 2)
      bad++;
    if (getprocs(procs, 0) != 0)
      bad++;
    if (getprocs(procs, -1) != -1)
      bad++;
    if (getprocs((struct uproc *)0x7fffffff0000, 3) != -1)
      bad++;
  }
  while (wait(0) > 0)
    ;
  printf(bad ? "psstress: FAILED %d\n" : "psstress: OK\n", bad);
  exit(bad != 0);
}
