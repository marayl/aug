/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_DAEMON_H
#define AUGSERV_DAEMON_H

#include "augserv/config.h"

#include "augtypes.h"

struct aug_options;

/**
 * Called from aug_main().
 *
 * @return On Windows, #AUG_FAILNONE if the service has not been installed.
 */

AUGSERV_API aug_result
aug_daemonise(const struct aug_options* options);

#endif /* AUGSERV_DAEMON_H */
