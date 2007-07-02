require 'augutil'
require 'log'

include AugUtil

# tcplisten(), tcpconnect()

module RbTest5
    def RbTest5.stop
        Log.info("stop()")
    end
    def RbTest5.start(sname)
        Log.debug("start(): #{sname}")
        @listener = AugRt.tcplisten("0.0.0.0", "1234", nil)
        @client = AugRt.tcpconnect("127.0.0.1", "1234", nil)
        @server = nil
    end
    def RbTest5.closed(sock)
        Log.debug("closed(): #{sock}")
    end
    def RbTest5.accepted(sock, addr, port)
        Log.debug("accepted(): #{sock}")
        sock.user = LineParser.new()
        @server = sock
    end
    def RbTest5.connected(sock, addr, port)
        Log.debug("connected(): #{sock}")
        sock.user = LineParser.new()
        AugRt.send(@server, "hello, world!\n")
    end
    def RbTest5.data(sock, buf)
        Log.debug("data(): #{sock}")
        sock.user.parse(buf) do |line|

            Log.debug("line: #{line}")
            if line != "hello, world!"
                Log.error("unexpected line from data()")
            end

            if sock == @server
                AugRt.send(sock, line + "\n")
            else
                AugRt.stopall
            end
        end
    end
end