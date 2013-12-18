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
    
    //
    // 为用户分配一个直播服务器
    //
    def publishUrl(String uid) {
        def online = Online.findByUid(uid);
    
        // apply mediaService
        def mediaServicesCount = MediaService.countByEnabled(true);
        def mediaServicesOffset = mediaServicesCount > 1 ? (int)(Math.abs(new Random().nextInt())%mediaServicesCount) : 0;
        def mediaServices = mediaServicesCount > 0 ? MediaService.findAllByEnabled(true, [offset:mediaServicesOffset, max:1])
                                                   : [];
        
        println "uid=$uid, mediaServicesCount=$mediaServicesCount, Random=${Math.abs(new Random().nextInt())}"
        if (mediaServices.size() > 0) {
            def sessionId = uid.toLowerCase() + '_' + Long.toHexString(System.nanoTime()).toLowerCase();
            def server = mediaServices[0]
            
            if (online) {
                // 存在上线记录，更新服务器
                online.mediaServiceId = server.id
                online.sessionId = sessionId // 更新会话ID
            } else {
                // 未上线，分配会话服务器
                online = new Online(
                                uid: uid, 
                                mediaServiceId: server.id, 
                                sessionId: sessionId);
            }
            
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
                ERROR = "没有找到任何可分配的流服务器"
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
                Method = "liveUrl"
                UID = uid
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
    // 获取用户直播录像
    //
    def recordLog(String uid) {
        params.login = params.login?params.login.toBoolean():true
        params.max = Math.max(100, params.max?params.max.toLong():10)
        def records = []
        
        records = params.login ? MediaBackup.findAllByFirstAndUid(true, uid, params)
                               : MediaBackup.findAllByUid(uid, params)
        
        render(contentType: "application/json") {
            Method = "recordLog"
            UID = uid
            Records = records.collect{rec -> [
                    'UUID': rec.id,
                    'IP': rec.domain,
                    'SID': rec.sessionId,
                    'FID': rec.flvid,
                    'Login': rec.first,
                    'Date': rec.backupDate
                    ]}
        }
    }
    

    //
    // 处理Log
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
                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }

                _postLog(new StringBuffer('op=hostLive&room_id=').append(online.uid).append('&timestamp=').append(ts).append('&Version=video'));
                
                new MediaEvent(uid: online.uid, 
                               sessionId: online.sessionId,
                               mediaServiceId: online.mediaServiceId,
                               dateLive: new Date(ts.toLong())).save(flush:true)
                break;

            case 'broadcastClose':
                def online = Online.findBySessionId(sn?:'')
                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }

                online.delete(flush:true);
                _postLog(new StringBuffer('op=hostOffLive&room_id=').append(online.uid).append('&timestamp=').append(ts).append('&Version=video'));
                
                def me = MediaEvent.findByUidAndSessionId(online.uid, online.sessionId)
                if (me) {
                    me.dateLeave = new Date(ts.toLong())
                    me.duration = (me.dateLeave.time - me.dateLive.time)/60000
                    me.save(flush:true)
                }
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
                    uid: online.uid,
                    domain: mediaService?mediaService.domain:'-',
                    sessionId: online.sessionId,
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

    private static ExecutorService threadpool = Executors.newFixedThreadPool(5);
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
