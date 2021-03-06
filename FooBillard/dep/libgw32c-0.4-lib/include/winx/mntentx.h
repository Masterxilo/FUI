#ifndef __WINX_MNTENTX_H__
#define __WINX_MNTENTX_H__
#ifdef __GW32__

#ifndef _MNTENT_H
#include <winx/misc/mntentx.h>

/* Now define the internal interfaces.  */
extern FILE *__setmntent (__const char *__file, __const char *__mode);
extern struct mntent *__getmntent_r (FILE *__stream,
				     struct mntent *__result,
				     char *__buffer, int __bufsize);
extern int __addmntent (FILE *__stream, __const struct mntent *__mnt);
extern int __endmntent (FILE *__stream);
extern char *__hasmntopt (__const struct mntent *__mnt,
			  __const char *__opt);
#endif

#endif /* __GW32__ */

#endif /* __WINX_MNTENTX_H__ */
