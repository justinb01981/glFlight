package com.domain17.glflight;

import static android.opengl.GLES10.*;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.EGL14;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.EGLExt;
import android.view.Choreographer;

import com.domain17.glflight.GameRunnable;

public class GameRenderer implements Renderer {
	
	static {
		System.loadLibrary("glFlight");
	}
	
	static native void onSurfaceCreated();
	static native void onSurfaceChanged(float f[]);
	static native void onDrawFrame();
	
	public static int clientVersion = 1;

	public static long lastDrawTime = 0;
	public static long fps = 60;

	public GLSurfaceView surfaceView;

	boolean glflightInitDone = false;

	@Override
	public void onDrawFrame(GL10 arg0) {
		onDrawFrame();
	}

	@Override
	public void onSurfaceChanged(GL10 arg0, int arg1, int arg2) {
		float f[] = {arg1, arg2};

		onSurfaceChanged(f);

		if(!glflightInitDone) {
			glflightInitDone = true;
			GameRunnable.glFlightInit();    // this is funky (but unused in the JNI code for now
		}
	}

	@Override
	public void onSurfaceCreated(GL10 arg0, EGLConfig arg1) {
		onSurfaceCreated();
	}

	public void requestRender() {
		if(surfaceView != null) surfaceView.requestRender();
	}

	@Override
	protected void finalize() throws Throwable {
		super.finalize();
		GameRunnable.glFlightUninit();
	}
}
