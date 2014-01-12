package {
    
    import flash.display.Sprite;
    import flash.media.Camera;
    import flash.media.H264Level;
    import flash.media.H264Profile;
    import flash.media.H264VideoStreamSettings;
    import flash.media.Microphone;
    import flash.media.Video;
    import flash.net.NetConnection;
    import flash.net.NetStream;
    import flash.text.TextField;
    import flash.text.TextFieldAutoSize;
    import flash.text.TextFormat;
    import flash.filters.DropShadowFilter;
    import flash.utils.*;
    import flash.events.*;
    
    import flash.system.Security;
    import flash.system.SecurityPanel;
    
    import mx.core.UIComponent;
    
    public class Producer extends Sprite {
        
        private var rtmpURL:String;
        private var rtmpInstance:String;
        private var nc:NetConnection;
        private var nsOut:NetStream;
        private var cam:Camera;
        private var mic:Microphone;
        private var vidLocal:Video;
        private var cameraSettings:H264VideoStreamSettings;
        private var established:Boolean = false;
        private var minuteTimer:Timer = new Timer(1000, 0);
        private var fieldSpeed:TextField = new TextField();
        
        private var remote_host:String = "rm.boboxiu.tv";
        private var remote_port:int = 80;
        private var remote_app:String = "/videochat"
        
        private var uid:String;
        private var myuid:String;
        
        private var api:ServiceAPI;

        public function Producer() {

            //if (root.loaderInfo.parameters["host"])
            //    remote_host = stage.loaderInfo.parameters["host"];
            //if (root.loaderInfo.parameters["port"])
            //    remote_port = parseInt(stage.loaderInfo.parameters["port"]);
            if (root.loaderInfo.parameters["uid"])
                uid = stage.loaderInfo.parameters["uid"];
            //if (root.loaderInfo.parameters["app"])
            //    remote_app = stage.loaderInfo.parameters["app"];
            
            init();
            
            if (api == null)
                api = new ServiceAPI(apiCallback, remote_host, remote_port, remote_app);

            Log.trace("Host = ", remote_host, ", Port = ", remote_port, ", App = ", remote_app);
            
            if (remote_host == "rm.boboxiu.tv") {
                api.publishUrl(uid);
            }

            minuteTimer.addEventListener(TimerEvent.TIMER, onTick);
        }

        public function onTick(event:TimerEvent):void {
            if(nc != null && nc.connected) {
                if(nsOut.info.currentBytesPerSecond > int(150*1024/8)) {
                    fieldSpeed.textColor = 0x00FF00;
                } else if(nsOut.info.currentBytesPerSecond > int(120*1024/8)) {
                    fieldSpeed.textColor = 0xFF8800;
                } else {
                    fieldSpeed.textColor = 0xFF0000;
                }

                fieldSpeed.text = int(nsOut.info.currentBytesPerSecond*8/1024) + "kbps";
            } else if (established){
                established = false;
                fieldSpeed.textColor = 0xFF0000;
                fieldSpeed.text = "重新连接网络...";
                nc.connect(rtmpURL, myuid);
            }

            /*
            Log.trace("==============================================");
            Log.trace("currentBytesPerSecond = ", int(nsOut.info.currentBytesPerSecond*8/1024));
            Log.trace('connected = ', nc.connected);
            Log.trace('established = ', established);
            Log.trace('dataBytesPerSecond = ', nsOut.info.dataBytesPerSecond);
            Log.trace('videoLossRate = ', nsOut.info.videoLossRate);
            Log.trace('audioLossRate = ', nsOut.info.audioLossRate);           
            */ 
        }
        
        public function apiCallback(funcName:String, response:Object):void {
            if (funcName == "publishUrl") {
            
                var idx:int = response.ServiceURI ? response.ServiceURI.lastIndexOf("/") : -1;
                
                if (idx >= 0) {
                    rtmpURL = response.ServiceURI.substr(0, idx);
                    rtmpInstance = response.ServiceURI.substr(idx+1);
                } else if (response.Host) {
                    rtmpURL = "rtmp://" + response.Host 
                              + ":" + response.Port 
                              + "/" + response.Application;
                              
                    rtmpInstance = response.Session;
                    //Log.trace(rtmpURL);
                } else {
                    throw new Error("服务通讯故障");
                    Log.trace("服务通讯故障");
                }
                    

                // connect media server
                if (nc == null) {
                    nc = new NetConnection();
                    nc.addEventListener(NetStatusEvent.NET_STATUS, checkCon);
                    nc.client = this;
                }
                
                myuid = "UID="+HTTPCookie.getUID();
                nc.connect(rtmpURL, myuid);
                
            }
        }
        
        private function init():void {
            // init media device
            setCam();
            setMic();
            setVideo();
        
            // info text
            var myformat:TextFormat = new TextFormat();
            myformat.size = 18;
            myformat.align="right";
            myformat.bold = true;
            myformat.font = "Arial";
            
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
        
        private function checkCon(event:NetStatusEvent):void {
            var connected:Boolean = event.info.code == "NetConnection.Connect.Success";
            
            if (connected) {
                if (nsOut == null)
                    nsOut = new NetStream(nc);
                
                // H.264 codec setting
                if (cameraSettings == null)
                    cameraSettings = new H264VideoStreamSettings(); 
                    
                cameraSettings.setProfileLevel(H264Profile.BASELINE, H264Level.LEVEL_3_1);
                nsOut.videoStreamSettings = cameraSettings;
                
                nsOut.attachAudio(mic);
                nsOut.attachCamera(cam);
                nsOut.publish(rtmpInstance, "live");
                
                established = true;
                minuteTimer.start();
            } else if (established && event.info.code == "NetConnection.Connect.Closed") {
            } else if (established && event.info.code == "NetConnection.Connect.NetworkChange") {
            } else if (established && event.info.code == "NetConnection.Connect.Failed") {
            }
            
            Log.trace("NetStatus: ", event.info.code);
        }
        
        private function setCam():void {
            cam = Camera.getCamera();
            // Security.showSettings(SecurityPanel.CAMERA);
            cam.setKeyFrameInterval(5);
            cam.setMode(320,240,10);
            //cam.setMode(320,240,15);
            cam.setQuality(22000,0); // Bytes per second
        }
        
        private function setMic():void {
            mic = Microphone.getMicrophone();
            // Security.showSettings(SecurityPanel.MICROPHONE);
            mic.codec = "Speex";
            mic.encodeQuality = 8;
            mic.noiseSuppressionLevel = 0;
            mic.gain = 45;
            //mic.rate = 11;
            mic.setSilenceLevel(0, 0);
            //mic.framesPerPacket = 3;
            mic.setUseEchoSuppression(false);
            mic.enableVAD = false;
        }
        
        private function setVideo():void {
            var w:Number = stage.stageWidth;
            var h:Number = cam.height / cam.width * stage.stageWidth;
            
            if (vidLocal == null)
                vidLocal = new Video(w,h);
            vidLocal.smoothing = true;
            vidLocal.x = 0;
            vidLocal.y = (stage.stageHeight - h) / 2;
            vidLocal.attachCamera(cam);
            addChild(vidLocal);
        }
    }
}