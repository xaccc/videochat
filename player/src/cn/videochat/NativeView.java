package cn.videochat;


import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class NativeView extends GLSurfaceView {
	
	private static String TAG = "NativeView";
	
	public NativeView(Context context){
		super(context);
	}

	public NativeView(Context context, VideoChat vc) {
		super(context);
		
		setEGLContextClientVersion(2);
		setRenderer(new Renderer(vc));
	}
	
	private static class Renderer implements GLSurfaceView.Renderer {
		VideoChat videochat;

		public Renderer(VideoChat vc) {
			videochat = vc;
		}

		final int fps = 1000/15; // 15fps
		long startTime = System.currentTimeMillis();
		public void onDrawFrame(GL10 gl) {

			long dt = System.currentTimeMillis() - startTime;
		    if (dt < fps) {
				try {
					Thread.sleep(fps - dt);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
		    }

			videochat.RenderFrame();
			
		    startTime = System.currentTimeMillis();
		}

		public void onSurfaceChanged(GL10 gl, int width, int height) {
			videochat.InitRender(width, height);
			Log.w(TAG, String.format("[onSurfaceChanged] width=%d, height=%d", width, height));
		}

		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			// set thread priority
			Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
		}
	}
}
