/**
 * 
 * Copyright (C) 2013 ALiang (illuspas@gmail.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
package cn.videochat;

import android.view.SurfaceView;

public class VideoChat {

	public interface OnEventCallback {
		public void onEvent(int event);
	}

	static {
		System.loadLibrary("vc");
	}

	public native void Init();
	public native void SetSurfaceView(SurfaceView view);
	public native void OpenPublisher(String rtmpUrl);
	public native void ClosePublisher();
	public native void OpenPlayer(String rtmpUrl);
	public native void ClosePlayer();
	public native void Release();

	public OnEventCallback m_onEventCallback;

	private void onEventCallback(int event) {
		System.out.println("onEventCallback: " + event);
		if (m_onEventCallback != null) {
			m_onEventCallback.onEvent(event);
		}
	}

	public void setOnEventCallback(OnEventCallback onEventCallback) {
		m_onEventCallback = onEventCallback;
	}

}
