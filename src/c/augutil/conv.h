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
#ifndef AUGUTIL_CONV_H
#define AUGUTIL_CONV_H

/**
 * @file augutil/conv.h
 *
 * String conversion functions.
 */

#include "augutil/config.h"

AUGUTIL_API unsigned long*
aug_strtoul(const char* src, unsigned long* dst, int base);

AUGUTIL_API unsigned*
aug_strtoui(const char* src, unsigned* dst, int base);

AUGUTIL_API unsigned short*
aug_strtous(const char* src, unsigned short* dst, int base);

#endif /* AUGUTIL_CONV_H */
