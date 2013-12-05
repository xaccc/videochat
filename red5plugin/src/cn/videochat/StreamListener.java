package cn.videochat;

import org.apache.log4j.Logger; 

import org.apache.mina.core.buffer.IoBuffer;
import org.red5.server.api.stream.IStreamListener;
import org.red5.server.api.stream.IBroadcastStream;
import org.red5.server.api.stream.IStreamPacket;


class StreamListener implements IStreamListener {

    private Logger log = Logger.getLogger(StreamListener.class);

    public void packetReceived(IBroadcastStream stream, IStreamPacket packet) {
        IoBuffer iobuf = packet.getData();
        try {
            int limit = iobuf.limit(); // iobuf.buf();
            log.info("IStreamPacket data size: " + limit);
        } catch(Exception e) {
            e.printStackTrace();
        } finally {
            iobuf = null;
        }
    }
}
