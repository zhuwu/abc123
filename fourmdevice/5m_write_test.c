#include <fcntl.h>
#include <stdio.h>

#define FIVEM 5242880

char file_data[FIVEM];

int main() {
  int file_fd = open("random.txt", O_RDONLY);
  read(file_fd, file_data, FIVEM);
  close(file_fd);

  int dev_fd = open("/dev/fourmdevice", O_RDWR);
  if (dev_fd == -1) {
    printf("Failed to open device\n");
    exit(-1);
  }

  int result = write(dev_fd, file_data, FIVEM);
  printf("Written result = %d\n", result);

  close(dev_fd);
  return 0;
}
