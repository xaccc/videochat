package
{
    import flash.utils.*;
	import flash.external.ExternalInterface;

	final public class Log
	{
		public function Log()
		{
		}
		
		static public function trace(...args):void {
			try {
				var msg:String = "";
				for (var i:int = 0; i < args.length; i++) {
					msg += args[i];
				}
				ExternalInterface.call("console.log", msg);
			} catch(e:Error)  {			
			}
		}
        
        static public function sleep(counter:int, subsequentFunction:Function, args:Array): void {
            if (counter > 0)
                callLater(sleep, [counter - 1, subsequentFunction, args]);
            else
                callLater(subsequentFunction, args);
        }        
	}
}