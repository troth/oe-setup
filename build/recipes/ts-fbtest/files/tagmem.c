#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tagmem.h"

volatile unsigned short *open_gpio(int *fd) {
  volatile unsigned short *gpio;

  *fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (*fd < 0) {
    perror("/dev/mem");
    return 0;
  }
  gpio = (unsigned short *) mmap(0, 4096, PROT_READ | PROT_WRITE,
				 MAP_SHARED, *fd, 0x600ff000);
  assert (gpio != MAP_FAILED);
  gpio += (0x86 / sizeof(unsigned short));
  return gpio;
}

void close_gpio(int fd,volatile unsigned short *gpio) {
  munmap((void *)gpio,4096);
  close(fd);
}

void sspi_cmd(volatile unsigned short *gpio,unsigned int cmd) {
  unsigned int i;

  // pulse CS#
  *gpio = (*gpio & 0xFFF0) | 0x2;
  *gpio = (*gpio & 0xFFF0) | 0x0;

  for (i = 0; i < 32; i++, cmd <<= 1) {
    if (cmd & 0x80) {
      *gpio = (*gpio & 0xFFF0) | 0x4;
      *gpio = (*gpio & 0xFFF0) | 0xc;
    } else {
      *gpio = (*gpio & 0xFFF0) | 0x0;
      *gpio = (*gpio & 0xFFF0) | 0x8;
    }
  }
}

int read_tagmem(int *tagmem) {
  int i, j, fd;
  unsigned int cmd, ret;
  volatile unsigned short *gpio = open_gpio(&fd);

  if (!gpio) return 0;

  sspi_cmd(gpio,0xac); // X_PROGRAM_EN
  sspi_cmd(gpio,0x4e); // READ_TAG

  for (j = 0; j < 20; j++) {
    for (ret = 0x0, i = 0; i < 32; i++) {
      *gpio = (*gpio & 0xFFF0) | 0x0;
      *gpio = (*gpio & 0xFFF0) | 0x8;
      ret = ret << 1 | (*gpio & 0x1);
    }
    tagmem[j] = ret;
    //printf("%08X ",tagmem[j]);
    //if (j % 5 == 4) printf("\n");
  }

  sspi_cmd(gpio,0x78); // PROGRAM_DIS

  *gpio = (*gpio & 0xFFF0) | 0x2;
  close_gpio(fd,gpio);
  return 1;
}

int write_tagmem(int *tagmem) {
  int i, j, fd;
  unsigned int cmd, ret;
  volatile unsigned short *gpio = open_gpio(&fd);

  if (!gpio) return 0;

  sspi_cmd(gpio,0xac); // X_PROGRAM_EN
  sspi_cmd(gpio,0x8e); // WRITE_TAG

  for (j = 0; j < 20; j++) {
    unsigned int x = tagmem[j];
    for (i = 0; i < 32; i++, x <<= 1) {
      if (x & 0x80000000UL) {
	*gpio = (*gpio & 0xFFF0) | 0x4;
	*gpio = (*gpio & 0xFFF0) | 0xc;
      } else {
	*gpio = (*gpio & 0xFFF0) | 0x0;
	*gpio = (*gpio & 0xFFF0) | 0x8;
      }
      if (i == 23 && j == 19) break;
    }
  }

  for (i = 0; i < 8; i++) {
    *gpio = (*gpio & 0xFFF0) | 0x2;
    *gpio = (*gpio & 0xFFF0) | 0xa;
  }
  *gpio = (*gpio & 0xFFF0) | 0x2;
  close_gpio(fd,gpio);
  usleep(25000);
  return 1;
}

int erase_tagmem() {
  int i, j, fd;
  unsigned int cmd, ret;
  volatile unsigned short *gpio = open_gpio(&fd);

  sspi_cmd(gpio,0xac); // X_PROGRAM_EN
  sspi_cmd(gpio,0xe); // ERASE_TAG

  for (i = 0; i < 8; i++) {
    *gpio = (*gpio & 0xFFF0) | 0x2;
    *gpio = (*gpio & 0xFFF0) | 0xa;
  }
  *gpio = (*gpio & 0xFFF0) | 0x2;
  close_gpio(fd,gpio);
  usleep(1000000);
  return 1;
}

// gcc  -mcpu=arm9  -c tagmem.c
