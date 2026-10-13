#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

// exec can take at most MAXARG-1 arguments in this kernel.
#define MAXWORDS (MAXARG - 1)

char *cmd[MAXARG + 1];     // the command and its arguments
int ncmd;                  // how many words came from xargs' own arguments
char words[MAXARG][256];   // words read from stdin
int status;                // 0 if every run succeeded, else 1

void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

// Run the command with the first nwords words appended, and wait for it.
void
run(int nwords)
{
  int i, pid, st;

  for (i = 0; i < nwords; i++)
    cmd[ncmd + i] = words[i];
  cmd[ncmd + nwords] = 0;

  pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    status = 1;
    return;
  }
  if (pid == 0) {
    exec(cmd[0], cmd);
    fprintf(2, "xargs: exec %s failed\n", cmd[0]);
    exit(1);
  }
  wait(&st);
  if (st != 0)
    status = 1;
}

int
main(int argc, char *argv[])
{
  int i, n, nwords, len;
  int use_n = 0;  // 1 if -n was given
  char c;

  // Parse "-n N".
  i = 1;
  n = MAXWORDS;
  if (argc > 1 && strcmp(argv[1], "-n") == 0) {
    if (argc < 3)
      usage();
    for (i = 0; argv[2][i] != 0; i++) {
      if (argv[2][i] < '0' || argv[2][i] > '9')
        usage();
    }
    if (argv[2][0] == 0 || atoi(argv[2]) < 1)
      usage();
    use_n = 1;
    n = atoi(argv[2]);
    i = 3;
  }

  // The rest of the arguments are the command.
  if (i >= argc)
    usage();
  for (; i < argc; i++) {
    cmd[ncmd] = argv[i];
    ncmd++;
  }

  // Never pass more arguments than exec allows.
  if (n > MAXWORDS - ncmd)
    n = MAXWORDS - ncmd;
  if (n < 1) {
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }

  // Read stdin one character at a time and split it into words.
  nwords = 0;
  len = 0;  // length of the word being built (0 = not in a word)
  while (1) {
    int got = read(0, &c, 1);

    if (got == 1 && c != ' ' && c != '\t' && c != '\n') {
      // Part of a word.
      if (len < 255) {
        words[nwords][len] = c;
        len++;
      }
      continue;
    }

    // A space, tab, newline or end of input: finish the current word.
    if (len > 0) {
      words[nwords][len] = 0;
      nwords++;
      len = 0;
      if (nwords == n) {
        run(nwords);
        nwords = 0;
      }
    }

    // Without -n, the end of a line means "run now".
    if ((got == 0 || c == '\n') && !use_n && nwords > 0) {
      run(nwords);
      nwords = 0;
    }

    if (got <= 0)
      break;
  }

  // Leftover words (the last, shorter run).
  if (nwords > 0)
    run(nwords);

  exit(status);
}
