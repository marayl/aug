from augas import *
import log

class Client:
    def __del__(self):
        log.info("destroying State object")

    def __init__(self, sname, cid):
        self.sname = sname
        self.cid = cid
        self.tid = settimer(sname, 0, 10, self)
        self.n = 10

    def cancel(self):
        canceltimer(self.tid)

    def done(self):
        self.n = self.n - 1
        return self.n < 0

    def expire(self):
        if not self.done():
            send(self.cid, "hello, world!")
        else:
            log.info("done: shutting client connection")
            shutdown(self.cid)
            return 0

def init(sname):
    log.info("connecting to client's service")
    for x in xrange(1, 10):
        tcpconnect(sname, "localhost", getenv("session.modclient.to"), None)

def closed(sock):
    if sock.user != None:
        sock.user.cancel()

def connected(sock, addr, port):
    log.info("client established, starting timer")
    sock.user = Client(sock.sname, sock.id)

def data(sock, buf):
    log.info("received by client: %s" % buf)

def expire(timer, ms):
    return timer.user.expire()
