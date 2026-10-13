#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

char path[512];  // the path we are currently looking at
char *pattern;   // what the user typed, e.g. *.c
int want;        // 0 = files and dirs, T_FILE = only files, T_DIR = only dirs
int failed;      // set to 1 if anything could not be opened

// Does name match pat? A '*' in pat matches any characters (even none).
int
match(char *pat, char *name)
{
  if (*pat == 0)
    return *name == 0;

  if (*pat == '*') {
    // Try letting the * eat 0, 1, 2, ... characters of name.
    while (1) {
      if (match(pat + 1, name))
        return 1;
      if (*name == 0)
        return 0;
      name++;
    }
  }

  if (*pat != *name)
    return 0;
  return match(pat + 1, name + 1);
}

// Look inside the directory stored in path (len = its length).
void
search(int len)
{
  struct stat st;
  struct dirent *entries;
  char name[DIRSIZ + 1];
  int fd, total, n, i;

  fd = open(path, 0);
  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    failed = 1;
    return;
  }
  fstat(fd, &st);
  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  // Read the whole directory, then close it right away. A process can
  // only have 16 files open, so we must not keep one open per level.
  entries = malloc(st.size + 1);
  total = 0;
  while (total < st.size) {
    n = read(fd, (char *)entries + total, st.size - total);
    if (n <= 0)
      break;
    total += n;
  }
  close(fd);
  total = total / sizeof(struct dirent);

  // Add a '/' after the directory name (unless it already has one).
  if (path[len - 1] != '/') {
    path[len] = '/';
    len++;
  }

  for (i = 0; i < total; i++) {
    if (entries[i].inum == 0)
      continue;
    memmove(name, entries[i].name, DIRSIZ);
    name[DIRSIZ] = 0;

    // Never look at . or ..
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    // path becomes "<dir>/<name>"
    strcpy(path + len, name);
    if (stat(path, &st) < 0) {
      fprintf(2, "find: cannot open %s\n", path);
      failed = 1;
      continue;
    }

    // Print it if the name matches and the type is what was asked for.
    if (match(pattern, name)) {
      if (want == 0 || (want == T_DIR && st.type == T_DIR) ||
          (want == T_FILE && st.type != T_DIR))
        printf("%s\n", path);
    }

    // Go inside directories.
    if (st.type == T_DIR)
      search(len + strlen(name));
  }

  path[len] = 0;
  free(entries);
}

void
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

  // If the directory can't be opened, say so and exit with status 1.
  fd = open(argv[1], 0);
  if (fd < 0 || strlen(argv[1]) > 400) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  close(fd);

  pattern = argv[2];
  strcpy(path, argv[1]);
  search(strlen(path));
  exit(failed);
}
