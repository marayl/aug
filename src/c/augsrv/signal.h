/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_SIGNAL_H
#define AUGSRV_SIGNAL_H

#include "augsrv/config.h"

#include "augtypes.h"

AUGSRV_API aug_result
aug_setsighandler(void (*handler)(int));

AUGSRV_API aug_result
aug_blocksignals(void);

AUGSRV_API aug_result
aug_unblocksignals(void);

#endif /* AUGSRV_SIGNAL_H */
