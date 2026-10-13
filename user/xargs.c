#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXWORD 1024
// exec() in this kernel accepts at most MAXARG-1 arguments plus the NULL.
#define LIMIT (MAXARG - 1)

static char *cmdv[MAXARG + 1];
static int base; // number of leading words from the command line
static int nargs;
static int status;

static void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

static void
run(void)
{
  int pid, st, i;

  cmdv[base + nargs] = 0;
  pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    status = 1;
  } else if (pid == 0) {
    exec(cmdv[0], cmdv);
    fprintf(2, "xargs: exec %s failed\n", cmdv[0]);
    exit(1);
  } else {
    if (wait(&st) < 0 || st != 0)
      status = 1;
  }
  for (i = 0; i < nargs; i++)
    free(cmdv[base + i]);
  nargs = 0;
}

int
main(int argc, char *argv[])
{
  int i = 1, perrun = 0, usen = 0, len = 0, c, n, inword = 0;
  char buf[512], word[MAXWORD];
  int got, pos;

  if (argc > 1 && strcmp(argv[1], "-n") == 0) {
    char *s;
    if (argc < 3)
      usage();
    for (s = argv[2]; *s; s++) {
      if (*s < '0' || *s > '9')
        usage();
    }
    if (*argv[2] == 0)
      usage();
    perrun = atoi(argv[2]);
    if (perrun < 1)
      usage();
    i = 3;
    usen = 1;
  }
  if (i >= argc)
    usage();
  for (; i < argc; i++)
    cmdv[base++] = argv[i];
  if (base > LIMIT) {
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }
  n = LIMIT - base; // max extra words per run
  if (n < 1) {
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }
  if (perrun == 0 || perrun > n)
    perrun = n;
  // Without -n a run covers one line (split only if the line is too long).

  int lineflag = !usen;

  while ((got = read(0, buf, sizeof(buf))) > 0) {
    for (pos = 0; pos < got; pos++) {
      c = buf[pos];
      if (c == ' ' || c == '\t' || c == '\n') {
        if (inword) {
          word[len] = 0;
          cmdv[base + nargs] = malloc(len + 1);
          strcpy(cmdv[base + nargs], word);
          nargs++;
          len = 0;
          inword = 0;
          if (nargs == perrun)
            run();
        }
        if (c == '\n' && lineflag && nargs > 0)
          run();
      } else {
        inword = 1;
        if (len < MAXWORD - 1)
          word[len++] = c;
      }
    }
  }
  if (inword) {
    word[len] = 0;
    cmdv[base + nargs] = malloc(len + 1);
    strcpy(cmdv[base + nargs], word);
    nargs++;
  }
  if (nargs > 0)
    run();
  exit(status);
}
