package cn.videochat;

import java.util.*;
import java.net.URI;
import java.util.concurrent.*;
import java.io.FileInputStream;

import org.apache.log4j.Logger;

import org.red5.server.adapter.ApplicationAdapter;
import org.red5.server.api.IConnection;
import org.red5.server.api.scope.IScope;
import org.red5.server.api.IClient;
import org.red5.server.api.stream.ISubscriberStream;
import org.red5.server.api.stream.IBroadcastStream;
import org.red5.server.api.stream.IPlayItem;
import org.red5.server.stream.ClientBroadcastStream;
import org.red5.server.stream.PlaylistSubscriberStream;
import org.red5.server.stream.AbstractClientStream;

import org.apache.http.client.fluent.Async;
import org.apache.http.client.fluent.Content;
import org.apache.http.client.fluent.Request;


//
// 流程：
// Publisher: start -> connect -> join -> streamPublishStart -> streamBroadcastStart -> streamBroadcastClose -> disconnect -> stop
// Client:    connect -> join -> streamSubscriberStart -> streamPlayItemPlay -> streamSubscriberClose -> disconnect -> leave
//


class MyApplicationAdapter extends ApplicationAdapter {

    private Hashtable<String,String> properties = new Hashtable<String,String>();
    
    String apiHost = "localhost";
    int    apiPort = 80;
    String apiLogPath = "/service/api/postlog";
    
    private Logger log = Logger.getLogger(MyApplicationAdapter.class);
    private StreamListener streamListener = new StreamListener();
    
    ExecutorService threadpool = Executors.newFixedThreadPool(2);
    Async async = Async.newInstance().use(threadpool);

    public MyApplicationAdapter() {
        super();

        Properties pps = new Properties();

        try {
            properties.clear();
            
            String configurePath = MyApplicationAdapter.class.getResource("/").getPath() + "config.ini";
            log.info("Load Properties File: " + configurePath);
            pps.load(new FileInputStream(configurePath));
            for(String strKey : pps.stringPropertyNames()) {
                properties.put(strKey, pps.getProperty(strKey));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        if (properties.containsKey("apiHost")) apiHost = properties.get("apiHost");
        if (properties.containsKey("apiPort")) apiPort = Integer.getInteger(properties.get("apiPort"));
        if (properties.containsKey("apiPath")) {
            apiLogPath = properties.get("apiPath") + "/postlog";
        }

        this.registerStreamPlaybackSecurity(new StreamPlaybackSecurity());
        this.registerStreamPublishSecurity(new StreamPublishSecurity());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // application
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    public boolean start(IScope scope) {
        log.info("Start");

        PostLog(new StringBuffer("type=server&event=start"));
        return super.start(scope);
    }

    public void stop(IScope scope) {
        log.info("Stop");
        PostLog(new StringBuffer("type=server&event=stop"));
        super.stop(scope);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Client
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    public boolean connect(IConnection conn, IScope scope, Object[] params) {
        StringBuffer query = new StringBuffer("type=client&event=connect");
        
        query.append("&addr=");
        query.append(conn.getRemoteAddress());
        query.append("&port=");
        query.append(conn.getRemotePort());
        
        for(Object param : params) {
            if (param.toString().length() > 0) {
                query.append("&");
                query.append(param.toString());
            }
        }
        
        PostLog(query);
        return super.connect(conn, scope, params);
    }

    public void disconnect(IConnection conn, IScope scope) {
        StringBuffer query = new StringBuffer("type=client&event=disconnect");
        
        query.append("&addr=");
        query.append(conn.getRemoteAddress());
        query.append("&port=");
        query.append(conn.getRemotePort());
        
        PostLog(query);
        super.disconnect(conn, scope);
    }

    public boolean join(IClient client, IScope scope) {
        StringBuffer query = new StringBuffer("type=client&event=join");
        
        query.append("&id=");
        query.append(client.getId());
        
        PostLog(query);
        return super.join(client, scope);
    }

    public void leave(IClient client, IScope scope) {
        StringBuffer query = new StringBuffer("type=client&event=leave");
        
        query.append("&id=");
        query.append(client.getId());
        
        PostLog(query);
        super.leave(client, scope);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Stream
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    public void streamPublishStart(IBroadcastStream stream) {
        super.streamPublishStart(stream);
        stream.addStreamListener(streamListener);
        
        StringBuffer query = new StringBuffer("type=stream&event=publishStart");

        if (stream instanceof ClientBroadcastStream) {
            ClientBroadcastStream broadcastStream = (ClientBroadcastStream)stream;

            query.append("&name=");
            query.append(broadcastStream.getPublishedName());
        }
        
        PostLog(query);
    }

    public void streamBroadcastStart(IBroadcastStream stream) {
        super.streamBroadcastStart(stream);
        
        StringBuffer query = new StringBuffer("type=stream&event=broadcastStart");

        if (stream instanceof ClientBroadcastStream) {
            ClientBroadcastStream broadcastStream = (ClientBroadcastStream)stream;

            query.append("&name=");
            query.append(broadcastStream.getPublishedName());
        }

        PostLog(query);
    }

    public void streamBroadcastClose(IBroadcastStream stream) {
        super.streamBroadcastClose(stream);

        StringBuffer query = new StringBuffer("type=stream&event=broadcastClose");

        if (stream instanceof ClientBroadcastStream) {
            ClientBroadcastStream broadcastStream = (ClientBroadcastStream)stream;
            
            query.append("&name=");
            query.append(broadcastStream.getPublishedName());
        }
        
        PostLog(query);
    }

    public void streamSubscriberStart(ISubscriberStream stream) {
        super.streamSubscriberStart(stream);

        StringBuffer query = new StringBuffer("type=stream&event=subscriberStart");

        if (stream instanceof PlaylistSubscriberStream) {
            PlaylistSubscriberStream subscriberStream = (PlaylistSubscriberStream)stream;

            query.append("&name=");
            query.append(subscriberStream.getCurrentItem().getName());
        }

        PostLog(query);
    }

    public void streamSubscriberClose(ISubscriberStream stream) {
        super.streamSubscriberClose(stream);
        
        StringBuffer query = new StringBuffer("type=stream&event=subscriberClose");

        if (stream instanceof PlaylistSubscriberStream) {
            PlaylistSubscriberStream subscriberStream = (PlaylistSubscriberStream)stream;

            // query.append("&name=");
            // query.append(subscriberStream.getItem(0).getName());
        }
        
        PostLog(query);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // utils
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    private String getConnectParams(AbstractClientStream stream) {
        String result = "";
        Map<String,Object> params = stream.getConnection().getAttributes();
        for(String k : stream.getConnection().getAttributeNames()) {
            if (result.length() > 0) result += "&";
            result += k + "=" + params.get(k).toString();
        }

        return result;
    }
    
    public void PostLog(StringBuffer query) {
        try {
            Request request = Request.Get(new URI("http", null, apiHost, apiPort, apiLogPath, 
                query.append("&timestamp=").append(System.currentTimeMillis()).toString(), null));
            Future<Content> future = async.execute(request);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

}
