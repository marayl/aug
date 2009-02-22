\nocon % omit table of contents
\datethis % print date on listing

\def\AUG/{{\sc AUG}}
\def\CYGWIN/{{\sc CYGWIN}}
\def\IPV6/{{\sc IPv6}}
\def\GNU/{{\sc GNU}}
\def\LINUX/{{\sc LINUX}}
\def\MINGW/{{\sc MINGW}}
\def\MSVC/{{\sc MSVC}}
\def\POP3/{{\sc POP3}}
\def\POSIX/{{\sc POSIX}}
\def\PYTHON/{{\sc PYTHON}}
\def\RUBY/{{\sc RUBY}}
\def\SMTP/{{\sc SMTP}}
\def\SMTP/{{\sc SMTP}}
\def\SSL/{{\sc SSL}}
\def\WINDOWS/{{\sc WINDOWS}}

@s std int @s string int

@s mod int
@s basic_factory int
@s basic_module int
@s basic_session int
@s echo int
@s object int
@s session_base int
@s user int

@f line normal

@* Introduction.

\AUG/ is an event-driven network application server, licensed under the GNU
General Public License (GPL).  \AUG/'s Module system makes it ideally suited
to building heterogeneous Internet applications.  Python and Ruby Modules are
bundled with the \AUG/ package.  The core system is written in portable
\CEE/\AM\CPLUSPLUS/.  \AUG/ also supports \IPV6/ and \SSL/, and is available
for \LINUX/, \WINDOWS/ and other \POSIX/-compliant systems.

\yskip\noindent

