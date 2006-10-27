/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file mar.hpp
   \brief TODO
 */

#ifndef AUGMARPP_MAR_HPP
#define AUGMARPP_MAR_HPP

#include "augmarpp/smartmar.hpp"

#include "augsyspp/exception.hpp"

#include <utility> // pair<>

namespace aug {

    inline bool
    ismatch(int result)
    {
        verify(result);
        return AUG_RETNOMATCH != result;
    }
    inline void
    copymar(marref dst, marref src)
    {
        verify(aug_copymar(dst.get(), src.get()));
    }
    inline smartmar
    createmar()
    {
        return smartmar::attach(verify(aug_createmar()));
    }
    inline smartmar
    openmar(const char* path, int flags)
    {
        return smartmar::attach(verify(aug_openmar(path, flags)));
    }
    inline smartmar
    openmar(const char* path, int flags, mode_t mode)
    {
        return smartmar::attach(verify(aug_openmar(path, flags, mode)));
    }
    inline void
    releasemar(aug_mar_t m)
    {
        verify(aug_releasemar(m));
    }
    inline void
    retainmar(aug_mar_t m)
    {
        verify(aug_retainmar(m));
    }
    inline void
    compactmar(marref ref)
    {
        verify(aug_compactmar(ref.get()));
    }
    inline void
    removefields(marref ref)
    {
        verify(aug_removefields(ref.get()));
    }
    inline unsigned
    setfield(marref ref, const aug_field& f)
    {
        unsigned ord;
        verify(aug_setfield(ref.get(), &f, &ord));
        return ord;
    }
    inline void
    setfield(marref ref, unsigned ord, const void* cdata, unsigned size)
    {
        verify(aug_setvalue(ref.get(), ord, cdata, size));
    }
    inline void
    setfield(marref ref, unsigned ord, const char* cdata)
    {
        setfield(ref, ord, cdata, (unsigned)strlen(cdata));
    }
    inline std::pair<unsigned, bool>
    unsetfield(marref ref, const char* name)
    {
        unsigned ord;
        bool match(ismatch(aug_unsetbyname(ref.get(), name, &ord)));
        return std::make_pair(ord, match);
    }
    inline bool
    unsetfield(marref ref, unsigned ord)
    {
        return ismatch(aug_unsetbyord(ref.get(), ord));
    }
    inline const void*
    getfield(marref ref, const char* name)
    {
        return verify(aug_valuebyname(ref.get(), name, 0));
    }
    inline const void*
    getfield(marref ref, const char* name, unsigned& size)
    {
        return verify(aug_valuebyname(ref.get(), name, &size));
    }
    inline const void*
    getfield(marref ref, unsigned ord, unsigned& size)
    {
        return verify(aug_valuebyord(ref.get(), ord, &size));
    }
    inline const void*
    getfield(marref ref, unsigned ord)
    {
        return verify(aug_valuebyord(ref.get(), ord, 0));
    }
    inline bool
    getfield(marref ref, aug_field& f, unsigned ord)
    {
        return ismatch(aug_getfield(ref.get(), &f, ord));
    }
    inline unsigned
    getfields(marref ref)
    {
        unsigned size;
        verify(aug_getfields(ref.get(), &size));
        return size;
    }
    inline bool
    toname(marref ref, const char*& s, unsigned ord)
    {
        return ismatch(aug_ordtoname(ref.get(), &s, ord));
    }
    inline std::pair<unsigned, bool>
    toord(marref ref, const char* name)
    {
        unsigned ord;
        bool match(ismatch(aug_nametoord(ref.get(), &ord, name)));
        return std::make_pair(ord, match);
    }
    inline void
    insertmar(marref ref, const char* path)
    {
        verify(aug_insertmar(ref.get(), path));
    }
    inline off_t
    seekmar(marref ref, off_t offset, int whence)
    {
        return verify(aug_seekmar(ref.get(), offset, whence));
    }
    inline void
    setcontent(marref ref, const void* cdata, unsigned size)
    {
        verify(aug_setcontent(ref.get(), cdata, size));
    }
    inline void
    setcontent(marref ref, const char* data)
    {
        setcontent(ref, data, (unsigned)strlen(data));
    }
    inline void
    syncmar(marref ref)
    {
        verify(aug_syncmar(ref.get()));
    }
    inline void
    truncatemar(marref ref, unsigned size)
    {
        verify(aug_truncatemar(ref.get(), size));
    }
    inline unsigned
    writemar(marref ref, const void* buf, unsigned size)
    {
        return verify(aug_writemar(ref.get(), buf, size));
    }
    inline void
    extractmar(marref ref, const char* path)
    {
        verify(aug_extractmar(ref.get(), path));
    }
    inline const void*
    content(marref ref, unsigned& size)
    {
        return verify(aug_content(ref.get(), &size));
    }
    inline const void*
    content(marref ref)
    {
        return verify(aug_content(ref.get(), 0));
    }
    inline unsigned
    readmar(marref ref, void* buf, unsigned size)
    {
        return verify(aug_readmar(ref.get(), buf, size));
    }
    inline unsigned
    contentsize(marref ref)
    {
        unsigned size;
        verify(aug_contentsize(ref.get(), &size));
        return size;
    }
}

#endif // AUGMARPP_MAR_HPP
