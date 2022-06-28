#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int powOfTwo(int n);

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p, authority[7];
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:

      for (int i = 0; i < 6; i++) {
//        printf(1, "%d, %d, %d\n", powOfTwo(5 - i), st.authority, powOfTwo(5 - i) & st.authority);
        if(powOfTwo(5 - i) & st.authority){
          if (i % 3 == 0) {
            authority[i + 1] = 'r';
          } else if (i % 3 == 1) {
            authority[i + 1] = 'w';
          } else {
            authority[i + 1] = 'x';
          }
        } else {
          authority[i + 1] = '-';
        }
      }

      authority[0] = '-';
//      printf(0, "file\n");


      printf(0, "%s %s %d %d %d\n", fmtname(path), authority, st.type, st.ino, st.size);
//    printf(5, "%s %d %d %d \n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
      if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }

      for (int i = 0; i < 6; i++) {
//        printf(1, "%d, %d, %d\n", powOfTwo(5 - i), st.authority, powOfTwo(5 - i) & st.authority);
        if(powOfTwo(5 - i) & st.authority){
          if (i % 3 == 0) {
            authority[i + 1] = 'r';
          } else if (i % 3 == 1) {
            authority[i + 1] = 'w';
          } else {
            authority[i + 1] = 'x';
          }
        } else {
          authority[i + 1] = '-';
        }
//        printf(0, "%s\n", authority);
      }

      if (st.type == T_DIR){
        authority[0] = 'd';
//        printf(0, "dir\n");
      } else {
        authority[0] = '-';
//        printf(0, "file\n");
      }


//      printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
//      printf(0, "%s\n", authority);
      printf(0, "%s %s\nowner: %s type: %d ino: %d size:%d\n\n", fmtname(buf), authority, st.ownername, st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int powOfTwo(int n){
  int tmp = 1;
  for (int i = 0; i < n; i++){
    tmp *= 2;
  }
  return tmp;
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit();
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit();
}
