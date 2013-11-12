package 
{
    import flash.utils.*;
	import flash.external.ExternalInterface;

    public class HTTPCookie {

        static public function getCookies():Dictionary {
            var cookie:String;
            var result:Dictionary = new Dictionary();
            
            try{
                cookie = ExternalInterface.call("eval","document.cookie");
                
                var cookieList:Array = cookie.split(";");
                for(var i:int = 0; i < cookieList.length; i++) {
                    var keyvalue:Array = cookieList[i].split("=");
                    if (keyvalue[0]) {
                        result[keyvalue[0]] = keyvalue[1];
                        Log.trace("Cookie: ", keyvalue[0], "=", keyvalue[1]);
                    }
                }
                
            } catch (e:Error) {
            }
            
            return result;
        }
        
    }
}