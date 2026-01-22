#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/param.h"

void find(char *path, char *name, char *cmd[], int cmd_count)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    fprintf(2, "find needs first arg as directory.");
    close(fd);
    exit(1);
  }

  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
    fprintf(2, "find: path too long\n");
    close(fd);
    return;  
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
      printf("find: cannot stat %s\n", buf);
      continue;
    }
    if (st.type == T_DIR && strcmp(de.name, ".") && strcmp(de.name, "..")) {
        find(buf, name, cmd, cmd_count);
    }
    else if (st.type == T_FILE && strcmp(de.name, name) == 0) {
      if (cmd_count == 0) {
        printf("%s\n", buf);
      } else {
        int pid = fork();
        if (pid < 0) {
          fprintf(2, "an error occured when creating new child \n");
          exit(1);
        }
        if (fork() == 0) {
          cmd[cmd_count] = buf;
          exec(cmd[0], cmd);
          exit(0);
        }
        wait((int *)0);
      }
      
    }
  }
  close(fd);
}

int main(int argc, char **argv)
{
  if(argc < 3){
    fprintf(2, "usage: find path name [-exec cmd is optional] \n");
    exit(1);
  }
  if (argc >= MAXARG) {
	fprintf(2, "find: too much args\n");
	exit(1);
  }
  if (strlen(argv[2]) > MAXPATH) {
    fprintf(2, "find: filename too long\n");
    exit(1);
  }

  if (argc == 3) {
    find(argv[1], argv[2], 0, 0);
  }

  char *cmd[MAXARG];
  if (argc > 3 && strcmp(argv[3], "-exec") == 0) {
    int i = 0;
    for(; i < argc - 4; i++) {
      cmd[i] = argv[i + 4];
    }
    // cmd[i] = argv[2];
    find(argv[1], argv[2], cmd, i);
  }
  
  exit(0);
}