package {
    
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.events.NetStatusEvent;
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
        
        private var remote_host:String = "localhost";
        private var remote_port:int = 80;
        private var remote_app:String = "/videochat"
        
        private var uid:String;
        
        private var api:ServiceAPI;

        public function Producer() {

            if (root.loaderInfo.parameters["host"])
                remote_host = stage.loaderInfo.parameters["host"];
            if (root.loaderInfo.parameters["port"])
                remote_port = parseInt(stage.loaderInfo.parameters["port"]);
            if (root.loaderInfo.parameters["uid"])
                uid = stage.loaderInfo.parameters["uid"];
            if (root.loaderInfo.parameters["app"])
                remote_app = stage.loaderInfo.parameters["app"];
            
            init();
            
            if (api == null)
                api = new ServiceAPI(apiCallback, remote_host, remote_port, remote_app);

            Log.trace("Host = ", remote_host, ", Port = ", remote_port, ", App = ", remote_app);
            
            if (remote_host == "183.203.16.207") {
                api.publishUrl(uid);
            }
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
                } else {
                    throw new Error("服务通讯故障");
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
        
        private function init():void {
            // init media device
            setCam();
            setMic();
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
            fieldSpeed.text = "本地视频";
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
            
            Log.trace("NetStatus: ", event.info.code);
        }
        
        private function setCam():void {
            cam = Camera.getCamera();
            Security.showSettings(SecurityPanel.CAMERA);
            cam.setKeyFrameInterval(5);
            cam.setMode(320,240,25);
            cam.setQuality(0,90);
        }
        
        private function setMic():void {
            mic = Microphone.getMicrophone();
            Security.showSettings(SecurityPanel.MICROPHONE);
            mic.codec = "Speex";
            mic.encodeQuality = 8;
            mic.gain = 85;
            mic.rate = 11;
            mic.setSilenceLevel(0, -1);
            mic.framesPerPacket = 3;
            mic.setUseEchoSuppression(false);
            mic.enableVAD = true;
        }
        
        private function setVideo():void {
            var w:Number = stage.stageWidth;
            var h:Number = cam.height / cam.width * stage.stageWidth;
            
            if (vidLocal == null)
                vidLocal = new Video(w,h);
            vidLocal.x = 0;
            vidLocal.y = (stage.stageHeight - h) / 2;
            vidLocal.attachCamera(cam);
            addChild(vidLocal);
        }
    }
}