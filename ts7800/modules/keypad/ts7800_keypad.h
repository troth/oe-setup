#ifndef TS7800_KEYPAD_H
#define TS7800_KEYPAD_H

#include <linux/cdev.h>
#include <linux/mutex.h>

struct ts7800_keypad_dev {
    struct cdev cdev;           /* Char device structure. */
};

//#define ENABLE_DEBUG_PROC
#ifndef ENABLE_DEBUG_PROC
#  define dbg_ts7800_keypad_create_proc()
#  define dbg_ts7800_keypad_remove_proc()
#else
extern void dbg_ts7800_keypad_create_proc (void);
extern void dbg_ts7800_keypad_remove_proc (void);
#endif

#endif  /* TS7800_KEYPAD_H */
