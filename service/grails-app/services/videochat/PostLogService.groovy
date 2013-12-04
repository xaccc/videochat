package videochat

import java.util.*;
import java.net.URI;
import java.util.concurrent.*;

import org.apache.http.client.fluent.Async;
import org.apache.http.client.fluent.Content;
import org.apache.http.client.fluent.Request;


class PostLogService {
    // static scope = "singleton"  // default

    private String logHost = "localhost";
    private int    logPort = 80;
    private String logLogPath = "/videochat/log";

    private ExecutorService threadpool = Executors.newFixedThreadPool(2);
    private Async async = Async.newInstance().use(threadpool);

    def postLog(def params) {
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
                _postLog(new StringBuffer('event=live&uid=').append(uid).append('timestamp').append(ts));
                break;
            case 'broadcastClose':
                def online = Online.findBySessionId(sn?:'')
                def uid = online?online.uid:''
                online?.delete(flush:true);
                _postLog(new StringBuffer('event=offlive&uid=').append(uid).append('timestamp').append(ts));
                break;
            }
        }
    }

    private void _postLog(StringBuffer query) {
        try {
            Request request = Request.Get(new URI("http", null, logHost, logPort, logLogPath, 
                query.toString(), null));
            Future<Content> future = async.execute(request);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
}
