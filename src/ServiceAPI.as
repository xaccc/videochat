package 
{
    import flash.net.*;
    import flash.utils.*;
    import flash.events.Event;
    import flash.system.Security;

    public class ServiceAPI {
    
        private var remote_scheme:String = "http";
        private var remote_host:String = "localhost";
        private var remote_port:int = 80;
        private var remote_app:String = "/videochat"

        private var request_busy:Boolean = false; // smp
        private var request_function:String;
        private var response_data:String;
        private var response_succeeded:Boolean = false;
        private var response_callback:Function = null;

        public function ServiceAPI(cb:Function, host:String = "localhost", port:int = 80, app:String = "/videochat"):void {
            remote_host = host;
            remote_port = port;
            remote_app = app;
            response_callback = cb;
        }
        
        public function publishUrl(uid:String):void {
            httpRequest("publishUrl", "/api/publishUrl", {"uid":uid});
        }
        
        public function liveUrl(uid:String):void {
            httpRequest("liveUrl", "/api/liveUrl", {"uid":uid});
        }
        
        public function liveStatus(uid:String):void {
            httpRequest("liveStatus", "/api/liveStatus", {"uid":uid});
        }
        
        private function httpRequest(functionName:String, path:String, params:Object):void {
            // smp
            if (request_busy) {
                Log.trace("http request busy!");
                return;
            }
            request_function = functionName;
            request_busy = true;
            response_succeeded = false;
            
            var url:String = remote_scheme + "://" + remote_host + ":" + remote_port + remote_app + path + '?' + Math.random();
            var request:URLRequest = new URLRequest(url);
            
            var variables:URLVariables = new URLVariables();
            for(var prop:String in params)
                variables[prop] = params[prop];
                
            request.data = variables;
            request.method = URLRequestMethod.POST;

            var urlLoader:URLLoader = new URLLoader();
            urlLoader.addEventListener(Event.COMPLETE, urlLoader_complete);
            
            try
            {
                urlLoader.load(request);
            }
            catch (error:Error)
            {
                // smp
                response_succeeded = false;
                request_busy = false;
                
                Log.trace("URL Load Error: ", error.message, "(", error.errorID, ")");
            }
        }
        
        private function urlLoader_complete(event:Event):void {
        
            var loader:URLLoader = URLLoader(event.target);
            response_data = loader.data;
            
            // smp
            response_succeeded = true;
            request_busy = false;

            try {
                response_callback.call(null, request_function, JSON.parse(response_data));
            } catch (error:Error) {
                Log.trace("response callback error: ", error.message, ", response text: ", response_data);
            }
            
        }
        
    }
}