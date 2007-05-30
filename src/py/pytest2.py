from augrt import *
from cgi import *
from urllib import *
import log

# post(), dispatch()

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    dispatch("group1", "foo", str(101))
    post("group2", "application/x-www-form-urlencoded", urlencode({"a": 1}))
    post(sname, "none", None)

def event(frm, type, user):
    log.debug("event(): %s" % user)
    if type == "foo":
        if int(user) != 101:
            log.error("unexpected user data")
        dispatch(frm, "bar", buffer("202"))
    elif type == "bar":
        if int(user) != 202:
            log.error("unexpected user data")
    elif type == "application/x-www-form-urlencoded":
        if int(parse_qs(user)["a"][0]) != 1:
            log.error("unexpected user data")
    elif type == "none":
        if user is not None:
            log.error("unexpected user data")
        stopall()
    else:
        log.error("unexpected type")
