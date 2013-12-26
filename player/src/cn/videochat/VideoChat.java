/**
 * 
 * Copyright (C) 2013 Dai Jingjing (jjyyis@qq.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
package cn.videochat;

import java.io.IOException;
import java.util.HashSet;
import java.util.Iterator;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.StatusLine;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;

import cn.videochat.VideoChat.View.MyVideoRender;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.AsyncTask;
import android.util.Log;

public class VideoChat {
	private static final String TAG = "VideoChat.LOG";

	//
	// Error Codes
	//
	public static final int ERROR_FETCH_PLAYER_URL = -1; // 获取主播地址失败
	public static final int ERROR_CONNECT_MEDIA_SERVER = -2;// 连接媒体服务器失败
	public static final int ERROR_RECV_MEDIA_PACKET = -3; // 接收媒体包错误
	public static final int ERROR_INVALID_PACKET_TYPE = -4; // 无效的数据包类型

	//
	//
	//
	public static final int EVENT_BUFFERING_BEGIN = 1;
	public static final int EVENT_BUFFERING_UPDATE_S = 100;
	public static final int EVENT_BUFFERING_UPDATE_E = 200;
	public static final int EVENT_BUFFERING_COMPLETION = 3;
	public static final int EVENT_COMPLETION = 4;
	
	
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

	public interface ErrorListener {
		public boolean onError(VideoChat player, int what, int extra);
	}

	protected interface glSurfaceViewCreateListener {
		public void onCreateListener(MyVideoRender myVideoRender);
	}

	public static class View extends GLSurfaceView {

		MyVideoRender render = new MyVideoRender();

		public View(Context context) {
			super(context);
			setEGLContextClientVersion(2);
			setRenderer(render);
		}

		public MyVideoRender getRender() {
			return render;
		}

		public class MyVideoRender implements GLSurfaceView.Renderer {
			private long renderTime = System.currentTimeMillis();
			private final int fps = 1000 / 15; // max 25fps
			private long _handler = 0;

			public MyVideoRender() {
			}

			public void Destory() {
				VideoChat.RenderRelease(_handler);
				_handler = 0;
				Log.i("FUCK", "[Destory]");
			}

			public long getHandler() {
				return _handler;
			}

			public void onDrawFrame(GL10 gl) {
				// GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT |
				// GLES20.GL_DEPTH_BUFFER_BIT);

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
				if (height == 0) {
					height = 1;
				}
				gl.glViewport(0, 0, width, height);
				Log.i("FUCK", "[onSurfaceChanged]");
			}

			public void onSurfaceCreated(GL10 gl, EGLConfig config) {
				Log.i("FUCK", "[onSurfaceCreated]");
				this.Destory();
				_handler = VideoChat.CreateRender();
				Thread.currentThread().setPriority(Thread.MIN_PRIORITY);

				fireCreateEvent();
			}

			private HashSet<glSurfaceViewCreateListener> createListeners = new HashSet<glSurfaceViewCreateListener>();

			public void addCreateListener(glSurfaceViewCreateListener listener) {
				createListeners.add(listener);
			}

			public void removeCreateListener(
					glSurfaceViewCreateListener listener) {
				createListeners.remove(listener);
			}

			private void fireCreateEvent() {
				Iterator<glSurfaceViewCreateListener> itr = createListeners.iterator();
				while (itr.hasNext()) {
					glSurfaceViewCreateListener listener = (glSurfaceViewCreateListener) itr.next();
					listener.onCreateListener(this);
				}
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
	private String UserID = "";
	private String MyID = "";
	private View view;
	private long _handler = 0L;
	private boolean _isPlayering = false;

	public VideoChat() {
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

	public void setDataSource(String myId, String uId) {
		UserID = uId;
		MyID = myId;
		StringBuffer sb = new StringBuffer(buildApiUrl());
		sb.append("liveUrl?myid=").append(myId).append("&uid=").append(uId);
		url = sb.toString();
		Log.i(TAG, "[setDataSource]");
	}

	public void setUrl(String _url) {
		url = _url;
		Log.i(TAG, "[setUrl]");
	}

	public void play() {
		OpenPlayer(_handler, url);
		Log.i(TAG, "[play]");
	}

	public boolean isPlaying() {
		return _isPlayering;
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
		SetVideoRender(_handler, view.getRender().getHandler());
		view.getRender().addCreateListener(onViewCreated);
	}

	//
	// surface view created
	//
	glSurfaceViewCreateListener onViewCreated = new glSurfaceViewCreateListener() {
		@Override
		public void onCreateListener(MyVideoRender myVideoRender) {
			SetVideoRender(_handler, myVideoRender.getHandler());
		}
	};

	public void render() {
		RenderFrame(_handler);
	}

	//
	// 公共事件接口处理
	//
	private HashSet<BufferingUpdateListener> bufferingUpdateListeners = new HashSet<BufferingUpdateListener>();
	public void addBufferingUpdateListener(BufferingUpdateListener listener) {
		bufferingUpdateListeners.add(listener);
	}
	public void removeBufferingUpdateListener(BufferingUpdateListener listener) {
		bufferingUpdateListeners.remove(listener);
	}
	private void fireBufferingBeginEvent() {
		Iterator<BufferingUpdateListener> itr = bufferingUpdateListeners
				.iterator();
		while (itr.hasNext()) {
			BufferingUpdateListener listener = (BufferingUpdateListener) itr
					.next();
			listener.onBufferingBegin(this);
			listener.onBufferingUpdate(this, 0);
		}
	}
	private void fireBufferingCompleteEvent() {
		Iterator<BufferingUpdateListener> itr = bufferingUpdateListeners
				.iterator();
		while (itr.hasNext()) {
			BufferingUpdateListener listener = (BufferingUpdateListener) itr
					.next();
			listener.onBufferingUpdate(this, 100);
			listener.onBufferingComplete(this);
		}
	}
	private void fireBufferingUpdateEvent(int bufferingProgress) {
		Iterator<BufferingUpdateListener> itr = bufferingUpdateListeners
				.iterator();
		while (itr.hasNext()) {
			BufferingUpdateListener listener = (BufferingUpdateListener) itr
					.next();
			listener.onBufferingUpdate(this, bufferingProgress);
		}
	}

	private HashSet<ErrorListener> errorListeners = new HashSet<ErrorListener>();
	public void addErrorListener(ErrorListener listener) {
		errorListeners.add(listener);
	}
	public void removeErrorListener(ErrorListener listener) {
		errorListeners.remove(listener);
	}
	private void fireErrorEvent(int what, int extra) {
		Iterator<ErrorListener> itr = errorListeners.iterator();
		while (itr.hasNext()) {
			ErrorListener listener = (ErrorListener) itr.next();
			listener.onError(this, what, extra);
		}
	}

	private HashSet<CompletionListener> completionListeners = new HashSet<CompletionListener>();

	public void addCompletionListener(CompletionListener listener) {
		completionListeners.add(listener);
	}

	public void removeCompletionListener(CompletionListener listener) {
		completionListeners.remove(listener);
	}

	private void fireCompletionEvent() {
		Iterator<CompletionListener> itr = completionListeners.iterator();
		while (itr.hasNext()) {
			CompletionListener listener = (CompletionListener) itr.next();
			listener.onCompletionListener(this);
		}
	}

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

	// /////////////////////////////////////////////////////////////////////////
	//
	// JNI
	//
	// /////////////////////////////////////////////////////////////////////////
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

	private native static long SetRenderViewPort(long handler, int width,
			int height);

	private native static void RenderFrame(long handler);

	private native static void RenderRelease(long handler);

	//
	// 内部JNI事件相关接口定义(不能修改函数名)
	//
	int lastErrorId = 0;

	public void onJniEvent(int eventId) {
		Log.i(TAG, "[onJniEvent] eventId=" + eventId);
		if (eventId == 0) {
			// NDK ping
		} else if (eventId == EVENT_BUFFERING_BEGIN) {
			StringBuffer sb = new StringBuffer(buildApiUrl());
			sb.append("postLog?type=client&event=start&uid=").append(UserID)
					.append("&myid=").append(MyID);
			new PostLog().execute(sb.toString());

			_isPlayering = true;
			fireBufferingBeginEvent();
		} else if (eventId >= EVENT_BUFFERING_UPDATE_S && eventId <= EVENT_BUFFERING_UPDATE_E) {
			fireBufferingUpdateEvent(eventId - 100);
		} else if (eventId == EVENT_BUFFERING_COMPLETION) {
			lastErrorId = 0;
			fireBufferingCompleteEvent();
		} else if (eventId == EVENT_COMPLETION) {
			StringBuffer sb = new StringBuffer(buildApiUrl());
			sb.append("postLog?type=client&event=stop&uid=").append(UserID)
					.append("&myid=").append(MyID);
			new PostLog().execute(sb.toString());

			_isPlayering = false;
			fireCompletionEvent();
		} else if (eventId < 0) {
			if (lastErrorId != eventId) {
				lastErrorId = eventId;
				StringBuffer sb = new StringBuffer(buildApiUrl());
				sb.append("postLog?type=client&event=error&code=")
						.append(Math.abs(eventId)).append("&uid=")
						.append(UserID).append("&myid=").append(MyID);
				new PostLog().execute(sb.toString());
			}

			fireErrorEvent(eventId, 0);
		}
	}

	private class PostLog extends AsyncTask<String, String, String> {

		@Override
		protected String doInBackground(String... uri) {
			HttpClient httpclient = new DefaultHttpClient();
			HttpResponse response;
			String responseString = null;
			try {
				response = httpclient.execute(new HttpGet(uri[0]));
				StatusLine statusLine = response.getStatusLine();
				if (statusLine.getStatusCode() == HttpStatus.SC_OK) {
				}
			} catch (ClientProtocolException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			return responseString;
		}

		@Override
		protected void onPostExecute(String result) {
			super.onPostExecute(result);
		}
	}

}
