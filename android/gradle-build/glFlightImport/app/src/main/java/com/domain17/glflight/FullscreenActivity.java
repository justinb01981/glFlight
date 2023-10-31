
package com.domain17.glflight;

import java.math.RoundingMode;
import java.util.*;

import android.app.*;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.opengl.EGL14;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.View;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.view.ViewGroup.LayoutParams;
import android.view.*;
import android.media.*;
import android.opengl.GLSurfaceView.*;
import android.opengl.EGLExt;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;

import com.domain17.glflight.GameRenderer;
import com.domain17.glflight.util.*;

//import javax.microedition.khronos.egl.EGL10;
//import javax.microedition.khronos.egl.EGLDisplay;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 *
 * @see SystemUiHider
 */
public class FullscreenActivity extends Activity implements SensorEventListener {

    public boolean running = false;
    public boolean renderContinuously = false;        // near as I can tell this makes no difference now  more testing needed
    Context appCtx;
    int accuracyLast = SensorManager.SENSOR_STATUS_UNRELIABLE;


////////////////////////////////////////////////////

    private GameRenderer gameRenderer = new GameRenderer();
    private View contentView;
    private SensorManager mSensorManager;
    private Sensor mSensorGyro;

    ////////////////////////////////////////////////////

    public class GameGLConfigChooser implements EGLConfigChooser {

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

            /* http://www.khronos.org/registry/egl/sdk/docs/man/html/eglChooseConfig.xhtml */
            int[] config_attrs = {
                    EGL10.EGL_LEVEL, 0,
                    EGL10.EGL_COLOR_BUFFER_TYPE, EGL10.EGL_RGB_BUFFER,
                    EGL10.EGL_RED_SIZE, 8,
                    EGL10.EGL_GREEN_SIZE, 8,
                    EGL10.EGL_BLUE_SIZE, 8,
                    EGL10.EGL_ALPHA_SIZE, 8,
                    EGL10.EGL_DEPTH_SIZE, 16,
//                    EGL10.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES_BIT,
                    EGL10.EGL_NONE
            };
            num_config = new int[1];

            if (!egl.eglChooseConfig(display, config_attrs, null, 0,
                    num_config)) {
                throw new IllegalArgumentException("eglChooseConfig failed");
            }

            int config_count = num_config[0];
            configs = new EGLConfig[config_count];

            if (!egl.eglChooseConfig(display, config_attrs, configs, config_count,
                    num_config)) {
                throw new IllegalArgumentException("eglChooseConfig failed");
            }

            return configs[0];
        }

