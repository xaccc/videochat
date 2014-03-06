package
{
    import flash.display.Sprite;
    import flash.net.NetConnection;
    import flash.net.NetStream;
    import flash.media.Video;
    import flash.utils.*;
    import flash.events.*;
    
    import flash.text.TextField;
    import flash.text.TextFieldAutoSize;
    import flash.text.TextFormat;
    import flash.filters.DropShadowFilter;

    public class Consumer extends Sprite
    {
        private var rtmpURL:String;
        private var rtmpInstance:String;
        private var nc:NetConnection = null;
        private var nsIn:NetStream;
        private var vidStream:Video;
        private var established:Boolean = false;
        private var minuteTimer:Timer = new Timer(1000, 0);
        private var fieldSpeed:TextField = new TextField();
        
        private var remote_host:String = "rm.boboxiu.tv";
        private var remote_port:int = 80;
        private var remote_app:String = "/videochat"
        
        private var uid:String;
        private var myuid:String;
        
        private var api:ServiceAPI;

        public function Consumer()
        {
            //if (root.loaderInfo.parameters["host"])
            //    remote_host = stage.loaderInfo.parameters["host"];
            if (root.loaderInfo.parameters["port"])
                remote_port = parseInt(stage.loaderInfo.parameters["port"]);
            if (root.loaderInfo.parameters["uid"])
                uid = stage.loaderInfo.parameters["uid"];
            if (root.loaderInfo.parameters["app"])
                remote_app = stage.loaderInfo.parameters["app"];
            
            init();
            
            if (api == null)
                api = new ServiceAPI(apiCallback, remote_host, remote_port, remote_app);
            
            //Log.trace("Host = ", remote_host, ", Port = ", remote_port, ", App = ", remote_app);
            
            if (remote_host == "rm.boboxiu.tv") {
                api.liveUrl(uid);
            }

            minuteTimer.addEventListener(TimerEvent.TIMER, onTick);
            minuteTimer.start();
        }
        
        private var zeroTimes:int = 0;
        public function onTick(event:TimerEvent):void {
            if (zeroTimes > 5) {
                nc.close();
                zeroTimes = 0;
            }

            if(nc != null && nc.connected) {
                if(nsIn.info.currentBytesPerSecond > int(150*1024/8)) {
                    fieldSpeed.textColor = 0x00FF00;
                    fieldSpeed.filters[0].color = 0xFFFFFF;
                    fieldSpeed.text = "网络良好";
                } else if(nsIn.info.currentBytesPerSecond > int(120*1024/8)) {
                    fieldSpeed.textColor = 0xFF8800;
                    fieldSpeed.filters[0].color = 0xFFFFFF;
                    fieldSpeed.text = "网络一般";
                } else {
                    fieldSpeed.textColor = 0xFF0000;
                    fieldSpeed.filters[0].color = 0xFFFFFF;
                    fieldSpeed.text = "网络质量差";
                }

                if (nsIn.info.currentBytesPerSecond == 0)
                    zeroTimes ++;
                else
                    zeroTimes = 0;
                
            } else if (established){
                established = false;
                fieldSpeed.textColor = 0xFF0000;
                fieldSpeed.text = "重新连接网络...";
                nc = null;
                nsIn = null;
                api.liveUrl(uid);
            }

            /*
            Log.trace("==============================================");
            Log.trace("currentBytesPerSecond = ", int(nsIn.info.currentBytesPerSecond*8/1024));
            Log.trace("connected = ", nc.connected);
            Log.trace("established = ", established);
            Log.trace("dataBytesPerSecond = ", nsIn.info.dataBytesPerSecond);
            Log.trace("videoLossRate = ", nsIn.info.videoLossRate);
            Log.trace("audioLossRate = ", nsIn.info.audioLossRate);
            */
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
                
                if (response.Session) {
                    //Log.trace('URL: ',rtmpURL,'/',rtmpInstance);
                    //myuid = "UID="+HTTPCookie.getUID();
                    nc.connect(rtmpURL, myuid);
                } else {
                    fieldSpeed.text = "主播不在线";
                }
            }
        }

        protected function init():void
        {
            setVideo();

            // info text
            var myformat:TextFormat = new TextFormat();
            myformat.size = 18;
            myformat.align="right";
            myformat.bold = true;
            myformat.font = "微软雅黑";
            
            fieldSpeed.x = 0;
            fieldSpeed.y = 10;
            fieldSpeed.width = stage.stageWidth - 10;
            fieldSpeed.visible = true;
            fieldSpeed.text = "";
            fieldSpeed.textColor = 0x00FF00; 
            fieldSpeed.selectable = false;
            fieldSpeed.autoSize = TextFieldAutoSize.RIGHT;
            fieldSpeed.defaultTextFormat = myformat;
            fieldSpeed.filters = [new DropShadowFilter()];
            fieldSpeed.filters[0].color = 0xFFFFFF;
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
                nsIn.backBufferTime = 2;
                nsIn.bufferTime = 2;
                nsIn.play(rtmpInstance);
                vidStream.attachNetStream(nsIn);
                established = true;
            } else if (established && event.info.code == "NetConnection.Connect.Closed") {
            } else if (established && event.info.code == "NetConnection.Connect.NetworkChange") {
            } else if (established && event.info.code == "NetConnection.Connect.Failed") {
            }
            
            trace("NetStatus: " + event.info.code);
        }
        
        private function setVideo():void
        {
            var w:Number = stage.stageWidth;
            var h:Number = 3 / 4 * stage.stageWidth;
            
            if (vidStream == null)
                vidStream = new Video(w, h);
            vidStream.smoothing = true;
            vidStream.x = 0;
            vidStream.y = (stage.stageHeight - h) / 2;
            addChild(vidStream);
        }
    }
}