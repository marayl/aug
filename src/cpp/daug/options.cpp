/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "daug/options.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "daug/exception.hpp"

#include "augutilpp/file.hpp"

using namespace aug;
using namespace daug;
using namespace std;

namespace {
    aug_result
    cb(void* arg, const char* name, const char* value)
    {
        map<string, string>& x(*static_cast<map<string, string>*>(arg));
        x[name] = value;
        return AUG_SUCCESS;
    }
}

options::~options() AUG_NOTHROW
{
}

void
options::read(const char* path)
{
    options_.clear();
    readconf(path, confcb<cb>, &options_);
}

void
options::set(const string& name, const string& value)
{
    options_[name] = value;
}

const char*
options::get(const string& name, const char* def) const
{
    map<string, string>::const_iterator it(options_.find(name));
    if (options_.find(name) == options_.end())
        return def;
    return it->second.c_str();
}

const string&
options::get(const string& name) const
{
    map<string, string>::const_iterator it(options_.find(name));
    if (options_.find(name) == options_.end())
        throw daug_error(__FILE__, __LINE__, ECONFIG,
                         "missing option: name=[%s]", name.c_str());
    return it->second;
}
