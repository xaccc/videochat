package videochat

class ApiController {

    def postLogService

    def index() {
        render(contentType: "application/json") {
            ERROR = "调用错误"
        }
    }
    
    // 需要实现的接口：
    // 1、为当前用户分配一个直播地址。(使用Cookie来区别当前用户，Cookie名为"UID")
    // 2、获取指定用户的直播地址。
    // 3、获取指定用户是否正在直播。

    //
    // 为用户分配一个直播服务器
    //
    def publishUrl(String uid) {
        def online = Online.findByUid(uid);
        
        if (!online) {
            // apply mediaService
            def mediaServicesCount = MediaService.count();
            def mediaServicesOffset = mediaServicesCount > 1 ? new Random().nextInt(mediaServicesCount-1) : 0;
            def mediaServices = mediaServicesCount > 0 ? MediaService.list(offset:mediaServicesOffset, max:1)
                                                       : [];
            
            if (mediaServices.size() > 0) {
                def sessionId = uid.toLowerCase() + '_' + Long.toHexString(System.nanoTime()).toLowerCase();
                def server = mediaServices[0]
                online = new Online(
                                uid: uid, 
                                mediaServiceId: server.id, 
                                sessionId: sessionId);
                
                online.save(flush:true);

                render(contentType: "application/json") {
                    Method = "publishUrl"
                    UID = uid
                    Host = server.domain        // 183.203.16.207
                    Port = server.port          // 8100
                    Application = server.path   // live
                    Session = online.sessionId  // uid.encodeAsMD5()
                }
                
            } else {
                // havn't media server
                render(contentType: "application/json") {
                    ERROR = "没有找到任何可分配的媒体服务器"
                }
            }
        } else {
            // online
            def server = MediaService.get(online.mediaServiceId);
            render(contentType: "application/json") {
                Method = "publishUrl"
                UID = uid
                Host = server.domain        // 183.203.16.207
                Port = server.port          // 8100
                Application = server.path   // live
                Session = online.sessionId  // uid.encodeAsMD5()
            }
        }
    }
    
    //
    // 获取用户直播服务信息
    //
    def liveUrl(String uid) {
        def online = Online.findByUid(uid);
        
        if (online) {

            // online
            def server = MediaService.get(online.mediaServiceId);
            render(contentType: "application/json") {
                Method = "liveUrl"
                UID = uid
                Host = server.domain        // 183.203.16.207
                Port = server.port          // 8100
                Application = server.path   // live
                Session = online.sessionId  // uid.encodeAsMD5()
            }

        } else {
            // offline
            render(contentType: "application/json") {
                ERROR = "未进行直播"
            }
        }
    }
    
    //
    // 获取用户直播状态
    //
    def liveStatus(String uid) {
        def online = Online.findByUid(uid);
        render(contentType: "application/json") {
            Method = "liveStatus"
            UID = uid
            Status = (online ? 'online' : 'offline')
        }
    }
    
    //
    // 记录
    //
    def postlog() {
        postLogService.postLog(params);
        render(contentType: "application/json") {
            Return = "OK"
        }
    }
}
