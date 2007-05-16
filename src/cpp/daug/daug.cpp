/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "daug/clntconn.hpp"
#include "daug/exception.hpp"
#include "daug/listener.hpp"
#include "daug/manager.hpp"
#include "daug/module.hpp"
#include "daug/options.hpp"
#include "daug/servconn.hpp"
#include "daug/ssl.hpp"
#include "daug/utility.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory> // auto_ptr<>
#include <sstream>
#include <queue>
#include <vector>

#include <time.h>

using namespace aug;
using namespace std;

#define DAUG_WAKEUP   (AUG_EVENTUSER + 0)
#define DAUG_MODEVENT (AUG_EVENTUSER + 1)

namespace augas {

    typedef char cstring[AUG_PATH_MAX + 1];

    const char* program_;
    cstring conffile_= "";
    cstring rundir_ = "";
    cstring logdir_ = "";
    options options_;
    bool daemon_(false);
    unsigned stopping_(0);

    void
    stop_(const aug_var& var, int id, unsigned& ms)
    {
        aug_info("giving-up, closing connections");
        stopping_ = 2;
    }

    void
    openlog_()
    {
        // The current date is appended to the log file name.

        tm tm;
        aug::gmtime(tm);
        stringstream ss;
        ss << "daug-" << setfill('0')
           << setw(4) << tm.tm_year + 1900
           << setw(2) << tm.tm_mon + 1
           << setw(2) << tm.tm_mday;
        openlog(makepath(logdir_, ss.str().c_str(), "log").c_str());
    }

    void
    reopen_(const aug_var& var, int id, unsigned& ms)
    {
        AUG_DEBUG2("re-opening log file");
        openlog_();
    }

    void
    doreconf_()
    {
        const char* value;

        // Always set log-level first so that any subsequent log statements
        // use the new level.

        if ((value = options_.get("loglevel", 0))) {
            unsigned level(strtoui(value, 10));
            AUG_DEBUG2("setting log level: level=[%d]", level);
            aug_setloglevel(level);
        }

        // Other config directories may be specified relative to the run
        // directory.

        aug::chdir(rundir_);
        if (daemon_) {

            aug::chdir(options_.get("logdir", "."));

            // Cache real path so that the log file can be re-opened without
            // having to change directories.

            realpath(logdir_, getcwd().c_str(), sizeof(logdir_));

            // Re-opening the log file facilitates rolling.

            openlog_();
        }

        AUG_DEBUG2("loglevel=[%d]", aug_loglevel());
        AUG_DEBUG2("rundir=[%s]", rundir_);
    }

    typedef map<int, servptr> idtoserv;
    typedef queue<connptr> connected;

    struct event {
        string from_, to_, type_;
        aug_var var_;
        ~event() AUG_NOTHROW
        {
            try {
                destroyvar(var_);
            } AUG_PERRINFOCATCH;
        }
        event(const string& from, const string& to, const string& type)
            : from_(from),
              to_(to),
              type_(type)
        {
            var_.type_ = 0;
            var_.arg_ = 0;
        }
    };

    struct state {

        aug_nbfilecb_t cb_;
        aug_var var_;
#if HAVE_OPENSSL_SSL_H
        sslctxs sslctxs_;
#endif // HAVE_OPENSSL_SSL_H
        aug::nbfiles nbfiles_;
        manager manager_;
        timers timers_;
        timer grace_;

        // Mapping of timer-ids to services.

        idtoserv idtoserv_;

        // Pending calls to connected().

        connected connected_;

        ~state() AUG_NOTHROW
        {
            AUG_DEBUG2("removing event pipe from list");
            try {
                removenbfile(aug_eventin());
            } AUG_PERRINFOCATCH;
        }
        state(aug_nbfilecb_t cb, const aug_var& var, const string& pass64)
            : cb_(cb),
              var_(var),
              grace_(timers_)
        {
#if HAVE_OPENSSL_SSL_H
            initssl();
            createsslctxs(sslctxs_, options_, pass64);
#endif // HAVE_OPENSSL_SSL_H

            AUG_DEBUG2("inserting event pipe to list");
            insertnbfile(nbfiles_, aug_eventin(), cb, var);
            setnbeventmask(aug_eventin(), AUG_FDEVENTRD);
        }
    };

