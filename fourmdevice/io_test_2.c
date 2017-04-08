#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <sys/ioctl.h> 
//needed for IO things. Attention that this is different from kernel mode
int lcd;
#define SCULL_IOC_MAGIC 'k'
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC, 1)
#define SCULL_MSG_WRITE _IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_MSG_READ _IOR(SCULL_IOC_MAGIC, 3, int)
#define SCULL_MSG_READ_WRITE _IOWR(SCULL_IOC_MAGIC, 4, int)

void test()
{
  int k, i, sum;
  char s[3];

  memset(s, '2', sizeof(s));
  printf("test begin!\n");

  k = write(lcd, s, sizeof(s));
  printf("written = %d\n", k);

  k = ioctl(lcd, SCULL_HELLO);
  printf("result = %d\n", k);

  char *dev_msg = "this is an interesting message!";
  k = ioctl(lcd, SCULL_MSG_WRITE, dev_msg);
  printf("msg write result = %d. dev message address = %d\n", k, dev_msg);

  char *user_msg = "this is an awesome message!";
  k = ioctl(lcd, SCULL_MSG_READ_WRITE, user_msg);
  printf("msg read result = %d. user message address = %d\n", k, user_msg);
  printf(user_msg);
}

int main(int argc, char **argv)
{ 
  lcd = open("/dev/fourmdevice", O_RDWR);
  if (lcd == -1) {
    perror("unable to open lcd");
    exit(EXIT_FAILURE);
  }

  test();
  close(lcd);
  return 0;
}

