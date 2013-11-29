package cn.videochat;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class NativeView extends GLSurfaceView {
	
	private static String TAG = "NativeView";

	public NativeView(Context context, VideoChat vc) {
		super(context);
		setEGLContextClientVersion(2);	
		//setEGLContextFactory(new ContextFactory());
		setRenderer(new Renderer(vc));
	}
	
    private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
            Log.w(TAG, "creating OpenGL ES 2.0 context");
            checkEglError("Before eglCreateContext", egl);
            int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
            checkEglError("After eglCreateContext", egl);
            return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
            egl.eglDestroyContext(display, context);
        }
    }

    private static void checkEglError(String prompt, EGL10 egl) {
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
            Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }


	private static class Renderer implements GLSurfaceView.Renderer {
		VideoChat videochat;

		public Renderer(VideoChat vc) {
			videochat = vc;
		}

		long startTime = System.currentTimeMillis();
		public void onDrawFrame(GL10 gl) {

			long dt = System.currentTimeMillis() - startTime;
		    if (dt < 33) {
				try {
					Thread.sleep(33 - dt);
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
