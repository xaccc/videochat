package
{
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.events.NetStatusEvent;
    import flash.net.NetConnection;
    import flash.net.NetStream;
    import flash.media.Video;
    import flash.utils.*;
    
    import flash.text.TextField;
    import flash.text.TextFieldAutoSize;
    import flash.text.TextFormat;
    import flash.filters.DropShadowFilter;

    public class Consumer extends Sprite
    {
        private var rtmpURL:String;
        private var rtmpInstance:String;
        private var nc:NetConnection;
        private var nsIn:NetStream;
        private var vidStream:Video;
        private var established:Boolean = false;
        
        private var title:String = "直播";
        
        private var remote_host:String = "localhost";
        private var remote_port:int = 80;
        private var remote_app:String = "/videochat"
        
        private var uid:String;
        
        private var api:ServiceAPI;

        public function Consumer()
        {
            if (root.loaderInfo.parameters["host"])
                remote_host = stage.loaderInfo.parameters["host"];
            if (root.loaderInfo.parameters["port"])
                remote_port = parseInt(stage.loaderInfo.parameters["port"]);
            if (root.loaderInfo.parameters["uid"])
                uid = stage.loaderInfo.parameters["uid"];
            if (root.loaderInfo.parameters["app"])
                remote_app = stage.loaderInfo.parameters["app"];
            if (root.loaderInfo.parameters["name"])
                title = stage.loaderInfo.parameters["name"];
            
            init();
            
            if (api == null)
                api = new ServiceAPI(apiCallback, remote_host, remote_port, remote_app);
            
            Log.trace("Host = ", remote_host, ", Port = ", remote_port, ", App = ", remote_app);
            
            if (remote_host == "183.203.16.207") {
                api.liveUrl(uid);
            }
        }
        
        public function apiCallback(funcName:String, response:Object):void {
            if (funcName == "liveUrl") {
            
                var idx:int = response.LiveURI ? response.LiveURI.lastIndexOf("/") : -1;
                
                if (idx >= 0) {
                    rtmpURL = response.LiveURI.substr(0, idx);
                    rtmpInstance = response.LiveURI.substr(idx+1);
                } else {
                    rtmpURL = "rtmp://" + response.Host 
                              + ":" + response.Port 
                              + "/" + response.Application;
                              
                    rtmpInstance = response.Session;
                }
                    
                // connect media server
                if (nc == null) {
                    nc = new NetConnection();
                    nc.addEventListener(NetStatusEvent.NET_STATUS, checkCon);
                    nc.client = this;
                }
                
                nc.connect(rtmpURL);
                
            }
        }

        protected function init():void
        {
            setVideo();

            // info text
            var myformat:TextFormat = new TextFormat();
            myformat.size = 24;
            myformat.align="center";                
            myformat.font = "Wingdings";
            var fieldSpeed:TextField = new TextField();
            fieldSpeed.x = 5;
            fieldSpeed.y = 5;
            fieldSpeed.visible = true;
            fieldSpeed.text = title;
            fieldSpeed.textColor = 0x00FF00; 
            fieldSpeed.selectable = false;
            fieldSpeed.autoSize = TextFieldAutoSize.LEFT;
            fieldSpeed.setTextFormat(myformat);
            fieldSpeed.filters = [new DropShadowFilter()];
            fieldSpeed.filters[0].color = 0xFFFFFF
            addChild(fieldSpeed);
        }
        
        // bandwidth detection on the server
        public function onBWCheck(...arg):Number {
            return 0;
        }
        public function onBWDone(...arg):void {
            var p_bw:Number = -1; 
            if (arg.length > 0) p_bw = arg[0]; 
            // Log.trace("bandwidth = ", p_bw, " Kbps."); 
        }
        
        protected function checkCon(event:NetStatusEvent):void {
            var connected:Boolean = event.info.code == "NetConnection.Connect.Success";
            
            if (connected) {
                if (nsIn == null)
                    nsIn = new NetStream(nc);
                nsIn.play(rtmpInstance);
                vidStream.attachNetStream(nsIn);
                established = true;
            // } else if (established && event.info.code == "NetConnection.Connect.NetworkChange") {
                // // reconnect waitfor 1 second
                // setTimeout(nc.connect, 1000, rtmpURL);
            // } else if (established && event.info.code == "NetConnection.Connect.Closed") {
                // // reconnect waitfor 1 second
                // setTimeout(nc.connect, 1000, rtmpURL);
            // } else if (established && event.info.code == "NetConnection.Connect.Failed") {
                // // reconnect waitfor 1 second
                // setTimeout(nc.connect, 1000, rtmpURL);
            }
            
            trace("NetStatus: " + event.info.code);
        }
        
        private function setVideo():void
        {
            var w:Number = stage.stageWidth;
            var h:Number = 3 / 4 * stage.stageWidth;
            
            if (vidStream == null)
                vidStream = new Video(w, h);
            vidStream.x = 0;
            vidStream.y = (stage.stageHeight - h) / 2;
            addChild(vidStream);
        }
    }
}