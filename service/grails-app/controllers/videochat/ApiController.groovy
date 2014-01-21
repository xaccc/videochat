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
    
    def directRecord(String uid) {
        
        def record = null
        
        if (params.fid && params.sid) {
            record = MediaBackup.find("FROM MediaBackup a WHERE a.uid=? and a.flvid=? and a.sessionId=?", 
                                        [uid, params.fid, params.sid])
        } else {
            params.login = params.login?params.login.toBoolean():true
            record = MediaBackup.findByFirstAndUid(params.login, uid, [sort:'dateCreated', order:'desc'])
        }
        
        if (record) {
            redirect(url:   'http://' + record.domain
                            + (grailsApplication.config.directRecord.base?:'/record_streams/')
                            + record.sessionId + '_' 
                            + record.flvid 
                            + (grailsApplication.config.directRecord.ext?:'.flv'))
            return false;
        } else {
            render(status: 404,contentType: "application/json") {
                Method = "directRecord"
                UID = uid
                result = 'Record Don\'t Exists!'
            }
            return false;
        }
    }
    
    //
    // 获取用户直播录像
    //
    def recordLog(String uid) {
        params.login = params.login?params.login.toBoolean():true
        params.max = Math.min(100, params.max?params.max.toLong():10)
        def records = []
        
        records = params.login ? MediaBackup.findAllByFirstAndUid(true, uid, params)
                               : MediaBackup.findAllByUid(uid, params)
        
        render(contentType: "application/json") {
            Method = "recordLog"
            UID = uid
            Records = records.collect{rec -> [
                    //'UUID': rec.id,
                    //'IP': rec.domain,
                    'SID': rec.sessionId,
                    'FID': rec.flvid,
                    'Login': rec.first,
                    'Date': rec.backupDate
                    ]}
        }
    }

    //
    // 为呆死状态的连接手工下线
    //    
    def removeSession(String id) {
        def online = Online.findBySessionId(id)
        if (!online) {
            render(status: 404,contentType: "application/json") {
                Method = "removeSession"
                SessionId = id
                result = 'Session Don\'t Exists!'
            }
            return false;
        }

        Long ts = new Date().time;
        online.delete(flush: true)
        _postLog(new StringBuffer('op=hostOffLive&room_id=').append(online.uid).append('&timestamp=').append(ts).append('&Version=video'));
        def me = MediaEvent.findByUidAndSessionId(online.uid, online.sessionId)
        if (me) {
            me.dateLeave = new Date(ts)
            me.duration = (me.dateLeave.time - me.dateLive.time)/60000
            me.save(flush:true)
        }

        render(contentType: "application/json") {
            Method = "removeSession"
            SessionId = id
            Status = 'Removed'
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
            case 'broadcastStart': // 申请直播
                def online = Online.findBySessionId(sn?:'')
                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }

                def me = MediaEvent.findByUidAndSessionId(online.uid, online.sessionId)
                if(!me) {
                    new MediaEvent(uid: online.uid, 
                                   sessionId: online.sessionId,
                                   mediaServiceId: online.mediaServiceId,
                                   dateLive: new Date(ts.toLong())).save(flush:true)
                }
                break;

            case 'broadcastClose': // 结束直播
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

            case 'subscriberStart': // 观看开始
                def online = Online.findBySessionId(sn?:'')
                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }
                online.subscriber = online.subscriber ? online.subscriber + 1 : 1;
                online.save(flush:true);
                break;

            case 'subscriberClose': // 观看结束
                def online = Online.findBySessionId(sn?:'')
                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }
                online.subscriber = online.subscriber ? online.subscriber - 1 : 0;
                online.save(flush:true);
                break;
                
            case 'backup': // 数据备份记录
                def flvid = params['flvid']; // 文件ID
                def first = params['first']; // 是否上麦第一次备份
                def online = Online.findBySessionId(sn?:'')

                if (!online) {
                    render(status: 404,contentType: "application/json") {
                        result = 'Session Don\'t Exists!'
                    }
                    return false;
                }

                if (first) {
                    _postLog(new StringBuffer('op=hostLive&room_id=').append(online.uid).append('&timestamp=').append(ts).append('&Version=video'));
                }

                online.lastPing = new Date();
                online.save(flush:true);
                
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

    private static ExecutorService threadpool = Executors.newFixedThreadPool(1);
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
