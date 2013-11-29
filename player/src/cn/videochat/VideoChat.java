/**
 * 
 * Copyright (C) 2013 ALiang (illuspas@gmail.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
package cn.videochat;

public class VideoChat {

	public interface OnEventCallback {
		public void onEvent(int event);
	}

	static {
		System.loadLibrary("vc");
	}

	public native void Init();
	public native void Release();

	public native void OpenPublisher(String rtmpUrl);
	public native void PausePublisher(boolean paused);
	public native void ClosePublisher();
	
	public native void OpenPlayer(String rtmpUrl);
	public native void PausePlayer(boolean paused);
	public native void ClosePlayer();
	
	public native void InitRender(int width, int height);
	public native void RenderFrame();

	public OnEventCallback m_onEventCallback;

	private void onEventCallback(int event) {
		if (m_onEventCallback != null) {
			m_onEventCallback.onEvent(event);
		}
	}

	public void setOnEventCallback(OnEventCallback onEventCallback) {
		m_onEventCallback = onEventCallback;
	}

}