    auto_ptr<state> state_;

    void
    stop_()
    {
        if (!stopping_) {
            stopping_ = 1;
            state_->manager_.teardown();
            state_->grace_.set(15000, timercb<stop_>, null);
        }
    }

    void
    timercb_(const aug_var* var, int id, unsigned* ms)
    {
        AUG_DEBUG2("custom timer expiry");

        idtoserv::iterator it(state_->idtoserv_.find(id));
        servptr serv(it->second);
        augas_object timer = { id, var->arg_ };
        serv->expire(timer, *ms);

        if (0 == *ms) {
            AUG_DEBUG2("removing timer: ms has been set to zero");
            state_->idtoserv_.erase(it);
        }
    }

    void
    setsockopts_(const smartfd& sfd)
    {
        setnodelay(sfd, true);
        setnonblock(sfd, true);
    }

    void
    connected_(conn_base& conn)
    {
        setsockopts_(conn.sfd());

        const endpoint& ep(conn.endpoint());
        inetaddr addr(null);
        AUG_DEBUG2("connected: host=[%s], port=[%d]",
                   inetntop(getinetaddr(ep, addr)).c_str(),
                   static_cast<int>(ntohs(port(ep))));

        setnbeventmask(conn.sfd(), AUG_FDEVENTRD);
        conn.connected(ep);
    }

    // Thread-safe.

    void
    writelog_(int level, const char* format, ...)
    {
        // Cannot throw.

        va_list args;
        va_start(args, format);
        aug_vwritelog(level, format, args);
        va_end(args);
    }

    // Thread-safe.

    void
    vwritelog_(int level, const char* format, va_list args)
    {
        // Cannot throw.

        aug_vwritelog(level, format, args);
    }

    // Thread-safe.

    const char*
    error_()
    {
        return aug_errdesc;
    }

    // Thread-safe.

