/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGUTIL_BUILD
#include "augutil/lexer.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/networds.h"
#include "augutil/shellwords.h"
#include "augutil/types.h"
#include "augutil/xstr.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <ctype.h>  /* isspace() */

struct aug_lexer_ {
    aug_mpool* mpool_;
    struct aug_words words_;
    void (*put_)(struct aug_words*, int);
    aug_xstr_t xstr_;
    int what_;
    char save_;
};

static void
out_(void* arg, int what)
{
    aug_lexer_t lexer = (aug_lexer_t)arg;
    switch (what) {
    case AUG_TOKLABEL:
        lexer->what_ |= AUG_LEXLABEL;
        break;
    case AUG_TOKWORD:
        lexer->what_ |= AUG_LEXWORD;
        break;
    case AUG_TOKPHRASE:
        lexer->what_ |= AUG_LEXPHRASE;
        break;
    case AUG_TOKRTRIM:
        aug_clearxstrn(lexer->xstr_,
                       aug_rtrimword(aug_xstr(lexer->xstr_),
                                     aug_xstrlen(lexer->xstr_)));
        break;
    default:
        if (lexer->what_)
            lexer->save_ = what;
        else
            aug_xstrcatc(lexer->xstr_, what);
        break;
    }
}

static aug_lexer_t
createlexer_(aug_mpool* mpool, size_t size)
{
    aug_lexer_t lexer;

    /* Use default size when size has not been specified. */

    if (0 == size)
        size = AUG_MAXLINE;

    if (!(lexer = aug_allocmem(mpool, sizeof(struct aug_lexer_))))
        return NULL;

    if (!(lexer->xstr_ = aug_createxstr(mpool, size))) {
        aug_freemem(mpool, lexer);
        return NULL;
    }

    lexer->mpool_ = mpool;
    lexer->what_ = 0;
    lexer->save_ = 0;

    aug_retain(mpool);
    return lexer;
}

AUGUTIL_API aug_lexer_t
aug_createnetlexer(aug_mpool* mpool, size_t size)
{
    aug_lexer_t lexer = createlexer_(mpool, size);
    if (lexer) {
        aug_initnetwords(&lexer->words_, out_, lexer);
        lexer->put_ = aug_putnetwords;
    }
    return lexer;
}

AUGUTIL_API aug_lexer_t
aug_createshelllexer(aug_mpool* mpool, size_t size, aug_bool pairs)
{
    aug_lexer_t lexer = createlexer_(mpool, size);
    if (lexer) {
        aug_initshellwords(&lexer->words_, pairs, out_, lexer);
        lexer->put_ = aug_putshellwords;
    }
    return lexer;
}

AUGUTIL_API void
aug_destroylexer(aug_lexer_t lexer)
{
    aug_mpool* mpool = lexer->mpool_;
    aug_destroyxstr(lexer->xstr_);
    aug_freemem(mpool, lexer);
    aug_release(mpool);
}

AUGUTIL_API unsigned
aug_appendlexer(aug_lexer_t lexer, char ch)
{
    if (lexer->what_) {
        aug_clearxstr(lexer->xstr_);
        lexer->what_ = 0;
        if (lexer->save_) {
            aug_xstrcatc(lexer->xstr_, lexer->save_);
            lexer->save_ = 0;
        }
    }
    lexer->put_(&lexer->words_, ch);
    return lexer->what_;
}

AUGUTIL_API unsigned
aug_finishlexer(aug_lexer_t lexer)
{
    return aug_appendlexer(lexer, '\n');
}

AUGUTIL_API const char*
aug_lexertoken(aug_lexer_t lexer)
{
    return lexer->what_ ? aug_xstr(lexer->xstr_) : NULL;
}
