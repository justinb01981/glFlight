package com.domain17.glflight;

public class GameRunnable  {
	
	static {
		System.loadLibrary("glFlight");
	}
	
	static native void glFlightInit();
	static native void glFlightUninit();
	static native void glFlightRunBGThread();
	static native void glFlightTouchInput(float f[]);
	static native String glFlightNextAudioEvent(String s);
	static native boolean glFlightSensorNeedsCalibrate();
	
	static native void glFlightSensorInput(float f[]);
	
	public static void runBGThread() {
		glFlightRunBGThread();
	}
	
	public static void sensorInput(float f[]) {
		
		glFlightSensorInput(f);
	}
	
	public static void touchInput(float x, float y, float down, int pointerID) {
		float f[] = {x, y, down, pointerID};
		glFlightTouchInput(f);
	}
	
	public static java.lang.String nextSoundEvent() {
		String s = new String();
		s = glFlightNextAudioEvent(s);
		return s;
	}
}
