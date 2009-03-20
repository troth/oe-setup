/* TS-7800 user space driver for Lumex 2x24 LCD.

   Pinouts for the 2x8 connector on the LCD board looking at back of board
   oriented with 2x8 connector along north edge (silk screen has markings for
   pins 1, 2, 15, and 16):

   +----+----+----+----+----+----+----+----+
   | 16 | 14 | 12 | 10 |  8 |  6 |  4 |  2 |
   +----+----+----+----+----+----+----+----+
   | 15 | 13 | 11 |  9 |  7 |  5 |  3 |  1 |
   +----+----+----+----+----+----+----+----+

   Pinouts for the LCD 2x7 connector on the TS-7800 board:

   +----+----+----+----+----+----+----+
   |  2 |  4 |  6 |  8 | 10 | 12 | 14 |
   +----+----+----+----+----+----+----+
   |  1 |  3 |  5 |  7 |  9 | 11 | 13 |
   +----+----+----+----+----+----+----+

   Connections from TS-7800 board to LCD board (via 14 conductor ribbon
   cable with 2x7 IDC connectors on each each):

   +-----------------+-----+
   | Hdr Pin Numbers |     |
   +-----------------+ TS  +-----+---------+-----+---------------------------+
   | TS-7800 |  LCD  | BIT | Sym |  Level  | I/O | Function                  |
   +=========+=======+=====+=====+=========+=====+===========================+
   |     1   |   2   | N/A | Vcc |    -    |  -  | Power supply (+5V)        |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     2   |   1   | N/A | Vss |    -    |  -  | Power supply (GND)        |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     3   |   4   |  18 | RS  |   0/1   |  I  | 0 = Instruction input     |
   |         |       |     |     |         |     | 1 = Data input            |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     4   |   3   |  19 | Vee |    -    |  -  | Contrast adjust           |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     5   |   6   |  20 | E   |  1,1->0 |  I  | Enable signal             |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     6   |   5   |  21 | R/W |   0/1   |  I  | 0 = Write to LCD module   |
   |         |       |     |     |         |     | 1 = Read from LCD module  |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     7   |   8   |  22 | DB1 |   0/1   | I/O | Data bus line 1           |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     8   |   7   |  23 | DB0 |   0/1   | I/O | Data bus line 0 (LSB)     |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |     9   |  10   |  24 | DB3 |   0/1   | I/O | Data bus line 3           |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |    10   |   9   |  25 | DB2 |   0/1   | I/O | Data bus line 2           |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |    11   |  12   |  26 | DB5 |   0/1   | I/O | Data bus line 5           |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |    12   |  11   |  27 | DB4 |   0/1   | I/O | Data bus line 4           |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |    13   |  14   |  28 | DB7 |   0/1   | I/O | Data bus line 7 (MSB)     |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |    14   |  13   |  29 | DB6 |   0/1   | I/O | Data bus line 6           |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |    NC   |  15   | N/A |  A  |    -    |  -  | LED Backlight Anode       |
   +---------+-------+-----+-----+---------+-----+---------------------------+
   |    NC   |  16   | N/A |  K  |    -    |  -  | LED Backlight Cathode     |
   +---------+-------+-----+-----+---------+-----+---------------------------+

 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>

#define DEV "/dev/mem"

#define CHARS_PER_LINE 24

#define MASK(bit) (1 << (bit))

#define LO 0
#define HI 1

#define RS_INST 0
#define RS_DATA 1

#define RW_WRITE 0
#define RW_READ  1

#define LCD_RS    MASK(18)
#define LCD_EN    MASK(20)
#define LCD_RW    MASK(21)

#define LCD_DB0   MASK(23)
#define LCD_DB1   MASK(22)
#define LCD_DB2   MASK(25)
#define LCD_DB3   MASK(24)
#define LCD_DB4   MASK(27)
#define LCD_DB5   MASK(26)
#define LCD_DB6   MASK(29)
#define LCD_DB7   MASK(28)

#define LCD_DATA_MASK (LCD_DB0 | LCD_DB1 | LCD_DB2 | LCD_DB3 \
                       | LCD_DB4 | LCD_DB5 | LCD_DB6 | LCD_DB7)

#define LCD_BUSY  MASK(29)

/*
 * The commands are described here:
 *   http://home.iae.nl/users/pouweha/lcd/lcd0.shtml#hd44780
 */

