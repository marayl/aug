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
#ifndef AUGUTIL_EVENT_H
#define AUGUTIL_EVENT_H

/**
 * @file augutil/event.h
 *
 * Event type.
 */

#include "augutil/config.h"

#include "augsys/types.h"

#include "augext/mpool.h"

#include "augabi.h"
#include "augtypes.h"

#define AUG_EVENTRECONF 1
#define AUG_EVENTSTATUS 2
#define AUG_EVENTSTOP   3
#define AUG_EVENTSIGNAL 4
#define AUG_EVENTWAKEUP 5

/**
 * Base value for user-defined events.
 */

#define AUG_EVENTUSER   32

struct aug_event {
    int type_;
    aug_object* ob_;
};

typedef struct aug_events_* aug_events_t;

AUGUTIL_API aug_events_t
aug_createevents_BIN(aug_mpool* mpool);

AUGUTIL_API void
aug_destroyevents(aug_events_t events);

/**
 * Read next event.
 *
 *
 * @return See @ref TypesResult.  #AUG_FAILBLOCK if no events are available.
 */

AUGUTIL_API aug_result
aug_readevent(aug_events_t events, struct aug_event* event);

/**
 * Write event.  This call is thread-safe.  Events can be used to marshal data
 * to sessions from foreign threads.  Events can also be written from signal
 * handlers.  If this function is called from a thread where no context has
 * been initialised, then no errinfo structure will be populated on error.
 *
 * @return See @ref TypesResult.  #AUG_FAILBLOCK if the underlying event
 * buffer is full.
 */

AUGUTIL_API aug_result
aug_writeevent_B(aug_events_t events, const struct aug_event* event);

AUGUTIL_API aug_md
aug_eventsmd(aug_events_t events);

/**
 * This function simplifies the mapping of signals to events.  Some signals
 * have cannonical mappings to specific events types.  For example, #SIGHUP
 * maps to the #AUG_EVENTRECONF event type.  Others may get mapped to the more
 * general #AUG_EVENTSIGNAL type.
 */

AUGUTIL_API struct aug_event*
aug_sigtoevent(int sig, struct aug_event* event);

#endif /* AUGUTIL_EVENT_H */
