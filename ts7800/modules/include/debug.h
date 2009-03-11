#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUG_MOD_NAME
#  define DEBUG_MOD_NAME DBG
#endif

#define DEBUG_XSTR(s) DEBUG_STR(s)
#define DEBUG_STR(s) #s ": "
#define DEBUG_MOD DEBUG_XSTR(DEBUG_MOD_NAME)

#undef PDEBUG
#ifdef ENABLE_DEBUG
#  ifdef __KERNEL__             /* Kernel space version. */
#    define PDEBUG(fmt, args...) printk (KERN_DEBUG DEBUG_MOD fmt, ## args)
#  else                         /* User space version. */
#    define PDEBUG(fmt, args...) fprintf (stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)  /* Not debugging: do nothing. */
#endif

#undef PDEBUG_
#define PDEBUG_(fmt, args...)   /* Do nothing: it's a placeholder. */

#endif  /* DEBUG_H */
