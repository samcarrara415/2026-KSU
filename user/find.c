#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

static char path[512];
static char *pattern;
static int want; // 0 = both, T_FILE = files, T_DIR = directories
static int failed;

// Does name match pat, where '*' matches any sequence (including empty)?
static int
match(char *pat, char *name)
{
  if (*pat == 0)
    return *name == 0;
  if (*pat == '*') {
    for (;; name++) {
      if (match(pat + 1, name))
        return 1;
      if (*name == 0)
        return 0;
    }
  }
  return *pat == *name && match(pat + 1, name + 1);
}

// Search the directory named by path[0..len).
static void
walk(int len)
{
  struct stat st;
  struct dirent *de, *ents;
  int fd, n, got, i;
  char name[DIRSIZ + 1];

  if ((fd = open(path, 0)) < 0 || fstat(fd, &st) < 0 || st.type != T_DIR) {
    if (fd >= 0)
      close(fd);
    if (fd < 0) {
      fprintf(2, "find: cannot open %s\n", path);
      failed = 1;
    }
    return;
  }

  // Read the whole directory, then close it so deep trees don't run out of
  // file descriptors.
  ents = malloc(st.size + sizeof(*de));
  if (ents == 0) {
    close(fd);
    fprintf(2, "find: out of memory\n");
    failed = 1;
    return;
  }
  for (n = 0; n < st.size;) {
    got = read(fd, (char *)ents + n, st.size - n);
    if (got <= 0)
      break;
    n += got;
  }
  close(fd);
  n /= sizeof(*de);

  if (len > 0 && path[len - 1] != '/')
    path[len++] = '/';
  for (i = 0; i < n; i++) {
    de = &ents[i];
    if (de->inum == 0)
      continue;
    memmove(name, de->name, DIRSIZ);
    name[DIRSIZ] = 0;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;
    if (len + strlen(name) + 1 > sizeof(path)) {
      fprintf(2, "find: path too long\n");
      failed = 1;
      continue;
    }
    strcpy(path + len, name);
    if (stat(path, &st) < 0) {
      fprintf(2, "find: cannot open %s\n", path);
      failed = 1;
      continue;
    }
    if (match(pattern, name) &&
        (want == 0 || (want == T_DIR) == (st.type == T_DIR)))
      printf("%s\n", path);
    if (st.type == T_DIR)
      walk(len + strlen(name));
  }
  path[len] = 0;
  free(ents);
}

static void
usage(void)
{
  fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  int fd;

  if (argc != 3 && argc != 5)
    usage();
  if (argc == 5) {
    if (strcmp(argv[3], "-type") != 0)
      usage();
    if (strcmp(argv[4], "f") == 0)
      want = T_FILE;
    else if (strcmp(argv[4], "d") == 0)
      want = T_DIR;
    else
      usage();
  }
  if (strlen(argv[1]) >= sizeof(path)) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  if ((fd = open(argv[1], 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  close(fd);

  pattern = argv[2];
  strcpy(path, argv[1]);
  walk(strlen(path));
  exit(failed);
}
