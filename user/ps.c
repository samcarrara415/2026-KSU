#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

static struct uproc procs[NPROC];

static char *states[] = {"unused",  "used",    "sleeping",
                         "runnable", "running", "zombie"};

int
main(void)
{
  int n, i;

  n = getprocs(procs, NPROC);
  if (n < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }
  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");
  for (i = 0; i < n; i++) {
    char *st = (procs[i].state >= 0 && procs[i].state < 6)
                   ? states[procs[i].state]
                   : "?";
    printf("%d\t%d\t%s\t%lu\t%s\n", procs[i].pid, procs[i].ppid, st,
           procs[i].sz, procs[i].name);
  }
  exit(0);
}
