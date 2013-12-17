/**
 * 
 * Copyright (C) 2013 Dai Jingjing (jjyyis@qq.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
package cn.videochat;

import java.util.HashSet;
import java.util.Iterator;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.util.Log;
import android.view.SurfaceHolder;

public class VideoChat {
	private static final String TAG = "VideoChat.LOG";
	
	
	//
	// Error Codes
	//
	public static final int ERROR_FETCH_PLAYER_URL = -1;
	public static final int ERROR_CONNECT_MEDIA_SERVER = -2;
	public static final int ERROR_RECV_MEDIA_PACKET = -3;
	public static final int ERROR_INVALID_PACKET_TYPE = -4;
	
	

	//
	// 事件回调接口定义
	//
	public interface BufferingUpdateListener {
	     public void onBufferingBegin(VideoChat player);
	     public void onBufferingComplete(VideoChat player);
	     public void onBufferingUpdate(VideoChat player, int bufferingProgress);
	}
	
	public interface CompletionListener {
	     public void onCompletionListener(VideoChat player);
	}
	
	public interface ErrorListener	 {
		public boolean onError(VideoChat player, int what, int extra);
	}
	

	public static class View extends GLSurfaceView {

	    MyVideoRender render = new MyVideoRender();
	    
	    public View(Context context) {
	        super(context);
	        setEGLContextClientVersion(2);
	        setRenderer(render);
	    }
	    
	    public void surfaceDestroyed (SurfaceHolder holder){
	        render.Destory();
	    }
	    
	    public long getRenderHandler(){
	        return render.getHandler();
	    }

	    public class MyVideoRender implements GLSurfaceView.Renderer {
	        private long renderTime = System.currentTimeMillis();
	        private final int fps = 1000 / 25; // max 25fps
	        private long _handler = 0;

	        public MyVideoRender() {
	        }
	        
	        public void Destory() {
	            VideoChat.RenderRelease(_handler);
	            _handler = 0;
	        }

	        public long getHandler() {
	            return _handler;
	        }

	        public void onDrawFrame(GL10 gl) {
	        	//GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

	            long dt = System.currentTimeMillis() - renderTime;
	            if (dt < fps) {
	                try {
	                    Thread.sleep(fps - dt);
	                } catch (InterruptedException e) {
	                    e.printStackTrace();
	                }
	            }

	            VideoChat.RenderFrame(_handler);
	            renderTime = System.currentTimeMillis();
	        }

	        public void onSurfaceChanged(GL10 gl, int width, int height) {
	    		if(height == 0) { height = 1; }
	    		gl.glViewport(0, 0, width, height);
	        }

	        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
/*	    		// Set the background clear color to black.
	    		GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	    		
	    		// Use culling to remove back faces.
	    		GLES20.glEnable(GLES20.GL_CULL_FACE);
	    		
	    		// Enable depth testing
	    		GLES20.glEnable(GLES20.GL_DEPTH_TEST);
	    		*/
	    		// call native lib
	    		_handler = VideoChat.CreateRender();
	            Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
	        }
	    } // class MyVideoRender
	} // class View


	//
	// api params
	//
	private String apiHost = "122.0.67.180";
	private int apiPort = 80;
	private String apiApp = "videochat";
	private String url;
	private View view;
	private long _handler = 0L;

	public VideoChat(){
		_handler = Init();
	}
	
	public VideoChat(String _apiHost, int _apiPort, String _apiApp) {
		apiHost = _apiHost;
		apiPort = _apiPort;
		apiApp = _apiApp;
		_handler = Init();
	}

	public void release() {
		Release(_handler);
		_handler = 0;
		Log.i(TAG, "[release]");
	}
	
	public void setDataSource(String myId, String uId)
	{
		StringBuffer sb = new StringBuffer(buildApiUrl());
		sb.append("liveUrl?myid=").append(myId).append("&uid=").append(uId);
		url = sb.toString();
		Log.i(TAG, "[setDataSource]");
	}
	
	public void setUrl(String _url)
	{
		url = _url;
		Log.i(TAG, "[setUrl]");
	}

	public void play() {
		OpenPlayer(_handler, url);
		Log.i(TAG, "[play]");
	}

	public void pause(boolean paused) {
		PausePlayer(_handler, paused);
		Log.i(TAG, "[pause]");
	}

	public void stop() {
		ClosePlayer(_handler);
		Log.i(TAG, "[stop]");
	}

	public void setRender(View view) {
		SetVideoRender(_handler, view.getRenderHandler());
	}

	public void render() {
		RenderFrame(_handler);
	}

	//
	// 公共事件接口处理
	//
	private HashSet<BufferingUpdateListener> bufferingUpdateListeners = new HashSet<BufferingUpdateListener>();
	public void addBufferingUpdateListener(BufferingUpdateListener listener){
		bufferingUpdateListeners.add(listener);
	}
	
	public void removeBufferingUpdateListener(BufferingUpdateListener listener){
		bufferingUpdateListeners.remove(listener);
	}
	
	private void fireBufferingBeginEvent(){
		Iterator<BufferingUpdateListener> itr = bufferingUpdateListeners.iterator();
		while(itr.hasNext()){
			BufferingUpdateListener listener = (BufferingUpdateListener)itr.next();
			listener.onBufferingBegin(this);
			listener.onBufferingUpdate(this, 0);
		}
	}
	private void fireBufferingCompleteEvent(){
		Iterator<BufferingUpdateListener> itr = bufferingUpdateListeners.iterator();
		while(itr.hasNext()){
			BufferingUpdateListener listener = (BufferingUpdateListener)itr.next();
			listener.onBufferingUpdate(this, 100);
			listener.onBufferingComplete(this);
		}
	}
	private void fireBufferingUpdateEvent(int bufferingProgress){
		Iterator<BufferingUpdateListener> itr = bufferingUpdateListeners.iterator();
		while(itr.hasNext()){
			BufferingUpdateListener listener = (BufferingUpdateListener)itr.next();
			listener.onBufferingUpdate(this, bufferingProgress);
		}
	}
	
	private HashSet<ErrorListener> errorListeners = new HashSet<ErrorListener>();
	public void addErrorListener(ErrorListener listener){
		errorListeners.add(listener);
	}
	public void removeErrorListener(ErrorListener listener){
		errorListeners.remove(listener);
	}
	private void fireErrorEvent(int what, int extra){
		Iterator<ErrorListener> itr = errorListeners.iterator();
		while(itr.hasNext()){
			ErrorListener listener = (ErrorListener)itr.next();
			listener.onError(this, what, extra);
		}
	}
	
	private HashSet<CompletionListener> completionListeners = new HashSet<CompletionListener>();
	public void addCompletionListener(CompletionListener listener){
		completionListeners.add(listener);
	}
	public void removeCompletionListener(CompletionListener listener){
		completionListeners.remove(listener);
	}
	private void fireCompletionEvent(){
		Iterator<CompletionListener> itr = completionListeners.iterator();
		while(itr.hasNext()){
			CompletionListener listener = (CompletionListener)itr.next();
			listener.onCompletionListener(this);
		}
	}
	
	//
	// utils
	//
	private String buildApiUrl() {
		return new StringBuffer("http://").append(apiHost).append(":")
				.append(apiPort).append("/").append(apiApp).append("/api/")
				.toString();
	}
	
	// 是否支持OpenGL ES 2.0
    public static boolean IsSupportOpenGLES2(Context context) {
        ActivityManager activityManager = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager
                .getDeviceConfigurationInfo();

        return configurationInfo.reqGlEsVersion >= 0x20000;
    }
	
    ///////////////////////////////////////////////////////////////////////////
	//
	// JNI
	//
    ///////////////////////////////////////////////////////////////////////////
	static {
		System.loadLibrary("vc");
	}

	//
	// Video Player
	//
	private native long Init();
	private native void Release(long handler);
	private native void SetVideoRender(long handler, long videoRender);
	private native void OpenPublisher(long handler, String rtmpUrl);
	private native void PausePublisher(long handler, boolean paused);
	private native void ClosePublisher(long handler);
	private native void OpenPlayer(long handler, String rtmpUrl);
	private native void PausePlayer(long handler, boolean paused);
	private native void ClosePlayer(long handler);

	//
	// Video Render
	//
	private native static long CreateRender();
	private native static long SetRenderViewPort(long handler, int width, int height);
	private native static void RenderFrame(long handler);
	private native static void RenderRelease(long handler);

	//
	// 内部JNI事件相关接口定义(不能修改函数名)
	//
	public void onJniEvent(int eventId) {
		Log.i(TAG, "[onJniEvent] eventId=" + eventId);
		if (eventId == 0)
		{
			// NDK ping
		}
		else if (eventId == 1)
		{
			//onBufferingBegin
			fireBufferingBeginEvent();
		}
		else if(eventId >= 100 && eventId <= 200)
		{ 
			//onBufferingUpdate
			fireBufferingUpdateEvent(eventId - 100);
		}
		else if(eventId == 3) 
		{
			//onBufferingComplete
			fireBufferingCompleteEvent();
		}
		else if(eventId == 4)
		{
			//onBufferingComplete
			fireCompletionEvent();
		}
		else if ( eventId < 0)
		{
			//onError
			fireErrorEvent(eventId, 0);
		}
	}

}
