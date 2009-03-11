#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/fs.h>

#include <asm/uaccess.h>

#include "debug.h"
#include "ts7800_keypad.h"

/* Parameters that can be set at load time. */

// None yet.

static void ts7800_keypad_cleanup (void)
{
}

static int __init ts7800_keypad_init (void)
{
    int res = 0;

    PDEBUG ("module loaded\n");
    return 0;

 fail:
    ts7800_keypad_cleanup ();
    return res;
}

static void __exit ts7800_keypad_exit (void)
{
    ts7800_keypad_cleanup ();
    PDEBUG ("module unloaded\n");
}

module_init (ts7800_keypad_init);
module_exit (ts7800_keypad_exit);

MODULE_AUTHOR ("Theodore A. Roth");
MODULE_DESCRIPTION ("Input driver for TS-7800 DIO Keypad.");
MODULE_LICENSE ("GPL");
