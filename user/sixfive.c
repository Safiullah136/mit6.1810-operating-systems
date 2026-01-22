#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define BUF_SIZE 40

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: sixfive files...\n");
    exit(1);
  }

  char buf[BUF_SIZE];
  char separators[] = " -\r\t\n./,";
  int j = 0, read_num;
  uint8 valid_number = 1;

  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd == -1) {
        fprintf(2, "cannot open file %s \n", argv[i]);
        continue;
    }

    while((read_num = read(fd, &buf[j], 1)) >= 0) {
        if(strchr(separators, buf[j]) || read_num == 0) {
            if (valid_number) {
                buf[j] = 0;
                int n = atoi(buf);
                if (n % 5 == 0 || n % 6 == 0) printf("%d\n", n);
            }
            j = 0;
            valid_number = 1;
        } else {
            if(buf[j] < '0' || buf[j] > '9') valid_number = 0;
            j++;
        }

        if (read_num == 0) break;
    }
    close(fd);
  }

  exit(0);
}