This document offers a quick introduction to building and installing \AUG/
Modules, along with a brief insight into the application server itself.  For
further information, please visit the
\pdfURL{\AUG/ project home page}{http://www.xofy.org/aug} or email myself,
\pdfURL{Mark Aylett}{mailto:mark.aylett@@gmail.com}.

@* Event Model.

Carefully designed threading models can improve CPU utilisation on
multi-processor machines.  Similar effects, however, are rarely acheived when
ad hoc threads are used merely to ``simplify'' coding, or to enable
concurrency on blocking calls.  In such cases, complexity, resource contention
and the risk of deadlocks may actually increase, and performance degrade.

\yskip\noindent

Non-blocking APIs that support event multiplexing obviate the need for many
threads.  Fewer context switches, locks, and fuller cache pipelines lead to
greater efficiencies.  The downside is that multiplexing code often results in
more complex state transistions.  In a sense, these transitions are the
flattened interleavings of the multi-threaded model.

\yskip\noindent

Multiplexing code is best confined to specialised components dedicated to such
purposes.  This is where \AUG/ comes in: the \AUG/ application server uses an
event model, similar to the one described above, to multiplex signal, socket,
timer and user-event activity.  Complexity is confined to the application
server's internals, and Modules interact with the application server through a
sanitised interface.

\yskip\noindent

\AUG/ conducts all multiplexing activity on its event thread.  The event
thread is kept responsive by delegating CPU-intensive tasks to worker threads.
Iteractions between the event thread and worker threads are confined to event
queues, which minimise the possibility of deadlocks.  This model is synonymous
with the UI event thread model.

@* Modules and Sessions.

\AUG/ delegates event notifications to Modules and, in turn, Sessions.
Modules are physical components, dynamically loaded into the application
server at run-time.  Each Module manages one or more Sessions.  Modules and
Sessions are wired together at configuration-time, not compile-time.

\yskip\noindent

All Module calls are dispatched from the event thread.  A Session can,
therefore, either opt for a simple, single-threaded model, or a suitable
alternative such as a thread-pool --- \AUG/'s flexible design avoids imposing
artificial constraints on Module authors.

\yskip\noindent

The separation of physical Modules and logical Sessions allows Modules to
adapt and extend the application server environment exposed to Sessions.  The
\.{augpy} and \.{augrb} Modules, for example, adapt the host environment to
offer a \PYTHON/ or \RUBY/ oriented view to their associated Sessions.  These
language bindings are provided by Modules, and are unbeknown to the
application server.  A HTTP Session written in \RUBY/, for example, would be
managed by the \.{augrb} Module.

\yskip\noindent

Modules also help to promote component, rather than source-level reuse:
Sessions can interact with one-another by posting events to the event queue,
allowing Sessions to bridge language boundaries.

@* Administration.

System administrators are presented with a uniform interface across all
platforms.  Although, on \WINDOWS/, a d\ae monised \AUG/ process takes the
form of an NT service, from a sys-admin perspective, the interface remains the
same.  As with \LINUX/, the following command can be used to start the service
from a command prompt:

\yskip\noindent
\.{C:\\> augd -f augd.conf start}

@* Sample Module.

In the sections below, a Module is constructed, in \CPLUSPLUS/, that:

\yskip
\item{$\bullet$} exposes a TCP service;
\item{$\bullet$} reads line-based input from clients;
\item{$\bullet$} echos input lines back to clients in upper-case;
\item{$\bullet$} disconnects inactive clients.

\yskip\noindent

The Module is implemented in a single source file.  The layout of this file
is:

@c
@<include headers@>@;
@<using declarations@>@;
namespace {@/
@<implement echo session@>@;
}@/
@<declare export table@>

@ The \.{<augmodpp.hpp>} header is provided to simplify \CPLUSPLUS/ Module
implementations.  Modules can also be written in standard \CEE/.  A \CEE/
implementation would use the \.{<augmod.h>} header, instead.

@<include...@>+=
#define MOD_BUILD
#include <augmodpp.hpp>@/

@ For convenience, names are imported from the |aug|, |mod| and |std|
namespaces.

@<using...@>+=
using namespace aug;@/
using namespace mod;@/
using namespace std;

@ Session types (|echo| in this example) are fed into a class template which
assists with the \CEE/ to \CPLUSPLUS/ translation.  |basic_module<>| delegates
the task of creating Sessions to a factory object, specified as a template
argument.  Here, |basic_factory<>| is used to build a factory capable of
creating |echo| sessions.

\yskip\noindent
\AUG/ Modules are required to export two library functions, namely,
|mod_init()| and |mod_term()|.  The |MOD_ENTRYPOINTS| macro assists with the
definition of these two export functions.

@<declare...@>=
typedef basic_module<basic_factory<echo> > module;@/
MOD_ENTRYPOINTS(module::init, module::term)

@ The |echoline| functor handles each line received from the client.
\CPLUSPLUS/ Sessions implement the |session_base| interface.  Stub
implementations for most of |session_base|'s pure virtual functions are
provided by the |basic_session| class.  The following definition shows the
virtual functions that have been overriden in this example.

@<implement...@>=
@<echoline functor@>@;
struct echo : basic_session {@/
  bool
  do_start(const char* sname);

  void
  do_closed(const handle& sock);

  bool
  do_accepted(handle& sock, const char* name);

  void
  do_recv(const handle& sock, const void* buf, size_t size);

  void
  do_rdexpire(const handle& sock, unsigned& ms);

  static session_base*
  create(const char* sname);
};@/
@<member functions@>

@ The |do_start()| virtual function is called to start the Session.  This is
where Session initialisation is performed.  In this case, a TCP listener is
bound to a port which is read from the configuration file using the |getenv()|
function.  If the ``session.echo.serv'' property is missing from both the
configuration file and environment table, |false| is returned and the Session
deactivated.

@<member...@>+=
bool
echo::do_start(const char* sname)
{
  writelog(MOD_LOGINFO, "starting session [%s]", sname);
  const char* serv = mod::getenv("session.echo.serv");
  if (!serv)
    return false;
  tcplisten("0.0.0.0", serv);
  return true;
}

@ The |do_accepted()| function is called when a new client connection is
accepted.  The |setuser()| function binds an opaque, user-defined value to an
\AUG/ handle.  Here, a |string| buffer is assigned to track partial line data
received from the client.  An initial, {\sc ``HELLO''} message is sent to the
client.  The call to |setrwtimer()| establishes a timer that will expire when
no read activity has occurred on the |sock| handle for a period of 15 seconds
--- \AUG/ will automatically reset the timer whenever read activity occurs.

@<member...@>+=
bool
echo::do_accepted(handle& sock, const char* name)
{
  sock.setuser(new string());
  send(sock, "HELLO\r\n", 7);
  setrwtimer(sock, 15000, MOD_TIMRD);
  return true;
}

@ When a connection is closed, the |string| buffer associated with the socket
handle is deleted.

@<member...@>+=
void
echo::do_closed(const handle& sock)
{
  delete sock.user<string>();
}

@ |do_recv()| is called whenever new data is received from a client.  The
|tok| reference is bound to the |string| buffer used to store incomplete lines
between calls to |do_recv|.  The |tokenise()| function appends new data to the
back of |tok|.  Each complete line is processed by the |echoline| functor
before |tok| is cleared.

@<member...@>+=
void
echo::do_recv(const handle& sock, const void* buf, size_t size)
{
  string& tok(*sock.user<string>());
  tokenise(static_cast<const char*>(buf),
    static_cast<const char*>(buf) + size, tok, '\n', echoline(sock));
}

@ Read-timer expiry is communicated using the |do_rdexpire()| function.  If no
data arrives for 15 seconds, the connection is shutdown.  The |shutdown()|
function sends a FIN packet after ensuring that all buffered data has been
flushed.  \AUG/ ensures that any buffered messages are flushed before
performing the shutdown, and that any inflight messages sent by the client are
delivered to the Session.

@<member...@>+=
void
echo::do_rdexpire(const handle& sock, unsigned& ms)
{
  shutdown(sock, 0);
}

@ The |basic_factory| template requires that Session classes define a static
|create()| function.

@<member...@>+=
session_base*
echo::create(const char* sname)
{
  return 0 == strcmp(sname, "echo") ? new echo() : 0;
}

@ Each time the |echoline| functor is called, a response is prepared and sent
to the client associated with the |sock_| handle.

@<echoline...@>=
struct echoline {
  handle sock_;
  explicit
  echoline(const handle& sock)
    : sock_(sock)
  {
  }
  void
  operator ()(string& line)
  {
    @<prepare response@>@;
    send(sock_, line.c_str(), line.size());
  }
};

@ White-space, including any carriage-returns, are trimmed from the input
line.  Then, the response is prepared by converting the line to upper-case and
appending a CR/LF pair.  This end-of-line sequence is common in text-based,
network protocols such as \POP3/ and \SMTP/.

@<prepare...@>=
aug::trim(line);
transform(line.begin(), line.end(), line.begin(), aug::ucase);
line += "\r\n";

@ The \AUG/ libraries provide some standard string algorithms.

@<include...@>+=
#include <augutilpp/string.hpp>

@* Build and Install.

The \AUG/ package includes a \GNU/ makefile, named \.{aug.mk}, that can be
used to build Modules.  Simple Modules do not have any link-time dependencies;
all dependencies are injected into the Module when the Module is initialised.
Here is the complete \.{Makefile}:

\yskip
\.{CXXFLAGS = -I\$(AUG\_HOME)/include}

\.{CXXMODULES = modsample}

\.{modsample\_OBJS = modsample.o}

\.{all: all-aug}

\.{clean: clean-aug}

\.{include \$(AUG\_HOME)/etc/aug.mk}

@ First, the list of \CPLUSPLUS/ Modules to build are assigned to the
|CXXMODULES| variable:

\yskip
\.{CXXMODULES = modsample}

\yskip\noindent
Each Module can specify a list of |OBJS|, |LDFLAGS| and |LIBS|:

\yskip
\.{modsample\_OBJS = modsample.o}

\yskip\noindent
Finally, include the template makefile:

\yskip
\.{all: all-aug}

\.{clean: clean-aug}

\.{include \$(AUG\_HOME)/etc/aug.mk}

@ To configure the new Module, first add the name of the Session to the
``sessions'' property in the \.{augd.conf} file:

\yskip
\.{sessions = echo}

\yskip\noindent
Then, specify the Module that provides the Session, along with any additional
properties required by the Session implementation:

\yskip
\.{session.echo.module = sample}

\.{session.echo.serv = 5000}

\yskip\noindent
Finally, specify the path to the Module.  The file extension defaults to
|.dll| on \WINDOWS/, and |.so| otherwise:

\yskip
\.{module.sample.path = ./modsample}

\yskip
\yskip
\yskip
\yskip\noindent
Enjoy!

@* Index.