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

/* The keypad is a 4x4 grid layed out like this:

           +-------+-------+-------+-------+
    KP1    |  1/A  |  2/B  |  3/C  |  ^/<  |
           +-------+-------+-------+-------+
    KP2    |  4/D  |  5/E  |  6/F  |  v/>  |
           +-------+-------+-------+-------+
    KP3    |  7/G  |  8/H  |  9/I  |  2ND  |
           +-------+-------+-------+-------+
    KP4    |  CLR  |  0/J  |  HELP | ENTER |
           +-------+-------+-------+-------+

              KP5     KP6     KP7     KP8

   Where KPx is the keypad header pin number.

   Pressing a key will connect a Row Pin to a Col Pin. For example:
     * Pressing '1/A' key will make a connection from KP1 to KP5.
     * Pressing 'HELP' key will make a connection from KP4 to KP7.
     * etc.

   For more info: http://www.actcomponents.com/index_004.htm

   The keypad is connected to the DIO header of the TS7800 via ribbon
   cable. The DIO header is a 2x8 connector layed out like this:

           +----+----+----+----+----+----+----+----+
           |  2 |  4 |  6 |  8 | 10 | 12 | 14 | 16 |
           +----+----+----+----+----+----+----+----+
           | .1 |  3 |  5 |  7 |  9 | 11 | 13 | 15 |
           +----+----+----+----+----+----+----+----+

   The mapping of pins connected is such:

    KeyPad | DIO Hdr | Reg Bit
    -------+---------+--------
      KP1  |   DIO1  |   0
      KP2  |   DIO3  |   2
      KP3  |   DIO5  |   4
      KP4  |   DIO7  |   6
      KP5  |   DIO9  |   8
      KP6  |  DIO11  |  10
      KP7  |  DIO13  |  12
      KP8  |  DIO15  |  14
 */

#define DEV         "/dev/mem"
#define DIO_BASE    0xE8000000
#define DIO_INPUT   ((volatile uint32_t *)0xE8000004)
#define DIO_OUTPUT  ((volatile uint32_t *)0xE8000008)
#define PINMASK(x) (1 << (x << 1))

struct dio {
    uint32_t unused;
    uint32_t kp_input;
    uint32_t kp_output;
};
typedef volatile struct dio dio_t;

typedef enum keycodes {
    KEY_UNKNOWN     = 0,
    KEY_CLEAR       = 1,
    KEY_0           = 2,
    KEY_HELP        = 3,
    KEY_ENTER       = 4,
    KEY_1           = 5,
    KEY_2           = 6,
    KEY_3           = 7,
    KEY_UP          = 8,
    KEY_4           = 9,
    KEY_5           = 10,
    KEY_6           = 11,
    KEY_DOWN        = 12,
    KEY_7           = 13,
    KEY_8           = 14,
    KEY_9           = 15,
    KEY_2ND         = 16,
    KEY_NUM_CODES
} keycode_t;

const char * const key_names [] = {
    "UNKNOWN",
    "CLEAR",
    "0",
    "HELP",
    "ENTER",
    "1",
    "2",
    "3",
    "UP",
    "4",
    "5",
    "6",
    "DOWN",
    "7",
    "8",
    "9",
    "2ND",
};

dio_t *dio;

int
printkey (int key)
{
    if ((key < KEY_NUM_CODES) && (key > 0))
    {
        fprintf (stdout, "%s\n", key_names[key]);
        return 0;
    }

    fprintf (stdout, "%s [%d]\n", key_names[0], key);
    return -1;
}

int
getkey (void)
{
    int row;
    int col;
    uint32_t resp;
    int key = 0;

    for (row = 0; row < 4; row++)
    {
        /* Drive all pins high except the current one. */
        dio->kp_output = 0x5555U ^ PINMASK(row);

        /* Read the state of column pins. Column pins that are pulled low will
           show up as 1 bits in resp. */
        resp = 0x5500U ^ (0x5500U & dio->kp_input);

        if (resp)
        {
            for (col = 0; col < 4; col++)
            {
                if (PINMASK(col) & (resp >> 8))
                {
                    key = 4 * row + col + 1;
                }
            }
        }
    }

    return key;
}

int
accept_keypress (void)
{
    int key = 0;
    int cnt = 0;

    while (key == 0)
    {
        key = getkey ();
        usleep (1);
    }

    do {
        if (getkey () == key)
        {
            cnt = 0;
        }

        usleep (1);
    } while (++cnt < 10);

    return key;
}

int
main (int argc, char **argv)
{
    int fd;
    int key;

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

    fprintf (stdout, "Press keys on the keypad. Press 'ENTER' to quit.\n");

    do {
        key = accept_keypress ();
        printkey (key);
    } while (key != KEY_ENTER);

    fprintf (stdout, "\n");

    return 0;
}
