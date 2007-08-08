/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/ptimer.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augutil/posix/ptimer.c"
#else /* _WIN32 */
# include "augutil/win32/ptimer.c"
#endif /* _WIN32 */