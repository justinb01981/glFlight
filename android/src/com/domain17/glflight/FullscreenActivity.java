package com.domain17.glflight;

import java.util.*;

import android.app.*;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
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

import com.domain17.glflight.util.*;

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 *
 * @see SystemUiHider
 */
public class FullscreenActivity extends Activity implements SensorEventListener {
	
	public boolean running = false;
	Context appCtx;
	int accuracyLast = SensorManager.SENSOR_STATUS_UNRELIABLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        appCtx = this.getApplicationContext();
      
        running = true;
        
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
        
        GLSurfaceView glView = new GLSurfaceView(this);
        
        glView.setRenderer(gameRenderer);
        
        setContentView(glView);
        
        contentView = glView;
        
        contentView.setOnTouchListener(mOnTouchListener);
       
        mBGThread.start();
        
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
	    	if(accuracyLast == SensorManager.SENSOR_STATUS_ACCURACY_HIGH) GameRunnable.glFlightSensorInput(e.values);
    	}
    }
    
    public void onAccuracyChanged(Sensor s, int a) {
    	accuracyLast = a;
    }
    
    float touchLastX = 0;
    float touchLastY = 0;
    float touchLastAct = 0;
	float touchFudge = 5;
    
    public boolean onTouchEvent(MotionEvent motionEvent) {
    	System.out.println("touched at [" + motionEvent.getRawX() + "," + motionEvent.getRawY() +
    			"] action:" + motionEvent.getActionMasked() + "\n");
    	
    	float action = 0;
    	float x = 0;
    	float y = 0;
    	
		for(int i = 0; i < motionEvent.getPointerCount(); i++)
		{
			int index = i;
			
	    	if(motionEvent.getActionMasked() == MotionEvent.ACTION_DOWN)
	    	{
	    		x = motionEvent.getX();
	    		y = motionEvent.getY();
	    		action = 0;
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN)
	    	{
	    		
	    		x = motionEvent.getX(index);
	    		y = motionEvent.getY(index);
	    		action = 0;
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_UP)
	    	{
	    		action = 1;
	    		x = motionEvent.getX();
	    		y = motionEvent.getY();
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_POINTER_UP)
	    	{
	    		action = 1;
	    		x = motionEvent.getX(i);
	    		y = motionEvent.getY(i);
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_MOVE)
	    	{
	    		action = 2;
	    		x = motionEvent.getX(i);
	    		y = motionEvent.getY(i);
	    	}
	    	else if(motionEvent.getActionMasked() == MotionEvent.ACTION_CANCEL)
	    	{
	    		action = 3;
	    		x = motionEvent.getX(i);
	    		y = motionEvent.getY(i);
	    	}
	    	
	    	if(action == 2 && Math.abs(x - touchLastX) < touchFudge && Math.abs(y - touchLastY) < touchFudge)
	    	{
	    	}
	    	else
	    	{
	    		touchLastX = x;
	    		touchLastY = y;
	    		touchLastAct = action;
	    		GameRunnable.touchInput(x, y, action);
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

    Thread mBGThread = new Thread(new Runnable() {
    		public void run() {
    			long sleep_ms = 1000/30;

    			while(running)
    			{
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
