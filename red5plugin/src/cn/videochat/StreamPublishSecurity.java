package cn.videochat;

import org.apache.log4j.Logger; 

import org.red5.server.api.stream.IStreamPublishSecurity;
import org.red5.server.api.scope.IScope;

class StreamPublishSecurity implements IStreamPublishSecurity {

    private Logger log = Logger.getLogger(StreamPublishSecurity.class);

    public boolean isPublishAllowed(IScope scope, String name, String mode) {
        log.info("isPublishAllowed");
        return true;
    }
}
