/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_CONFIG_H
#define AUGSYS_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSYS_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGSYS_SHARED)
# define AUGSYS_API AUG_EXTERNC
#else /* AUGSYS_SHARED */
# if !defined(AUGSYS_BUILD)
#  define AUGSYS_API AUG_EXTERNC AUG_IMPORT
# else /* AUGSYS_BUILD */
#  define AUGSYS_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGSYS_BUILD */
#endif /* AUGSYS_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGSYS_BUILD)
#  pragma comment(lib, "libaugsys.lib")
# endif /* AUGSYS_BUILD */
# pragma comment(lib, "iphlpapi.lib")
#endif /* _MSC_VER */

#endif /* AUGSYS_CONFIG_H */
