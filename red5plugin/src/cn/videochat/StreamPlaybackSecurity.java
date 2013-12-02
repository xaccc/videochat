package cn.videochat;

import org.apache.log4j.Logger; 

import org.red5.server.api.stream.IStreamPlaybackSecurity;
import org.red5.server.api.scope.IScope;

class StreamPlaybackSecurity implements IStreamPlaybackSecurity {

    private Logger log = Logger.getLogger(StreamPlaybackSecurity.class);

    public boolean isPlaybackAllowed(IScope scope, String name, int start, int length, boolean flushPlaylist) {
        log.info("isPlaybackAllowed");
        return true;
    }
}