/* The following are command arguments. */

/* CMD ENTRY_MODE: Cursor position move direction (increment or decrement) */
#define DIR_DECR   0
#define DIR_INCR   MASK(1)

/* CMD ENTRY_MODE: Shift the display, yes or no. */
#define SHIFT_OFF 0
#define SHIFT_ON  MASK(0)

/* CMD DISP_CTRL: Display on or off. */
#define DISP_OFF 0
#define DISP_ON  MASK(2)

/* CMD DISP_CTRL: Cursor on or off. */
#define CUR_OFF 0
#define CUR_ON  MASK(1)

/* CMD DISP_CTRL: Cursor Blink on or off. */
#define BLINK_OFF 0
#define BLINK_ON  MASK(0)

/* CMD SHIFT: Move cursor or shift disp. */
#define MOVE_CURSOR 0
#define SHIFT_DISP  MASK(3)

/* CMD SHIFT: Shift left or right. */
#define SHIFT_LEFT  0
#define SHIFT_RIGHT MASK(2)

/* CMD FUNC: 4-bit or 8-bit interface. */
#define IF_4BIT 0
#define IF_8BIT MASK(4)

/* CMD FUNC: 1 line or 2 lines. */
#define LINES_ONE 0
#define LINES_TWO MASK(3)

/* CMD FUNC: Font 5x7 or 5x10 */
#define FONT_5X7  0
#define FONT_5X10 MASK(2)

/* The actual commands. */

#define LCD_CMD_CLR_SCREEN           MASK(0)
#define LCD_CMD_CURSOR_HOME          MASK(1)
#define LCD_CMD_ENTRY_MODE(dir, sh)  (MASK(2) | (dir) | (sh))
#define LCD_CMD_DISP_CTRL(d, c, b)   (MASK(3) | (d)   | (c)  | (b))
#define LCD_CMD_SHIFT(sc, rl)        (MASK(4) | (sc)  | (rl))
#define LCD_CMD_FUNC(dl, n, f)       (MASK(5) | (dl)  | (n)  | (f))
#define LCD_CMD_CGRAM_ADDR(addr)     (MASK(6) | (addr))
#define LCD_CMD_DDRAM_ADDR(addr)     (MASK(7) | (addr))

#define DIO_BASE 0xE8000000

struct dio {
    uint32_t unused;
    volatile uint32_t input;
    volatile uint32_t output;
};
typedef volatile struct dio dio_t;

dio_t *dio;

void
do_mem_map (void)
{
    int fd;

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
}

uint32_t
translate_data_to_device (int data)
{
    uint32_t dev = 0;

    if (data & MASK(0))
        dev |= LCD_DB0;

    if (data & MASK(1))
        dev |= LCD_DB1;

    if (data & MASK(2))
        dev |= LCD_DB2;

    if (data & MASK(3))
        dev |= LCD_DB3;

    if (data & MASK(4))
        dev |= LCD_DB4;

    if (data & MASK(5))
        dev |= LCD_DB5;

    if (data & MASK(6))
        dev |= LCD_DB6;

    if (data & MASK(7))
        dev |= LCD_DB7;

    return dev;
}

int
translate_data_from_device (uint32_t dev_data)
{
    uint8_t data = 0;

    if (dev_data & LCD_DB0)
        data |= MASK(0);

    if (dev_data & LCD_DB1)
        data |= MASK(1);

    if (dev_data & LCD_DB2)
        data |= MASK(2);

    if (dev_data & LCD_DB3)
        data |= MASK(3);

    if (dev_data & LCD_DB4)
        data |= MASK(4);

    if (dev_data & LCD_DB5)
        data |= MASK(5);

    if (dev_data & LCD_DB6)
        data |= MASK(6);

    if (dev_data & LCD_DB7)
        data |= MASK(7);

    return data;
}

/* Either set or clear all bits in mask. */

void
lcd_set_bits (int lvl, uint32_t mask)
{
    uint32_t tmp;

#if 1
    if (lvl == LO)
        dio->output &= ~mask;
    else
        dio->output |= mask;
#else
    tmp = dio->output;
    
    if (lvl == LO)
        tmp &= ~mask;
    else:
        tmp |= mask;

    dio->output = tmp;
#endif

    /* Force a bus cycle to flush the write. */
    tmp = dio->output;
}

