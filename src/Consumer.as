package
{
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.NetStatusEvent;
	import flash.net.NetConnection;
	import flash.net.NetStream;
	import flash.media.Video;
	
	public class Consumer extends Sprite
	{
		private var inited:Boolean = false;
		private var rtmpURL:String;
		private var nc:NetConnection;
		private var host:String = "localhost";
		private var appName:String = "live";
		private var nsIn:NetStream;
		private var vidStream:Video;
		
		public function Consumer()
		{
			this.addEventListener(Event.ADDED, onAdded);
		}
		
		protected function onAdded(event:Event):void
		{
			if (inited) return;
			inited = true;
			
			trace("Width: "+stage.stageWidth+"; Height: "+stage.stageHeight);
			
			if (root.loaderInfo.parameters["host"]) {
				host = stage.loaderInfo.parameters["host"]
			}
			if (root.loaderInfo.parameters["appName"]) {
				appName = stage.loaderInfo.parameters["appName"]
			}
			
			rtmpURL = "rtmp://" + host + "/" + appName;
			
			trace("connect " + rtmpURL);
			
			nc = new NetConnection();
			nc.addEventListener(NetStatusEvent.NET_STATUS, checkCon);
			nc.client = this;
			
			nc.connect(rtmpURL);
						
			
		}
		
		// bandwidth detection on the server
		public function onBWCheck(...arg):Number {
			return 0;
		}
		public function onBWDone(...arg):void {
			var p_bw:Number = -1; 
			if (arg.length > 0) p_bw = arg[0]; 
			Log.trace("bandwidth = ", p_bw, " Kbps."); 
		}
		
		protected function checkCon(event:NetStatusEvent):void
		{
			
			var connected:Boolean = event.info.code == "NetConnection.Connect.Success";
			
			if (connected) {
				setVideo();
			}
			
			trace("NetStatus: " + event.info.code);
		}
		
		private function setVideo():void
		{
			var w:Number = stage.stageWidth;
			var h:Number = 3 / 4 * stage.stageWidth;
			
			 nsIn = new NetStream(nc);
			 nsIn.play("user1");
			 vidStream = new Video(w, h);
			 vidStream.x = 0;
			 vidStream.y = (stage.stageHeight - h) / 2;
			 vidStream.attachNetStream(nsIn);
			 addChild(vidStream);
		}
	}
}