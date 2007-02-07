from augas import *
from buffer import *
from interpreter import *
import log

# void writelog(level, msg)
# string error()
# void reconf()
# void stop()
# void post(sname, type, user)
# void delegate(sname, type, user)
# string getenv(name)
# void shutdown(sock)
# int tcpconnect(sname, host, serv, user)
# int tcplisten(sname, host, serv, user)
# void send(sock, buffer buf)
# void setrwtimer(sock, ms, flags)
# void resetrwtimer(sock, ms, flags)
# void cancelrwtimer(sock, flags)
# int settimer(sname, ms, user)
# bool resettimer(timer, ms)
# bool canceltimer(timer)

class Commands:
    def __init__(self):
        self.props = {}

    def do_exit(self):
        return Quit

    def do_quit(self):
        return Quit

    def do_get(self, x):
        return self.props[x]

    def do_ls(self):
        return self.props.iteritems()

    def do_set(self, x, y):
        self.props[x] = y

    def do_unset(self, x):
        del self.props[x]

interp = Interpreter(Commands())

# for line in sys.stdin:
#     x = interp(line)
#     if x == Quit:
#         sys.exit()
#     elif x != None:
#         print x

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    tcplisten(sname, "0.0.0.0", getenv("session.pyskel.serv"), None)

def reconf(sname):
    log.debug("reconf(): %s" % sname)

def event(sname, type, user):
    log.debug("event(): %s" % sname)

def closed(sock):
    log.info("closed(): %s" % sock)

def teardown(sock):
    log.debug("teardown(): %s" % sock)
    shutdown(sock)

def accept(sock, addr, port):
    log.info("accept(): %s" % sock)
    sock.user = Buffer()
    setrwtimer(sock, 15000, TIMRD)
    send(sock, "+OK hello\r\n")

def connected(sock, addr, port):
    log.debug("connected(): %s" % sock)

def data(sock, buf):
    log.debug("data(): %s" % sock)
    global interp
    for line in sock.user.lines(str(buf)):
        x = interp(line)
        if x == Quit:
            send(sock, "+OK goodbye\r\n")
            shutdown(sock)
        elif x != None:
            send(sock, x + "\r\n")

def rdexpire(sock, ms):
    log.debug("rdexpire(): %s" % sock)
    send(sock, "+OK hello\r\n")

def wrexpire(sock, ms):
    log.debug("wrexpire(): %s" % sock)

def expire(timer, ms):
    log.debug("expire(): %s" % timer)