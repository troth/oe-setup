#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>


#define DEV         "/dev/mem"
#define DIO_BASE    0xE8000000
#define DIO_INPUT   ((volatile uint32_t *)0xE8000004)
#define DIO_OUTPUT  ((volatile uint32_t *)0xE8000008)

struct dio {
    uint32_t unused;
    volatile uint32_t input;
    volatile uint32_t output;
};
typedef volatile struct dio dio_t;

dio_t *dio;

/* Returns a mask with one bit set in bit number 0,2,4,6,... */
#define PINMASK(x) (1 << ((x) << 1))

void
debug (uint32_t mask)
{
    int i = 2000;
    while (i--)
    {
        dio->output = mask;
        usleep (1);
        dio->output = 0;
        usleep (1);
    }
}

int
main (int argc, char **argv)
{
    int fd;
    uint32_t mask;
    int bit;

    fd = open (DEV, (O_RDWR | O_SYNC));
    if (fd < 0)
    {
        fprintf (stderr, "ERR: open of " DEV " failed: %s\n", strerror (errno));
        exit (1);
    }

    dio = mmap (0, getpagesize (), (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                DIO_BASE);
    if (dio == MAP_FAILED)
    {
        fprintf (stderr, "ERR: mmap of " DEV " failed: %s\n", strerror (errno));
        exit (1);
    }

    if (argc > 1)
    {
        bit = atoi (argv[1]);
        mask = 1 << bit;

        printf ("Toggling mask: 0x%08x [bit=%d]\n", mask, bit);
        debug (mask);
    }
    else
    {
        printf ("Usage: %s <bit>\n", argv[0]);
    }

    return 0;
}
