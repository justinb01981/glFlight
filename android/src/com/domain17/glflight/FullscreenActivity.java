package com.domain17.glflight;

import java.util.*;

import android.app.*;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
//import android.opengl.EGLConfig;
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

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

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
	public boolean renderContinuously = false;
	Context appCtx;
	int accuracyLast = SensorManager.SENSOR_STATUS_UNRELIABLE;

	double viewWidthScaled;
	double viewHeightScaled;

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
					EGL10.EGL_RENDERABLE_TYPE, 4,
					EGL10.EGL_STENCIL_SIZE, 8,
					EGL10.EGL_SAMPLE_BUFFERS, 1,
					EGL10.EGL_SAMPLES, 0,
					EGL10.EGL_NONE
			};
			num_config = new int[1];
			if (!egl.eglChooseConfig(display, config_attrs, null, 0,
					num_config)) {
				throw new IllegalArgumentException("eglChooseConfig failed");
			}

			int config_count = num_config[0];
			configs = new EGLConfig[config_count];

			if (!egl.eglChooseConfig(display, null, configs, config_count,
					num_config)) {
				throw new IllegalArgumentException("eglChooseConfig failed");
			}

			return configs[0];
		}

		public EGLConfig[] configs;
		int[] num_config;
	};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        appCtx = this.getApplicationContext();
        
        // terminate when leaving front
        this.getIntent().addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        
        if(GameResources.init(this.getApplicationContext()) != 1)
        {
        	System.out.println("GameResources.init failed\n");
        }
        else
        {
        	System.out.println("GameResources.init succeeded\n");
        }
        
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        //this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,  WindowManager.LayoutParams.FLAG_FULLSCREEN);

		gameRenderer.surfaceView = new GLSurfaceView(this);

		SurfaceHolder h = gameRenderer.surfaceView.getHolder();

        /* iOS on iPhone5:320/568 */
		double sh = 640;
		double sw = sh * 1.5;

		viewWidthScaled = sw;
		viewHeightScaled = sh;

		h.setFixedSize((int) sw, (int) sh);

		GameGLConfigChooser glConfigChooser = new GameGLConfigChooser();
		gameRenderer.surfaceView.setEGLConfigChooser(false);
		gameRenderer.surfaceView.setEGLConfigChooser(glConfigChooser);
		gameRenderer.surfaceView.setDrawingCacheEnabled(false);

		gameRenderer.surfaceView.setRenderer(gameRenderer);

		if(!renderContinuously) gameRenderer.surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        
        setContentView(gameRenderer.surfaceView);
        
        contentView = gameRenderer.surfaceView;

        contentView.setOnTouchListener(mOnTouchListener);
        
        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        mSensorGyro = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        
        if(mSensorGyro != null)
        {
        	System.out.println("got gyro sensor\n");
        	mSensorManager.registerListener(this, mSensorGyro, SensorManager.SENSOR_DELAY_GAME);
        }
        else
        {
        	System.out.println("didn't get gyro sensor\n");
		}

		GameRunnable.glFlightInit(gameRenderer);

		running = true;
		mBGThread.start();
		mTimerThread.start();
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        GameRunnable.glFlightUninit();
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
    }
    
    @Override
    protected void onPause() {
    	super.onPause();
    	
    	running = false;
    }
    
    /**
     * SensorEventListener
     */
    
    public void onSensorChanged(SensorEvent e) {
    	/*
    	if(GameRunnable.glFlightSensorNeedsCalibrate() != 0)
    	{
    		System.out.println("gyro sensor recalibrating");
    		
    		mSensorManager.unregisterListener(this);
    		mSensorManager.registerListener(this, mSensorGyro, SensorManager.SENSOR_DELAY_GAME);
    	}
    	*/
    	if(e.sensor == mSensorGyro)
    	{
			if(/*accuracyLast >= SensorManager.SENSOR_STATUS_ACCURACY_LOW*/true)
                GameRunnable.glFlightSensorInput(e.values);
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
    	float pointerID = motionEvent.getPointerId(motionEvent.getActionIndex());

		InputDevice dev = motionEvent.getDevice();

		for(int i = 0; i < motionEvent.getPointerCount(); i++)
		{
			int index = i;

			double tX = motionEvent.getX(i) ;
			double tY = motionEvent.getY(i);

			// scale
			tX /= (dev.getMotionRange(InputDevice.MOTION_RANGE_X).getMax() / viewWidthScaled);
			tY /= (dev.getMotionRange(InputDevice.MOTION_RANGE_Y).getMax() /  viewHeightScaled);

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
	    		GameRunnable.touchInput(x, y, action, (int) pointerID);
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

	class GameTimer extends TimerTask {
		Timer t;
		public GameTimer() {
			t = new Timer();
			t.scheduleAtFixedRate(this, 0, 1000 / (GameRenderer.fps*16));
		}

		public void run() {
            if(running) {
			    GameRunnable.runTimerThread();

			    gameRenderer.requestRender();
            }
		}

		public void start() {
		}
	}

	GameTimer mTimerThread = new GameTimer();

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