void
lcd_set_data (uint32_t dev_data)
{
    uint32_t tmp;

    tmp = dio->output & ~LCD_DATA_MASK;
    tmp |= (dev_data & LCD_DATA_MASK);
    dio->output = tmp;

    /* Force a bus cycle to flush the write. */
    tmp = dio->output;
}

uint32_t
lcd_get_data (void)
{
    uint32_t dev_data = dio->input;

    return dev_data;
}

void
lcd_set_rs (int lvl)
{
    lcd_set_bits (lvl, LCD_RS);
}

void
lcd_set_en (int lvl)
{
    lcd_set_bits (lvl, LCD_EN);
}

void
lcd_set_rw (int lvl)
{
    lcd_set_bits (lvl, LCD_RW);
}

void
lcd_put_data (uint8_t data)
{
    uint32_t xfer_data = translate_data_to_device (data);

    lcd_set_data (xfer_data);
}

void
busy_loop (int cnt)
{
    volatile int i = cnt;

    while (i--)
        ;
}

/* Waveform for testing busy bit:
       _____________________________________________________
   RS
                _________________________________________
   R/W ________/                                         \__
         ____      ____      ____      ____      ____
   EN  _/    \____/    \____/    \____/    \____/    \______
         ____      ____      ____      ____                 
   DB7 #X____X####/    \####/    \####/    \####\____/######

         DATA      BUSY      BUSY      BUSY      READY
  */

int
lcd_wait_not_busy (void)
{
    int timeout = 1000;
    uint32_t data = 0;

    do {
        lcd_set_rs (RS_INST);
        lcd_set_rw (RW_READ);
        busy_loop (10000);

        lcd_set_en (HI);
        busy_loop (10000);

        data = lcd_get_data ();

        lcd_set_en (LO);
        busy_loop (10000);
    } while (timeout-- && (data & LCD_DB7));

    if (!timeout)
        printf ("TIMEOUT: wait not busy\n");

    /* Returns 0 on timeout, or non-zero for success. */

    return timeout;
}

void
lcd_cmd (uint8_t cmd)
{
    lcd_put_data (cmd);
    lcd_set_rw (RW_WRITE);
    lcd_set_rs (RS_INST);
    busy_loop (1000);

    lcd_set_en (HI);
    busy_loop (10000);
    lcd_set_en (LO);

    lcd_wait_not_busy ();
}

void
lcd_write_char (int ch)
{
    lcd_put_data (ch);
    lcd_set_rs (RS_DATA);
    lcd_set_rw (RW_WRITE);
    busy_loop (1000);

    lcd_set_en (HI);
    busy_loop (10000);
    lcd_set_en (LO);

    lcd_wait_not_busy ();    
}

void
lcd_write_line (int line, const char *buf)
{
    int cnt = CHARS_PER_LINE;
    uint32_t addr = (40 * line); /* Address of second line is 40. */

    lcd_cmd (LCD_CMD_DDRAM_ADDR(addr));

    while (cnt-- && *buf)
    {
        lcd_write_char (*buf);
        buf++;
    }
}

void
lcd_init (void)
{
    uint8_t cmd = LCD_CMD_FUNC (IF_8BIT, LINES_TWO, FONT_5X7);

    lcd_cmd (cmd);
    lcd_cmd (cmd);
    lcd_cmd (cmd);

    lcd_cmd (LCD_CMD_ENTRY_MODE (DIR_INCR, SHIFT_OFF));
    lcd_cmd (LCD_CMD_CLR_SCREEN);
    lcd_cmd (LCD_CMD_CURSOR_HOME);
    lcd_cmd (LCD_CMD_DISP_CTRL (DISP_ON, CUR_OFF, BLINK_OFF));
}

int
main (int argc, char **argv)
{
    const char *line1;
    const char *line2;

    if (argc > 1)
    {
        line1 = argv[1];
        line2 = argv[2];
    }
    else
    {
        fprintf (stderr, "Usage: %s <line_1> <line_2>\n", argv[0]);
        exit (1);
    }

    do_mem_map ();

    lcd_init ();

    lcd_write_line (0, line1);
    if (line2)
        lcd_write_line (1, line2);

    return 0;
}
