package
{
    import flash.utils.*;
	import flash.external.ExternalInterface;

	final public class Log
	{
		function Log()
		{
		}
		
		static public function trace(...args):void {
			try {
				var msg:String = "[VIDEOCHAT] ";
				for (var i:int = 0; i < args.length; i++) {
					msg += args[i];
				}
				ExternalInterface.call("console.log", msg);
			} catch(e:Error)  {
			}
		}
        
	}
}