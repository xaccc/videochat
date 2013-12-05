package cn.videochat;

import org.apache.log4j.Logger; 

import java.io.*;
import java.util.*;
import java.net.URI;
import java.util.concurrent.*;
import java.util.concurrent.atomic.*;
import java.nio.channels.FileChannel;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;


import org.red5.io.ITag;
import org.red5.io.flv.impl.Tag;
import org.red5.io.flv.impl.FLVWriter;

import org.red5.server.api.stream.IStreamPublishSecurity;
import org.red5.server.api.stream.IBroadcastStream;
import org.red5.server.api.stream.IStreamPacket;
import org.red5.server.api.stream.IStreamListener;
import org.red5.server.stream.ClientBroadcastStream;
import org.red5.server.net.rtmp.event.AudioData;
import org.red5.server.net.rtmp.event.VideoData;

import org.apache.mina.core.buffer.IoBuffer;

class AutoRecorder implements IStreamListener {

    private Hashtable<String,String> properties;
    
    private Logger log = Logger.getLogger(AutoRecorder.class);

    private MyApplicationAdapter app;
    private ClientBroadcastStream stream;
    private FLVWriter writer = null;
 
    long videoTs = 0;
    long audioTs = 0;
    long millis = 0;
    long starSleep = 0;
    long splitTime = 0;
    
    
    public AutoRecorder(MyApplicationAdapter _app, ClientBroadcastStream _stream, Hashtable<String,String> _properties) {
        app = _app;
        properties = _properties;
        stream = _stream;
        log.info("Create Recorder for " + stream.getPublishedName() );
    }
    
    public void release() {
        if (writer != null) {
            writer.close();
            writer = null;
        }
    }

    public void packetReceived(IBroadcastStream stream, IStreamPacket packet) {
        long curTime = System.currentTimeMillis();
        
        if (curTime - millis > 3*1000) { // reopen 每3秒钟，截个录像
            if (writer != null) {
                writer.close();
                writer = null;
                videoTs = 0;
                audioTs = 0;
                starSleep = System.currentTimeMillis();
            }
            
            if (curTime - starSleep > 3*1000 && 
                packet instanceof VideoData && 
                ((VideoData)packet).getFrameType() == VideoData.FrameType.KEYFRAME) { // sleep for writer & keyframe video data
                
                String recordingFile = new StringBuffer(properties.containsKey("liveFlvPath")?properties.get("liveFlvPath"):"./")
                                            .append(stream.getPublishedName())
                                            .append(".flv").toString();

                File flvFile = new File(recordingFile);
                backupFlvFile(flvFile, curTime);
                writer = new FLVWriter(flvFile, false);
                millis = System.currentTimeMillis();
            }
        }
        
        if (writer != null) {
            writePacket(packet);
        }
    }
    
    public void writePacket(IStreamPacket packet) {
        
        if (packet instanceof VideoData || packet instanceof AudioData) {
            
            ITag tag = new Tag();
            
            tag.setDataType(packet.getDataType());
            if (packet instanceof VideoData) { 
                if(0 == videoTs) {
                    videoTs = packet.getTimestamp();
                }
                tag.setTimestamp((int)(packet.getTimestamp() - videoTs));
            } else if (packet instanceof AudioData) {
                if(0 == audioTs) {
                    audioTs = packet.getTimestamp();
                }
                tag.setTimestamp((int)(packet.getTimestamp() - audioTs));
            }
            
            IoBuffer data = packet.getData();
            
            tag.setBodySize(data.limit());
            tag.setBody(data);
            
            try {
                writer.writeTag(tag);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }
    
    private boolean firstBackup = true;
    private void backupFlvFile(File flvFile, long curTime) {
        if (splitTime != (long)(curTime / (5*60*1000)) && flvFile.exists()) { // 每5分钟存个备份
            splitTime = (long)(curTime / (5*60*1000));
            
            String flvid = new SimpleDateFormat("yyyyMMddHHmm").format(new Date(curTime));
            String backupFile = new StringBuffer(properties.containsKey("backupFlvPath")?properties.get("backupFlvPath"):"./")
                                        .append(stream.getPublishedName())
                                        .append("_")
                                        .append(flvid)
                                        .append(".flv").toString();
            if (copoyfile(new File(backupFile), flvFile)) {
                // 7G Bytes 保存1个主播3年的历史数据（按照每5分钟，截5秒视频，每天在线3小时算 / max:250kbps/s）
                StringBuffer query = new StringBuffer("type=stream&event=backup");

                query.append("&name=");
                query.append(stream.getPublishedName());
                query.append("&flvid=");
                query.append(flvid);
                query.append("&first=");
                query.append(firstBackup?1:0);

                app.PostLog(query);
                firstBackup = false;
            }
        }
    }
    
    public static boolean copoyfile(File des,File src) {
        ByteBuffer buf = null;
        int length = 2097152;
        
        try {
            FileInputStream in = new FileInputStream(src);
            FileOutputStream out = new FileOutputStream(des);
            FileChannel inC = in.getChannel();
            FileChannel outC = out.getChannel();

            while(true){
                if(inC.position()==inC.size()){
                    inC.close();
                    outC.close();
                    return true;
                }
                if((inC.size() - inC.position()) < length){
                    length = (int)(inC.size() - inC.position());
                }else
                    length = 2097152;
                    
                buf = ByteBuffer.allocateDirect(length);
                inC.read(buf);
                buf.flip();
                outC.write(buf);
                outC.force(false);
            }
        } catch (Exception e) {
            return false;
        }
    }
}
