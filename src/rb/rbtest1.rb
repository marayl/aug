require 'log'

# stop(), Object()

module RbTest1
    def RbTest1.stop
        Log.debug("stop()")
    end
    def RbTest1.start(sname)
        Log.debug("start(): #{sname}")
        o = AugRt::Object.new(101, "our object")
        Log.debug("to_s(): #{o}")
        if o.id != 101
            Log.error("object returned unexpected id")
        end
        if o.user != "our object"
            Log.error("object returned unexpected user")
        end
        o.user = "new user"
        if o.user != "new user"
            Log.error("object returned unexpected user")
        end
        if o != AugRt::Object.new(101, nil)
            Log.error("comparison operator failed")
        end
        AugRt.stopall
    end
end