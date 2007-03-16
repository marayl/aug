#include "augaspp.hpp"

using namespace augas;
using namespace std;

namespace {

    struct eachline {
        object sock_;
        explicit
        eachline(const object& sock)
            : sock_(sock)
        {
        }
        void
        operator ()(std::string& tok)
        {
            trim(tok);
            transform(tok.begin(), tok.end(), tok.begin(), ucase);
            tok += "\r\n";

            send(sock_, tok.c_str(), tok.size());
        }
    };

    struct echoserv : basic_serv {
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");
            const char* serv = augas::getenv("service.echo.serv");
            if (!serv)
                return false;

            tcplisten("0.0.0.0", serv);
            return true;
        }
        void
        do_event(const char* from, const char* type, const void* user,
                 size_t size)
        {
            if (0 == strcmp(type, "application/x-www-form-urlencoded")) {
            }
        }
        void
        do_closed(const object& sock)
        {
            delete sock.user<string>();
        }
        bool
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            sock.setuser(new string());
            send(sock, "hello\r\n", 7);
            setrwtimer(sock, 15000, AUGAS_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const void* buf, size_t size)
        {
            string& tok(*sock.user<string>());
            tokenise(static_cast<const char*>(buf),
                     static_cast<const char*>(buf) + size, tok, '\n',
                     eachline(sock));
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
            writelog(AUGAS_LOGINFO, "no data received for 15 seconds");
            shutdown(sock);
        }
        static serv_base*
        create(const char* sname)
        {
            return new echoserv();
        }
    };

    typedef basic_module<basic_factory<echoserv> > module;
}

AUGAS_MODULE(module::init, module::term)
