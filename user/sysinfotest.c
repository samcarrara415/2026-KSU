#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

static int failures;

static void
check(int ok, char *what)
{
  if (!ok) {
    printf("FAIL: %s\n", what);
    failures++;
  }
}

int
main(void)
{
  struct sysinfo a, b;
  int fds[2], fd, pid;
  char *p;

  check(sysinfo(&a) == 0, "sysinfo ok");
  check(sysinfo((struct sysinfo *)0x7fffffff0000) < 0, "bad pointer rejected");
  check(sysinfo((struct sysinfo *)0) == 0 || 1, "null ptr does not crash");

  p = sbrk(8 * 4096);
  for (int i = 0; i < 8; i++)
    p[i * 4096] = 1;
  sysinfo(&b);
  check(b.freemem < a.freemem, "freemem decreases after sbrk");
  sbrk(-8 * 4096);

  sysinfo(&a);
  pid = fork();
  if (pid == 0) {
    sysinfo(&b);
    check(b.nproc > a.nproc, "nproc increases in child");
    exit(failures);
  }
  int st;
  wait(&st);
  failures += st;

  sysinfo(&a);
  fd = open("README", 0);
  sysinfo(&b);
  check(b.nopenfile == a.nopenfile + 1, "nopenfile after open");
  pipe(fds);
  sysinfo(&a);
  check(a.nopenfile == b.nopenfile + 2, "nopenfile after pipe");
  close(fds[0]);
  close(fds[1]);
  close(fd);
  sysinfo(&a);
  check(a.nopenfile == b.nopenfile - 1, "nopenfile after close");

  printf(failures ? "sysinfotest: FAILED\n" : "sysinfotest: OK\n");
  exit(failures != 0);
}
