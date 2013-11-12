package 
{
    import flash.net.*;
    import flash.utils.*;
    import flash.events.Event;
    import flash.system.Security;


    public class ServiceAPI {
    
        private var host:String = "localhost";
        private var port:int = 80;

        public function ServiceAPI():void {
        }
        
        
        public function test():String {
            var params:Dictionary = new Dictionary();
            params.key1 = "value1";
            
            return httpRequest("/test.php", params);
        }
        
        
        private function httpRequest(path:String, params:Dictionary):String {
            var url:String = "http://" + host + ":" + port + path;
            var request:URLRequest = new URLRequest(url);
            var data:String;
            

            Log.trace("Load URL: ", url);
            
            var variables:URLVariables = new URLVariables();
            for(var prop:String in params)
                variables[prop] = params[prop];
            request.data = variables;
            request.method = URLRequestMethod.GET;

            var urlLoader:URLLoader = new URLLoader();
            urlLoader.addEventListener(Event.COMPLETE, urlLoader_complete);
            
            try
            {
                urlLoader.load(request);
            }
            catch (error:Error)
            {
                Log.trace("Unable to load URL: ", url);
            }

            return data;
        }
        
        private function urlLoader_complete(event:Event):void {
            var loader:URLLoader = URLLoader(event.target);
            Log.trace("Data: " + loader.data);
        }
        
    }
}