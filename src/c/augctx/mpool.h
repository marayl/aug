/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_MPOOL_H
#define AUGCTX_MPOOL_H

#include "augctx/config.h"

#include "augext/mpool.h"

AUGCTX_API aug_mpool*
aug_getcrtmalloc(void);

AUGCTX_API aug_mpool*
aug_createdlmalloc(void);

#endif /* AUGCTX_MPOOL_H */
