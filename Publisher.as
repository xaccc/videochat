package {

    import flash.display.Sprite;
    import flash.events.NetStatusEvent;
    import flash.net.NetConnection;
    import flash.net.NetStream;
    import flash.media.Camera;
    import flash.media.Microphone;
    import flash.media.Video;
    import flash.media.H264Level;
    import flash.media.H264Profile;
    import flash.media.H264VideoStreamSettings;

    public class Publisher extends Sprite {
    
        private var nc:NetConnection;
        private var rtmpNow:String;
        private var nsIn:NetStream;
        private var nsOut:NetStream;
        private var cam:Camera;
        private var mic:Microphone;
        private var vidLocal:Video;
        private var vidStream:Video;
        private var cameraSettings:H264VideoStreamSettings;
        
        public function Publisher() {
            trace("Stage Width: " + this.stage.width);
            trace("Stage Height:" + this.stage.height);
            
            rtmpNow = "rtmp://localhost/live";
            
            nc = new NetConnection();
            nc.addEventListener(NetStatusEvent.NET_STATUS, checkCon);
            nc.client = this;
            
            nc.connect(rtmpNow);

            setCam();
            setMic();
        }
        
        // bandwidth detection on the server
        public function onBWCheck(...arg):Number {
            return 0;
        }
        public function onBWDone(...arg):void {
            var p_bw:Number = -1; 
            if (arg.length > 0) p_bw = arg[0]; 
            trace("bandwidth = " + p_bw + " Kbps."); 
        }
        
        private function checkCon(e:NetStatusEvent):void {
        
            var connected:Boolean = e.info.code == "NetConnection.Connect.Success";
        
            if (connected) {
                nsOut = new NetStream(nc);
                
                // H.264 codec setting
                cameraSettings = new H264VideoStreamSettings(); 
                cameraSettings.setProfileLevel(H264Profile.BASELINE, H264Level.LEVEL_1_2);
                nsOut.videoStreamSettings = cameraSettings;
                
                nsOut.attachAudio(mic);
                nsOut.attachCamera(cam);
                nsOut.publish("user1", "live");
                
                setVideo();
            }
            
            trace("NetStatus: " + e.info.code);
        }
        
        private function setCam():void {
        
            cam = Camera.getCamera();
            cam.setKeyFrameInterval(10);
            cam.setMode(320,240,24);
            cam.setQuality(0,85);
            
        }
        
        private function setMic():void {
        
            mic = Microphone.getMicrophone();
            mic.codec = "Speex";
            mic.encodeQuality = 6;
            mic.gain = 85;
            mic.rate = 11;
            mic.setSilenceLevel(15,2000);
            
        }

        private function setVideo():void {
        
            vidLocal = new Video(cam.width,cam.height);
            vidLocal.x = 0;
            vidLocal.y = 0;
            vidLocal.attachCamera(cam);
            addChild(vidLocal);
            
            nsIn = new NetStream(nc);
            nsIn.play("user1");
            vidStream = new Video(cam.width,cam.height);
            vidStream.x = vidLocal.x + cam.width + 10;
            vidStream.y = vidLocal.y;
            vidStream.attachNetStream(nsIn);
            addChild(vidStream);
            
        }
    }
}