package com.domain17.glflight;

import java.math.RoundingMode;
import java.time.temporal.ChronoUnit;
import java.time.temporal.TemporalUnit;
import java.util.*;

import android.app.*;

import java.time.*;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
//import android.opengl.EGLConfig;
import android.opengl.EGL14;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.view.MotionEvent;
import android.view.View;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.view.*;
import android.hardware.*;
import android.media.*;
import android.opengl.GLSurfaceView.*;
//import android.opengl.EGLDisplay;
import android.opengl.EGLExt;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

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

    double viewWidthScaled;
    double viewHeightScaled;

    

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        appCtx = this.getApplicationContext();

        // terminate when leaving front
        //this.getIntent().addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

        if (GameResources.init(this.getApplicationContext()) != 1) {
            System.out.println("GameResources.init failed\n");
        } else {
            System.out.println("GameResources.init succeeded\n");
        }

        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        //this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        gameRenderer.surfaceView = new GLSurfaceView(this);

        GameGLConfigChooser glConfigChooser = new GameGLConfigChooser();
        gameRenderer.surfaceView.setEGLConfigChooser(glConfigChooser);
        gameRenderer.surfaceView.setDrawingCacheEnabled(false);

        gameRenderer.surfaceView.setRenderer(gameRenderer);

        if (!renderContinuously)
            gameRenderer.surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        setContentView(gameRenderer.surfaceView);

        contentView = gameRenderer.surfaceView;

        contentView.setOnTouchListener(mOnTouchListener);

        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mSensorGyro = mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);

        // sensor registerListener called in onResume

        GameRunnable.glFlightInit(gameRenderer);

        running = true;
        mBGThread.start();
        if(!renderContinuously) mRenderThread.start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mSensorManager.unregisterListener(this);

        running = false;

        try {
            mBGThread.join();
            if(!renderContinuously) mRenderThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        /*
        mRenderThread.cancel();
        */

        GameRunnable.glFlightUninit();
    }

//    @Override
//    protected void onPostCreate(Bundle savedInstanceState) {
//        super.onPostCreate(savedInstanceState);
//    }

    @Override
    protected void onPause() {
        super.onPause();

        mSensorManager.unregisterListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();

        mSensorManager.registerListener(this, mSensorGyro, SensorManager.SENSOR_DELAY_FASTEST);
        running = true;
    }
    
    /**
     * SensorEventListener
     */

    public void onSensorChanged(SensorEvent e) {

        // thanks to
        // https://github.com/tutsplus/android-sensors-in-depth-proximity-and-gyroscope/blob/master/app/src/main/java/com/tutsplus/sensorstutorial/RotationVectorActivity.java

        float[] Rm = new float[16];
        SensorManager.getRotationMatrixFromVector(Rm, e.values);

        // no need to do remapCoordinateSystem but do change order and invert z
        int col_sz = 4;
        //Rm[2*col_sz] *= -1; Rm[2*col_sz+1] *= -1; Rm[2*col_sz+2] *= -1; // invert-z

        float[] orientations = new float[3];
        SensorManager.getOrientation(Rm, orientations);

    	if(e.sensor == mSensorGyro)
    	{
//            System.out.println("orientations: " + orientations[0] + "," + orientations [1] + "," + orientations[2]);

            GameRunnable.glFlightSensorInput(orientations);
    	}
    }

    public void onAccuracyChanged(Sensor s, int a) {
    	accuracyLast = a;
		//System.out.println("onAccuracyChanged\n");
    }
    
    float touchLastX = 0;
    float touchLastY = 0;
    float touchLastAct = 0;
	float touchFudge = 5;
    
    public boolean onTouchEvent(MotionEvent motionEvent) {
    	float action = 0;
    	float x = 0;
    	float y = 0;
    	float hackNavBar = (float) viewWidthScaled*0.065f; // from gameInterfaceInit (gameInterface.c)

		InputDevice dev = motionEvent.getDevice();

		for(int i = 0; i < motionEvent.getPointerCount(); i++)
        {
            int pointerID = motionEvent.getPointerId(i);
			double tX = (motionEvent.getX(i) / dev.getMotionRange(InputDevice.MOTION_RANGE_X).getMax()) * (viewWidthScaled+hackNavBar);
			double tY = (motionEvent.getY(i) / dev.getMotionRange(InputDevice.MOTION_RANGE_Y).getMax()) * viewHeightScaled;

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

    double mRenderNext = (double) java.lang.System.currentTimeMillis();

    /**
     * render scheduler
     */
    Thread mRenderThread = new Thread(new Runnable() {

        public void run() {

            while (running) {
                double delta = mRenderNext - java.lang.System.currentTimeMillis();

                if (delta <= 0) {
                    mRenderNext = mRenderNext + (1000.0 / GameRenderer.fps);

                    gameRenderer.requestRender();

                    delta = mRenderNext - java.lang.System.currentTimeMillis(); //Instant.now().compareTo(mRenderNext);
                    //System.out.println("delta="+delta);
                }

                try {
                    if(delta > 0) java.lang.Thread.sleep((long) delta);
                } catch (InterruptedException e) {
                }
            }
        }
    });

    Thread mBGThread = new Thread(new Runnable() {
    		public void run() {
    			long sleep_ms = 1000/(GameRenderer.fps/2);

    			while(running) {

					GameRunnable.runBGThread();
	    			
	    			while(true) {
	    				String sndName = GameRunnable.nextSoundEvent();
	    				if(sndName.length() <= 0) break;

	    				GameResources.playSound(sndName);
	    			}

	        		try 
	        		{
	        			java.lang.Thread.sleep(sleep_ms); 
	        		}
	        		catch (InterruptedException e)
	        		{
	        		}
    			}
    		}
    	});
    
    
    private GameRenderer gameRenderer = new GameRenderer();
    private View contentView;
    private SensorManager mSensorManager;
    private Sensor mSensorGyro; 
}
