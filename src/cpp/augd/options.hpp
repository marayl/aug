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
#ifndef AUGD_OPTIONS_HPP
#define AUGD_OPTIONS_HPP

#include <map>
#include <string>

#include "augsyspp/config.hpp"

#include "augctxpp/mpool.hpp"

namespace augd {

    class options : public aug::mpool_ops {
        std::map<std::string, std::string> options_;
    public:
        ~options() AUG_NOTHROW;

        void
        read(const char* path);

        void
        set(const std::string& name, const std::string& value);

        const char*
        get(const std::string& name, const char* def) const;

        const std::string&
        get(const std::string& name) const;
    };
}

#endif // AUGD_OPTIONS_HPP
