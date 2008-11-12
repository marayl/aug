/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/clock.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
# include <sys/time.h>
#else /* _WIN32 */
# include <sys/timeb.h> /* _ftime() */
# include <winsock2.h>  /* struct timeval */
# define ftime _ftime
# define timeb _timeb
#endif /* _WIN32 */

struct impl_ {
    aug_clock clock_;
    int refs_;
    aug_mpool* mpool_;
    long timezone_;
};

static void*
cast_(aug_clock* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_clockid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_clock* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, clock_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_clock* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, clock_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static aug_result
gettimeofday_(aug_clock* obj, struct timeval* tv)
{
#if !defined(_WIN32)
    /* FIXME: policy for setting errinfo? */
    if (-1 == gettimeofday(tv, NULL))
        return AUG_FAILERROR;
#else /* _WIN32 */
    struct timeb tb;
    ftime(&tb);
    tv->tv_sec = (long)tb.time;
    tv->tv_usec = tb.millitm * 1000;
#endif /* _WIN32 */
    return AUG_SUCCESS;
}

static long
gettimezone_(aug_clock* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, clock_, obj);
    return impl->timezone_;
}

static const struct aug_clockvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    gettimeofday_,
    gettimezone_
};

AUGCTX_API aug_clock*
aug_createclock(aug_mpool* mpool, long tz)
{
    struct impl_* impl = aug_allocmem(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->clock_.vtbl_ = &vtbl_;
    impl->clock_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->timezone_ = tz;

    aug_retain(mpool);
    return &impl->clock_;
}
