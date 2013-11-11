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
        private var good:Boolean;
        private var rtmpNow:String;
        private var nsIn:NetStream;
        private var nsOut:NetStream;
        private var cam:Camera;
        private var mic:Microphone;
        private var vidLocal:Video;
        private var vidStream:Video;
        private var cameraSettings:H264VideoStreamSettings;
        
        public function Publisher() {
            trace("Hello testing");
            rtmpNow = "rtmp://www.wenxi.gov.cn/live";
            nc = new NetConnection();
            nc.connect(rtmpNow);
            nc.addEventListener(NetStatusEvent.NET_STATUS,checkCon);
            setCam();
            setMic();
            setVideo();
        }
        
        private function checkCon(e:NetStatusEvent):void {
            good = e.info.code == "NetConnection.Connect.Success";
            if (good) {
                nsOut = new NetStream(nc);
                cameraSettings = new H264VideoStreamSettings(); 
                cameraSettings.setProfileLevel(H264Profile.BASELINE, H264Level.LEVEL_1_2);
                nsOut.videoStreamSettings = cameraSettings;
                nsOut.attachAudio(mic);
                nsOut.attachCamera(cam);
                nsOut.publish("user1", "live");
                
                nsIn = new NetStream(nc);
                nsIn.play("user1");
                vidStream.attachNetStream(nsIn);
            }
            
            trace("NetStatus: " + e.info.code);
        }
        
        private function setCam():void {        
            cam = Camera.getCamera();
            cam.setKeyFrameInterval(5);
            cam.setMode(160,120,24);
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
            
            vidStream = new Video(cam.width,cam.height);
            vidStream.x = vidLocal.x + cam.width + 10;
            vidStream.y = vidLocal.y;
            addChild(vidStream);
            
        }
    }
}