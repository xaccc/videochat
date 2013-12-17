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
    
    public class ProducerLocal extends Sprite {
        
        private var rtmpURL:String = "rtmp://localhost/videochat";
        private var rtmpInstance:String = "fuck";
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
        private var myuid:String;
        
        private var api:ServiceAPI;

        public function ProducerLocal() {


            init();
            
          
            
            rtmpInstance = "fuck";

            if (nc == null) {
                nc = new NetConnection();
                nc.addEventListener(NetStatusEvent.NET_STATUS, checkCon);
                nc.client = this;
            }
            
            nc.connect(rtmpURL, myuid);
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
            } else if (established && event.info.code == "NetConnection.Connect.Closed") {
                nc.connect(rtmpURL, myuid);
            } else if (established && event.info.code == "NetConnection.Connect.NetworkChange") {
            } else if (established && event.info.code == "NetConnection.Connect.Failed") {
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