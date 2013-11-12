package 
{
    import flash.net.*;
    import flash.utils.*;
    import flash.events.*;
    import flash.system.Security;

    public class NetAPI {
    
        private var host:String = "localhost";
        private var port:int = 80;
        public var sock:Socket;
        public var buffer:String;
     
        public function NetAPI()
        {
            Log.trace("NetAPI::NetAPI");

            sock = new Socket();
            sock.addEventListener(Event.CONNECT, connectHandler);
            sock.addEventListener(Event.CLOSE, closeHandler);
            sock.addEventListener(ProgressEvent.SOCKET_DATA, socketDataHandler);
            sock.addEventListener(Event.COMPLETE, completeHandler);
            
            Security.loadPolicyFile("http://"+host+":"+port+"/crossdomain.xml");
     
            sock.addEventListener(IOErrorEvent.IO_ERROR, function(e:IOErrorEvent):void{ Log.trace("ioerr"); } );
     
            sock.connect(host,port);
        }
        
        //完毕
        public function completeHandler(event:Event):void {
            Log.trace("completeHandler:"+event.target.data);
        }
        
        //关闭
        public function closeHandler(event:Event):void {
            Log.trace("close handler");
        }
     
        public function connectHandler(event:Event):void {
            trace("Connect...Ok");
            //设置Http请求头，其他协议的请求亦可
            var header:String = "GET /hls.m3u8 HTTP/1.1 \r\n";
            header += "Accept: */* \r\n";
            header += "Accept-Language: zh-cn \r\n";
            header += "User-Agent: Mozilla/4.0 \r\n";
            header += "Host: " + host + ":" + port + " \r\n";
            header += "Connection: Keep-Alive \r\n";
            header += "Cache-Control: no-cache \r\n\r\n";
            //写入请求头
            sock.writeUTFBytes(header);
            //清空缓冲区，发送请求
            sock.flush();
        }
        
        //收到数据
        public function socketDataHandler(event:ProgressEvent):void {
            Log.trace("socketDataHandler:"+event.bytesLoaded);
            var data:String = sock.readUTFBytes(sock.bytesAvailable);
            buffer += data;

            Log.trace("Data:", buffer);
        }
    }
}