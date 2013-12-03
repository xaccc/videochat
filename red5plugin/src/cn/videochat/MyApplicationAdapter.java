package cn.videochat;

import java.util.*;
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

//
// 流程：
// Publisher: start -> connect -> join -> streamPublishStart -> streamBroadcastStart -> streamBroadcastClose -> disconnect -> stop
// Client:    connect -> join -> streamSubscriberStart -> streamPlayItemPlay -> streamSubscriberClose -> disconnect -> leave
//


class MyApplicationAdapter extends ApplicationAdapter {

    private Hashtable<String,String> properties = new Hashtable<String,String>();
    
    private Logger log = Logger.getLogger(MyApplicationAdapter.class);
    private StreamListener streamListener = new StreamListener();

    public MyApplicationAdapter() {
        super();
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

        return super.start(scope);
    }

    public void stop(IScope scope) {
        log.info("Stop");
        super.stop(scope);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Client
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    public boolean connect(IConnection conn, IScope scope, Object[] params) {
        log.info(String.format("Connect IP:%s Port:%d, Session:%s", conn.getRemoteAddress(), conn.getRemotePort(), conn.getSessionId()));
        log.info("Connect IConnection:" + conn.getClass().getName());
        for(Object param : params) {
            log.info(param);
            if (param.toString().indexOf('=') > 0) {
                String[] kv = param.toString().split("=");
                conn.setAttribute(kv[0], kv.length > 1 ? kv[1] : null);
            }
        }
        return super.connect(conn, scope, params);
    }

    public void disconnect(IConnection conn, IScope scope) {
        log.info(String.format("Disconnect IP:%s Port:%d, Session:%s", conn.getRemoteAddress(), conn.getRemotePort(), conn.getSessionId()));
        super.disconnect(conn, scope);
    }

    public boolean join(IClient client, IScope scope) {
        log.info(String.format("Join ClientID:%s", client.getId()));
        log.info("join IClient:" + client.getClass().getName());
        return super.join(client, scope);
    }

    public void leave(IClient client, IScope scope) {
        log.info(String.format("Leave ClientID:%s", client.getId()));

        log.info("leave IClient:" + client.getClass().getName());
        super.leave(client, scope);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Stream
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

    public void streamPublishStart(IBroadcastStream stream) {
        log.info("streamPublishStart IBroadcastStream:" + stream.getClass().getName());
        super.streamPublishStart(stream);
        stream.addStreamListener(streamListener);
        if (stream instanceof ClientBroadcastStream) {
            ClientBroadcastStream broadcastStream = (ClientBroadcastStream)stream;
            log.info("streamPublishStart Params:" + getConnectParams(broadcastStream));
        }
    }

    public void streamBroadcastStart(IBroadcastStream stream) {
        log.info("streamBroadcastStart IBroadcastStream:" + stream.getClass().getName());
        super.streamBroadcastStart(stream);
        if (stream instanceof ClientBroadcastStream) {
            ClientBroadcastStream broadcastStream = (ClientBroadcastStream)stream;
            log.info("streamBroadcastStart Params:" + getConnectParams(broadcastStream));
        }
    }

    public void streamBroadcastClose(IBroadcastStream stream) {
        log.info("streamBroadcastClose IBroadcastStream:" + stream.getClass().getName());
        super.streamBroadcastClose(stream);
        if (stream instanceof ClientBroadcastStream) {
            ClientBroadcastStream broadcastStream = (ClientBroadcastStream)stream;
            log.info("streamBroadcastClose Params:" + getConnectParams(broadcastStream));
        }
    }

    public void streamSubscriberStart(ISubscriberStream stream) {
        log.info("streamSubscriberStart ISubscriberStream:" + stream.getClass().getName());
        super.streamSubscriberStart(stream);
        if (stream instanceof PlaylistSubscriberStream) {
            PlaylistSubscriberStream subscriberStream = (PlaylistSubscriberStream)stream;
            log.info("streamSubscriberStart Params:" + getConnectParams(subscriberStream));
        }
    }

    public void streamSubscriberClose(ISubscriberStream stream) {
        log.info("streamSubscriberClose ISubscriberStream:" + stream.getClass().getName());
        super.streamSubscriberClose(stream);
        if (stream instanceof PlaylistSubscriberStream) {
            PlaylistSubscriberStream subscriberStream = (PlaylistSubscriberStream)stream;
            log.info("streamSubscriberClose Params:" + getConnectParams(subscriberStream));
        }
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

}
