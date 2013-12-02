package cn.videochat;

import org.apache.log4j.Logger; 

import org.red5.server.adapter.ApplicationAdapter;
import org.red5.server.api.IConnection;
import org.red5.server.api.scope.IScope;
import org.red5.server.api.IClient;
import org.red5.server.api.stream.ISubscriberStream;
import org.red5.server.api.stream.IBroadcastStream;
import org.red5.server.api.stream.IPlayItem;

//
// 流程：
// Publisher: start -> connect -> join -> streamPublishStart -> streamBroadcastStart -> streamBroadcastClose -> disconnect -> leave
// Client:    connect -> join -> streamSubscriberStart -> streamPlayItemPlay -> streamSubscriberClose -> disconnect -> leave
//


class MyApplicationAdapter extends ApplicationAdapter {

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
        return super.connect(conn, scope, params);
    }
    
    public void disconnect(IConnection conn, IScope scope) {
        log.info(String.format("Disconnect IP:%s Port:%d, Session:%s", conn.getRemoteAddress(), conn.getRemotePort(), conn.getSessionId()));
        super.disconnect(conn, scope);
    }
    
    public boolean join(IClient client, IScope scope) {
        log.info(String.format("Join ClientID:%s", client.getId()));
        return super.join(client, scope);
    }
    
    public void leave(IClient client, IScope scope) {
        log.info(String.format("Leave ClientID:%s", client.getId()));

        super.leave(client, scope);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Stream
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////
    
    public void streamPublishStart(IBroadcastStream stream) {
        log.info("streamPublishStart");
        stream.addStreamListener(streamListener);
        
        super.streamPublishStart(stream);
    }
    
    public void streamBroadcastStart(IBroadcastStream stream) {
        log.info("streamBroadcastStart");
        super.streamBroadcastStart(stream);
    }
    
    public void streamBroadcastClose(IBroadcastStream stream) {
        log.info("streamBroadcastClose");
        super.streamBroadcastClose(stream);
    }
    
    public void streamPlayItemPlay(ISubscriberStream stream, IPlayItem item, boolean isLive) {
        log.info("streamPlayItemPlay");
        super.streamPlayItemPlay(stream, item, isLive);
    }
    
    public void streamSubscriberStart(ISubscriberStream stream) {
        log.info("streamSubscriberStart");
        super.streamSubscriberStart(stream);
    }
    
    public void streamSubscriberClose(ISubscriberStream stream) {
        log.info("streamSubscriberClose");
        super.streamSubscriberClose(stream);
    }
    
}
