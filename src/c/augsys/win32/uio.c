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
#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augtypes.h"

AUGSYS_API aug_rsize
aug_freadv_BI(aug_fd fd, const struct iovec* iov, int size)
{
    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug",
                    AUG_ESUPPORT, AUG_MSG("aug_freadv_BI() not supported"));
    return -1;
}

AUGSYS_API aug_rsize
aug_fwritev_BI(aug_fd fd, const struct iovec* iov, int size)
{
    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug",
                    AUG_ESUPPORT, AUG_MSG("aug_fwritev_BI() not supported"));
    return -1;
}
