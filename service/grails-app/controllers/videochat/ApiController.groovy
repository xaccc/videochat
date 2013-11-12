package videochat

class ApiController {

    def index() { }
    
    // 需要实现的接口：
    // 1、为当前用户分配一个直播地址。(使用Cookie来区别当前用户，Cookie名为"UID")
    // 2、获取指定用户的直播地址。
    // 3、获取指定用户是否正在直播。

    def publishUrl(String uid) {
        render(contentType: "application/json") {
            Method = "publishUrl"
            UID = uid
            ServiceURI = "rtmp://127.0.0.1/live/$uid"
        }
    }
    
    def liveUrl(String uid) {
        render(contentType: "application/json") {
            Method = "liveUrl"
            UID = uid
        }
    }
    
    def liveStatus(String uid) {
        render(contentType: "application/json") {
            Method = "liveStatus"
            UID = uid
        }
    }
    
}
