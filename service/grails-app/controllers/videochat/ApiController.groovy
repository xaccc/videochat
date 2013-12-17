package videochat

import java.util.*;
import java.net.URI;
import java.util.concurrent.*;

import org.apache.http.client.fluent.Async;
import org.apache.http.client.fluent.Content;
import org.apache.http.client.fluent.Request;


class ApiController {

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
    def postLog() {
        //
        def tp = params['type']
        def ev = params['event']
        def ts = params['timestamp']
        def sn = params['name']
        
        if(tp == 'stream') {
            switch(ev) {
            case 'broadcastStart':
                def online = Online.findBySessionId(sn?:'')
                def uid = online?online.uid:''
                _postLog(new StringBuffer('op=hostLive&room_id=').append(uid).append('&timestamp=').append(ts).append('&Version=video'));
                break;
            case 'broadcastClose':
                def online = Online.findBySessionId(sn?:'')
                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }

                def uid = online?online.uid:''
                online?.delete(flush:true);
                _postLog(new StringBuffer('op=hostOffLive&room_id=').append(uid).append('&timestamp=').append(ts).append('&Version=video'));
                break;
                
            case 'backup':
                def flvid = params['flvid']; // 文件ID
                def first = params['first']; // 是否上麦第一次备份
                def online = Online.findBySessionId(sn?:'')
                
                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }
                def mediaService = MediaService.get(online?online.mediaServiceId:'')
                new MediaBackup(
                    uid: online?online.uid:'-',
                    domain: mediaService?mediaService.domain:'-',
                    sessionId: online?online.sessionId:'-',
                    flvid: flvid,
                    first: first.toBoolean(),
                    backupDate: new Date(ts.toLong())
                    ).save(flush:true);
                break;
            }
        }
        
        render(contentType: "application/json") {
            result = "OK"
        }
    }

    private static ExecutorService threadpool = Executors.newFixedThreadPool(2);
    private static Async async = Async.newInstance().use(threadpool);
    
    private void _postLog(StringBuffer query) {
        try {
            Request request = Request.Get(new URI("http", null, 
                grailsApplication.config.postLog.Host, 
                grailsApplication.config.postLog.Port, 
                 grailsApplication.config.postLog.Path, 
                query.toString(), null));
            Future<Content> future = async.execute(request);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
    
}