    int
    reconfall_()
    {
        try {
            aug_event e = { AUG_EVENTRECONF, AUG_VARNULL };
            writeevent(aug_eventout(), e);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    // Thread-safe.

    int
    stopall_()
    {
        try {
            aug_event e = { AUG_EVENTSTOP, AUG_VARNULL };
            writeevent(aug_eventout(), e);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    // Thread-safe.

    int
    post_(const char* to, const char* type, const augas_var* var)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("post(): sname=[%s], to=[%s], type=[%s]", sname, to, type);
        try {

            auto_ptr<augas::event> arg(new augas::event(sname, to, type));
            aug_event e;
            e.type_ = DAUG_MODEVENT;
            e.var_.type_ = 0;
            e.var_.arg_ = arg.get();
            writeevent(aug_eventout(), e);
            aug_setvar(&arg->var_, var);
            arg.release();
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    dispatch_(const char* to, const char* type, const void* user, size_t size)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("dispatch(): sname=[%s], to=[%s], type=[%s]",
                   sname, to, type);
        try {

            vector<servptr> servs;
            state_->manager_.getservs(servs, to);

            vector<servptr>::const_iterator it(servs.begin()),
                end(servs.end());
            for (; it != end; ++it)
                (*it)->event(sname, type, user, size);

            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    const char*
    getenv_(const char* name, const char* def)
    {
        // Return absolute rundir.

        if (0 == strcmp(name, "rundir"))
            return rundir_;

        try {

            const char* value(options_.get(name, 0));

            /* Fallback to environment table. */

            if (!value)
                value = getenv(name);

            return value ? value : def;

        } AUG_SETERRINFOCATCH;
        return 0;
    }

    const augas_serv*
    getserv_()
    {
        try {
            return getserv();
        } AUG_SETERRINFOCATCH;
        return 0;
    }

    int
    shutdown_(augas_id cid)
    {
        AUG_DEBUG2("shutdown(): id=[%d]", cid);
        try {
            objectptr sock(state_->manager_.getbyid(cid));
            connptr cptr(smartptr_cast<conn_base>(sock));
            if (null != cptr)
                cptr->shutdown();
            else
                state_->manager_.erase(*sock);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    tcpconnect_(const char* host, const char* port, void* user)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("tcpconnect(): sname=[%s], host=[%s], port=[%s]",
                   sname, host, port);
        try {

            servptr serv(state_->manager_.getserv(sname));
            connptr cptr(new augas::clntconn(serv, user, state_->timers_,
                                             host, port));

            // Remove on exception.

            scoped_insert si(state_->manager_, cptr);

            if (ESTABLISHED == cptr->phase()) {

                // connected() must only be called after this function has
                // returned.

                insertnbfile(state_->nbfiles_, cptr->sfd(), state_->cb_,
                             state_->var_);
                setnbeventmask(cptr->sfd(), AUG_FDEVENTRD);

                if (state_->connected_.empty()) {

                    // Schedule an event to ensure that connected() is called
                    // after this function has returned.

                    aug_event e = { DAUG_WAKEUP, AUG_VARNULL };
                    writeevent(aug_eventout(), e);
                }

                // Add to pending queue.

                state_->connected_.push(cptr);

            } else {

                insertnbfile(state_->nbfiles_, cptr->sfd(), state_->cb_,
                             state_->var_);
                setnbeventmask(cptr->sfd(), AUG_FDEVENTALL);
            }

            si.commit();
            return (int)cptr->id();

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    tcplisten_(const char* host, const char* port, void* user)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("tcplisten(): sname=[%s], host=[%s], port=[%s]",
                   sname, host, port);
        try {

            // Bind listener socket.

            endpoint ep(null);
            smartfd sfd(tcplisten(host, port, ep));

            insertnbfile(state_->nbfiles_, sfd, state_->cb_, state_->var_);
            setnbeventmask(sfd, AUG_FDEVENTRD);

            inetaddr addr(null);
            AUG_DEBUG2("listening: interface=[%s], port=[%d]",
                       inetntop(getinetaddr(ep, addr)).c_str(),
                       static_cast<int>(ntohs(aug::port(ep))));

            // Prepare state.

            servptr serv(state_->manager_.getserv(sname));
            listenerptr lptr(new augas::listener(serv, user, sfd));
            scoped_insert si(state_->manager_, lptr);

            si.commit();
            return (int)lptr->id();

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    send_(augas_id cid, const void* buf, size_t len)
    {
        AUG_DEBUG2("send(): id=[%d]", cid);
        try {
            if (!state_->manager_.append(cid, buf, len))
                throw error(__FILE__, __LINE__, EHOSTCALL,
                            "connection has been shutdown");
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    sendv_(augas_id cid, const augas_var* var)
    {
        AUG_DEBUG2("sendv(): id=[%d]", cid);
        try {
            if (!state_->manager_.append(cid, *var))
                throw error(__FILE__, __LINE__, EHOSTCALL,
                            "connection has been shutdown");
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    setrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        AUG_DEBUG2("setrwtimer(): id=[%d], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            rwtimerptr rwtimer(smartptr_cast<
                               rwtimer_base>(state_->manager_.getbyid(cid)));
            if (null == rwtimer)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);
            rwtimer->setrwtimer(ms, flags);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resetrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        AUG_DEBUG2("resetrwtimer(): id=[%d], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            rwtimerptr rwtimer(smartptr_cast<
                               rwtimer_base>(state_->manager_.getbyid(cid)));
            if (null == rwtimer)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);
            return rwtimer->resetrwtimer(ms, flags) ? 0 : AUGAS_NONE;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    cancelrwtimer_(augas_id cid, unsigned flags)
    {
        AUG_DEBUG2("cancelrwtimer(): id=[%d], flags=[%x]", cid, flags);
        try {
            rwtimerptr rwtimer(smartptr_cast<
                               rwtimer_base>(state_->manager_.getbyid(cid)));
            if (null == rwtimer)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);
            return rwtimer->cancelrwtimer(flags) ? 0 : AUGAS_NONE;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    settimer_(unsigned ms, const struct augas_var* var)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("settimer(): sname=[%s], ms=[%u]", sname, ms);
        try {

            augas_id id(aug_nextid());

            // If aug_settimer() succeeds, it will call aug_destroyvar() on
            // var when the timer is destroyed.  The service is added to the
            // container first to minimise any chance of failure after
            // aug_settimer() has been called.

            state_->idtoserv_[id] = state_->manager_.getserv(sname);
            if (-1 == aug_settimer(cptr(state_->timers_), id, ms, timercb_,
                                   var))
                state_->idtoserv_.erase(id);

            return id;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resettimer_(augas_id tid, unsigned ms)
    {
        AUG_DEBUG2("resettimer(): id=[%d], ms=[%u]", tid, ms);
        try {
            return aug_resettimer(cptr(state_->timers_), tid, ms);
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    canceltimer_(augas_id tid)
    {
        AUG_DEBUG2("canceltimer(): id=[%d]", tid);
        try {
            int ret(aug_canceltimer(cptr(state_->timers_), tid));

            // Only erase if aug_canceltimer() returns true: may be in the
            // midst of a aug_foreachexpired() call, in which case,
            // aug_canceltimer() will return false for the timer being
            // expired.

            if (0 == ret)
                state_->idtoserv_.erase(tid);
            return ret;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    setsslclient_(augas_id cid, const char* ctx)
    {
        AUG_DEBUG2("setsslclient(): id=[%d], ctx=[%s]", cid, ctx);
#if HAVE_OPENSSL_SSL_H
        try {
            sslctxs::const_iterator it(state_->sslctxs_.find(ctx));
            if (it == state_->sslctxs_.end())
                throw error(__FILE__, __LINE__, ESSLCTX,
                            "SSL context [%s] not initialised", ctx);

            connptr cptr(smartptr_cast<
                         conn_base>(state_->manager_.getbyid(cid)));
            if (null == cptr)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);

            it->second->setclient(*cptr);
            return 0;
        } AUG_SETERRINFOCATCH;
#else // !HAVE_OPENSSL_SSL_H
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_setsslclient() not supported"));
#endif // !HAVE_OPENSSL_SSL_H
        return -1;
    }

    int
    setsslserver_(augas_id cid, const char* ctx)
    {
        AUG_DEBUG2("setsslserver(): id=[%d], ctx=[%s]", cid, ctx);
#if HAVE_OPENSSL_SSL_H
        try {
            sslctxs::const_iterator it(state_->sslctxs_.find(ctx));
            if (it == state_->sslctxs_.end())
                throw error(__FILE__, __LINE__, ESSLCTX,
                            "SSL context [%s] not initialised", ctx);

            connptr cptr(smartptr_cast<
                         conn_base>(state_->manager_.getbyid(cid)));
            if (null == cptr)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);

            it->second->setserver(*cptr);
            return 0;
        } AUG_SETERRINFOCATCH;
#else // !HAVE_OPENSSL_SSL_H
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_setsslserver() not supported"));
#endif // !HAVE_OPENSSL_SSL_H
        return -1;
    }

    const augas_host host_ = {
        writelog_,
        vwritelog_,
        error_,
        reconfall_,
        stopall_,
        post_,
        dispatch_,
        getenv_,
        getserv_,
        shutdown_,
        tcpconnect_,
        tcplisten_,
        send_,
        sendv_,
        setrwtimer_,
        resetrwtimer_,
        cancelrwtimer_,
        settimer_,
        resettimer_,
        canceltimer_,
        setsslclient_,
        setsslserver_
    };

    void
    teardown_(const augas_object* sock)
    {
        aug_info("teardown defaulting to shutdown");
        shutdown_(sock->id_);
    }

    void
    load_()
    {
        AUG_DEBUG2("loading services");
        state_->manager_.load(rundir_, options_, host_, teardown_);

        // Remove any timers allocated to services that could not be opened.

        idtoserv::iterator it(state_->idtoserv_.begin()),
            end(state_->idtoserv_.end());
        while (it != end) {
            if (!it->second->active()) {
                aug_warn("cancelling timer associated with inactive service");
                aug_canceltimer(cptr(state_->timers_), it->first);
                state_->idtoserv_.erase(it++);
            } else
                ++it;
        }
    }

    void
    accept_(const object_base& sock)
    {
        endpoint ep(null);

        AUG_DEBUG2("accepting connection");

        smartfd sfd(null);
        try {

            sfd = accept(sock.sfd(), ep);

        } catch (const errinfo_error& e) {

            if (aug_acceptlost()) {
                aug_warn("accept() failed: %s", e.what());
                return;
            }
            throw;
        }

        insertnbfile(state_->nbfiles_, sfd, state_->cb_, state_->var_);
        setnbeventmask(sfd, AUG_FDEVENTRD);


        setsockopts_(sfd);
        connptr cptr(new augas::servconn(sock.serv(), sock.user(),
                                         state_->timers_, sfd, ep));

        scoped_insert si(state_->manager_, cptr);
        AUG_DEBUG2("initialising connection: id=[%d], fd=[%d]", cptr->id(),
                   sfd.get());

        if (cptr->accept(ep))
            si.commit();
    }

    bool
    process_(const connptr& cptr, int fd, unsigned short events)
    {
        bool changed = false, ok = false;
        try {
            changed = cptr->process(events);
            ok = true;
        } AUG_PERRINFOCATCH;

        if (!ok) {

            // Connection is closed if an exception is thrown during
            // processing.

            state_->manager_.erase(*cptr);
            return false;
        }

        if (CONNECTING == cptr->phase()) {

            // The associated file descriptor may change as connection
            // attempts fail and alternative addresses are tried.

            insertnbfile(state_->nbfiles_, cptr->sfd(), state_->cb_,
                         state_->var_);
            setnbeventmask(cptr->sfd(), AUG_FDEVENTALL);

            state_->manager_.update(cptr, fd);

        } else if (changed)

            switch (cptr->phase()) {
            case ESTABLISHED:

                // Was connecting, now established: notify module of
                // connection establishment.

                connected_(*cptr);
                break;
            case CLOSED:
                state_->manager_.erase(*cptr);
                return false;
            default:
                break;
            }

        return true;
    }

    bool
    readevent_()
    {
        aug_event event;
        AUG_DEBUG2("reading event");

        switch (aug::readevent(aug_eventin(), event).type_) {
        case AUG_EVENTRECONF:
            AUG_DEBUG2("received AUG_EVENTRECONF");
            if (*conffile_) {
                AUG_DEBUG2("reading config-file: path=[%s]", conffile_);
                options_.read(conffile_);
            }
            doreconf_();
            state_->manager_.reconf();
            break;
        case AUG_EVENTSTATUS:
            AUG_DEBUG2("received AUG_EVENTSTATUS");
            break;
        case AUG_EVENTSTOP:
            AUG_DEBUG2("received AUG_EVENTSTOP");
            stop_();
            break;
        case AUG_EVENTSIGNAL:
            AUG_DEBUG2("received AUG_EVENTSIGNAL");
            break;
        case DAUG_WAKEUP:
            AUG_DEBUG2("received DAUG_WAKEUP");
            // Actual handling is performed in do_run().
            break;
        case DAUG_MODEVENT:
            AUG_DEBUG2("received DAUG_MODEVENT");
            {
                auto_ptr<augas::event> ev(static_cast<
                                          augas::event*>(event.var_.arg_));

                vector<servptr> servs;
                state_->manager_.getservs(servs, ev->to_);

                size_t size;
                const void* user(varbuf(ev->var_, size));

                vector<servptr>::const_iterator it(servs.begin()),
                    end(servs.end());
                for (; it != end; ++it)
                    (*it)->event(ev->from_.c_str(), ev->type_.c_str(), user,
                                 size);
            }
        }
        destroyvar(event.var_);
        return true;
    }

    class service {

        string pass64_;

    public:
        ~service() AUG_NOTHROW
        {
        }

        const char*
        getopt(enum aug_option opt)
        {
            switch (opt) {
            case AUG_OPTCONFFILE:
                return *conffile_ ? conffile_ : 0;
            case AUG_OPTEMAIL:
                return PACKAGE_BUGREPORT;
            case AUG_OPTLONGNAME:
                return "aug application server";
            case AUG_OPTPIDFILE:
                return options_.get("pidfile", "daug.pid");
            case AUG_OPTPROGRAM:
                return program_;
            case AUG_OPTSHORTNAME:
                return "daug";
            }
            return 0;
        }

        void
        readconf(const char* conffile, bool prompt, bool daemon)
        {
            if (prompt) {
                char pass[AUG_MAXPASSWORD + 1];
                aug_getpass("Enter PEM pass phrase:", pass, sizeof(pass));
                pass64_ = filterbase64(pass, strlen(pass), AUG_ENCODE64);
                memset(pass, 0, sizeof(pass));
            }

            // The conffile is optional, if specified it will be an absolute
            // path.

            if (conffile) {

                AUG_DEBUG2("reading config-file: path=[%s]", conffile);
                options_.read(conffile);

                // Store the absolute path to service any reconf requests.

                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon;

            // Once set, the run directory should not change.

            const char* rundir(options_.get("rundir", 0));
            realpath(rundir_, rundir ? rundir : getcwd().c_str(),
                     sizeof(rundir_));

            doreconf_();
        }

        void
        init()
        {
            AUG_DEBUG2("initialising daemon process");

            setsrvlogger("daug");

            aug_var var = { 0, this };
            auto_ptr<state> s(new state(nbfilememcb<service>, var, pass64_));
            state_ = s;
            try {
                load_();
            } catch (...) {

                // Ownership back to local.

                s = state_;
                s->manager_.clear();
                throw;
            }
        }

        void
        run()
        {
            timer reopen(state_->timers_);

            // Re-open log file every minute.

            if (daemon_)
                reopen.set(60000, timercb<reopen_>, null);

            AUG_DEBUG2("running daemon process");

            int ret(!0);
            while (!stopping_ || !state_->manager_.empty()) {

                if (2 == stopping_)
                    break;

                try {

                    if (state_->timers_.empty()) {

                        scoped_unblock unblock;
                        while (AUG_RETINTR == (ret = waitnbevents
                                               (state_->nbfiles_)))
                            ;

                    } else {

                        AUG_DEBUG2("processing timers");

                        timeval tv;
                        foreachexpired(state_->timers_, 0 == ret, tv);

                        scoped_unblock unblock;
                        while (AUG_RETINTR == (ret = waitnbevents
                                               (state_->nbfiles_, tv)))
                            ;
                    }

                    // Notify of any established connections before processing
                    // the files: data may have arrived on a newly established
                    // connection.

                    while (!state_->connected_.empty()) {
                        connected_(*state_->connected_.front());
                        state_->connected_.pop();
                    }

                    AUG_DEBUG2("processing files");

                    foreachnbfile(state_->nbfiles_);
                    continue;

                } AUG_PERRINFOCATCH;

                // When running in foreground, stop on error.

                if (!daemon_)
                    stop_();
            }
        }

        void
        term()
        {
            AUG_DEBUG2("terminating daemon process");

            // Clear services first.

            state_->manager_.clear();

            // Modules must be kept alive until remaining connections and
            // timers have been destroyed: a destroy_() function may depend on
            // a function implemented in a module.

            state_.reset();
        }

        bool
        nbfilecb(int fd, unsigned short events)
        {
            // Intercept activity on event pipe.

            if (fd == aug_eventin())
                return readevent_();

            objectptr sock(state_->manager_.getbyfd(fd));
            connptr cptr(smartptr_cast<conn_base>(sock)); // Downcast.

            AUG_DEBUG2("processing sock: id=[%d], fd=[%d]", sock->id(), fd);

            if (null != cptr)
                return process_(cptr, fd, events);

            accept_(*sock);
            return true;
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace augas;

    try {

        aug_errinfo errinfo;
        scoped_init init(errinfo);

        timeval tv;
        aug::gettimeofday(tv);
        aug::srand(getpid() ^ tv.tv_sec ^ tv.tv_usec);

        try {
            service serv;
            program_ = argv[0];

            blocksignals();
            aug_setloglevel(AUG_LOGINFO);
            return main(argc, argv, serv);

        } AUG_PERRINFOCATCH;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}