        public EGLConfig[] configs;
        int[] num_config;
    }

    ////////////////////////////////////////////////////

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        appCtx = this.getApplicationContext();

        // terminate when leaving front
        //this.getIntent().addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        //this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        gameRenderer.surfaceView = new GLSurfaceView(this);

        gameRenderer.surfaceView.setEGLConfigChooser(new GameGLConfigChooser());

        setContentView(gameRenderer.surfaceView);
        gameRenderer.surfaceView.setRenderer(gameRenderer);

        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mSensorGyro = mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);

        startSensor();
    }

    protected void initGraphicsEtc() {

        if (GameResources.init(this.getApplicationContext()) != 1) {    // this just saves resource path for texture init
            System.out.println("GameResources.init failed\n");
        } else {
            System.out.println("GameResources.init succeeded\n");
        }

        // sensor registerListener called in onResume

        gameRenderer.surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        GameRunnable.glFlightInit(gameRenderer);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        running = false;

        try {
            mBGThread.join();
            mRenderThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        GameRunnable.glFlightUninit();
    }

    @Override
    protected void onPause() {
        super.onPause();
        gameRenderer.surfaceView.onPause();

        running = false;
    }

    boolean onResumeDone = false;

    @Override
    protected void onResume() {
        super.onResume();

        if (!onResumeDone) {
            onResumeDone = true;
            gameRenderer.surfaceView.onResume();

            initGraphicsEtc();

            running = true;

            if(!renderContinuously) mRenderThread.start();
            sleepSome(100);
            mBGThread.start();  // after paths are set for sound effects?

        }
        else {
            gameRenderer.surfaceView.onResume();     // just resume
        }

    }
    
    /**
     * SensorEventListener
     */

    public void onSensorChanged(SensorEvent e) {

        // thanks to
        // https://github.com/tutsplus/android-sensors-in-depth-proximity-and-gyroscope/blob/master/app/src/main/java/com/tutsplus/sensorstutorial/RotationVectorActivity.java


        float[] Rm = new float[16];
        mSensorManager.getRotationMatrixFromVector(Rm, e.values);

        double delt = System.currentTimeMillis() - e.timestamp;

        // no need to do remapCoordinateSystem but do change order and invert z
        int col_sz = 4;
        //Rm[2*col_sz] *= -1; Rm[2*col_sz+1] *= -1; Rm[2*col_sz+2] *= -1; // invert-z

        float[] orientations = new float[3];
        mSensorManager.getOrientation(Rm, orientations);

        //System.out.println("orientations: " + orientations[0] + "," + orientations [1] + "," + orientations[2]);

        GameRunnable.glFlightSensorInput(orientations);
    }

    public void onAccuracyChanged(Sensor s, int a) {
    	accuracyLast = a;
		System.out.println("onAccuracyChanged: "+a+"\n");
    }
    
    float touchLastX = 0;
    float touchLastY = 0;
    float touchLastAct = 0;
	float touchFudge = 5;
    
    public boolean onTouchEvent(MotionEvent motionEvent) {
    	float action = 0;
    	float x = 0;
    	float y = 0;

		InputDevice dev = motionEvent.getDevice();

		for(int i = 0; i < motionEvent.getPointerCount(); i++)
        {
            int pointerID = motionEvent.getPointerId(i);
            //float hackNavBar = (float) viewWidthScaled*0.065f; // from gameInterfaceInit (gameInterface.c)
			double tX = motionEvent.getX(i); //(motionEvent.getX(i) / dev.getMotionRange(InputDevice.MOTION_RANGE_X).getMax()) * (viewWidthScaled+hackNavBar);
			double tY = motionEvent.getY(i); //(motionEvent.getY(i) / dev.getMotionRange(InputDevice.MOTION_RANGE_Y).getMax()) * viewHeightScaled;

			System.out.println("touched at [" + tX + "," + tY +
					"] action:" + motionEvent.getActionMasked() + "\n");

			x = (int) tX;
			y = (int) tY;

	    	if(motionEvent.getActionMasked() == MotionEvent.ACTION_DOWN)
	    	{
	    		action = 0;
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN)
	    	{
	    		action = 0;
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_UP)
	    	{
	    		action = 1;
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_POINTER_UP)
	    	{
	    		action = 1;
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_MOVE)
	    	{
	    		action = 2;
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_CANCEL)
	    	{
	    		action = 3;
	    	}
	    	
	    	if(action == 2 && Math.abs(x - touchLastX) < touchFudge && Math.abs(y - touchLastY) < touchFudge)
	    	{
	    	}
	    	else
	    	{
	    		touchLastX = x;
	    		touchLastY = y;
	    		touchLastAct = action;
	    		GameRunnable.touchInput(x, y, action, pointerID);
	    	}
		}
    	
    	return super.onTouchEvent(motionEvent);
    }

    /**
     * Touch listener to use for in-layout UI controls to delay hiding the
     * system UI. This is to prevent the jarring behavior of controls going away
     * while interacting with activity UI.
     */
    View.OnTouchListener mOnTouchListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View view, MotionEvent motionEvent) {
            return false;
        }
    };

    private void sleepSome(long lenms) {
        try {
            java.lang.Thread.sleep((long) lenms);
        } catch (InterruptedException e) {
        }
    }

    private void startSensor() {
        assert( mSensorManager.registerListener(this, mSensorGyro, 1000, 100000) );
    }

    private void stopSensor() {
        mSensorManager.unregisterListener(this);
    }

    private void flushSensor(){
        mSensorManager.flush(this);
    }

    double mRenderNext;

    /**
     * render scheduler
     */
    Thread mRenderThread = new Thread(new Runnable() {

        public void run() {

            while (running) {

                gameRenderer.requestRender();   // this is not a race :-P
                sleepSome(1000/ GameRenderer.fps);
            }
        }
    });

    Thread mBGThread = new Thread(new Runnable() {
    		public void run() {
                // this thread serves the network socket so should run more frequently than FPS

    			while(running) {

					GameRunnable.runBGThread();

                    while(true) {
                        String sndName = GameRunnable.nextSoundEvent();
                        if (sndName.length() == 0) break;

                        GameResources.playSound(sndName);
                    }

                    if(GameRunnable.glFlightSensorNeedsCalibrate()) {
                        sleepSome(10);
                        flushSensor();
                    }

                    sleepSome(20);
//                    System.out.println(("bgThread (heartbeat)"));
    			}
    		}
    	});
    
    

}
