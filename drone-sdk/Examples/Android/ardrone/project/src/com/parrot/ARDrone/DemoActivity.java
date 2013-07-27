/*
 * AR Drone demo
 * This file is based on the NDK sample app "San Angeles"
 */
package com.parrot.ARDrone;



import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.text.Layout;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.widget.Button;

public class DemoActivity extends Activity implements SensorEventListener {
	private static final  String LOG_TAG = "ARDrone Activity"; 
	private SensorManager mSensorManager;
    private DemoGLSurfaceView mGLView;
	private Sensor mSensor;
	private Button mMoveUpButton;
	private Button mMoveDownButton;
	private Button mTakeOffButton;
	private Button mEmergLandButton;
	private ViewGroup mLayoutView;
	// private GLSurfaceView mLayoutView;
	
	// Used for toggle the takeoff/landing button text
	private boolean mFlyingState = false;
	
	public final static int  COMMAND_TAKEOFF 			= 1;
	public final static int  COMMAND_EMERGLAND 	= 2;
	public final static int  COMMAND_MOVEUP  			= 3;
	public final static int  COMMAND_MOVEDOWN 	= 4;	
	public final static int  COMMAND_USE_ACCELEROMETER 	= 5;	
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
       
        setContentView( R.layout.main );
        
        mGLView =  (DemoGLSurfaceView) findViewById(R.id.glsurfaceview);

        mGLView.initialize( this );

        // Map initialize control buttons
		mTakeOffButton = (Button) findViewById( R.id.takeOffButton );
		mTakeOffButton.setOnTouchListener( new  View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
			//	Log.d(LOG_TAG, event.toString() );
				if( event.getAction() == MotionEvent.ACTION_DOWN ) {
					DemoActivity.nativeCommand(COMMAND_TAKEOFF, 0, 0, 0, 0, 0);
					if( mFlyingState == false ) {
						mTakeOffButton.setText( "Land");
					}
					else {
						mTakeOffButton.setText( "Take off");
					}
					mFlyingState = !mFlyingState;
				}

				return false;
			}
		});
		
		mEmergLandButton = (Button) findViewById( R.id.emergLandingButton );
		mEmergLandButton.setOnTouchListener( new  View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
		//		Log.d(LOG_TAG, event.toString() );
				if( event.getAction() == MotionEvent.ACTION_DOWN ) {
					DemoActivity.nativeCommand(COMMAND_EMERGLAND, 0, 0, 0, 0, 0);
				}

				return false;
			}
		});
		
		mMoveUpButton = (Button) findViewById(  R.id.moveUpButton );
		mMoveUpButton.setOnTouchListener( new  View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
			//	Log.d(LOG_TAG, event.toString() );
				if( event.getAction() == MotionEvent.ACTION_DOWN ) {
					DemoActivity.nativeCommand(COMMAND_MOVEUP, 1, 0, 0, 0, 0);
					DemoActivity.nativeCommand(COMMAND_USE_ACCELEROMETER,  1, 0, 0, 0, 0);
				}
				else if( event.getAction() == MotionEvent.ACTION_UP) {
					DemoActivity.nativeCommand(COMMAND_MOVEUP, 0, 0, 0, 0, 0);
					// Enable accelerometer use for steering
					DemoActivity.nativeCommand(COMMAND_USE_ACCELEROMETER,  0, 0, 0, 0, 0);
				}

				return false;
			}
		});
		
		mMoveDownButton = (Button) findViewById( R.id.moveDownButton  );
		mMoveDownButton.setOnTouchListener( new  View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
		//		Log.d(LOG_TAG, event.toString() );
				if( event.getAction() == MotionEvent.ACTION_DOWN ) {
					DemoActivity.nativeCommand(COMMAND_MOVEDOWN, 1, 0, 0, 0, 0);
					DemoActivity.nativeCommand(COMMAND_USE_ACCELEROMETER,  1, 0, 0, 0, 0);
				}
				else if( event.getAction() == MotionEvent.ACTION_UP) {
					DemoActivity.nativeCommand(COMMAND_MOVEDOWN, 0, 0, 0, 0, 0);
					// Enable accelerometer use for steering
					DemoActivity.nativeCommand(COMMAND_USE_ACCELEROMETER,  0, 0, 0, 0, 0);
				}

				return false;
			}
		});


		mLayoutView = (ViewGroup) findViewById(  R.id.layoutView );
		mLayoutView.setOnTouchListener( new  View.OnTouchListener() {
			@Override
			public boolean onTouch(View v, MotionEvent event) {
			//	Log.d(LOG_TAG, event.toString() );
				if( event.getAction() == MotionEvent.ACTION_DOWN ) {
					// Disable accelerometer use for steering
					DemoActivity.nativeCommand(COMMAND_USE_ACCELEROMETER,  1, 0, 0, 0, 0);
				}
				else if( event.getAction() == MotionEvent.ACTION_UP) {
					// Enable accelerometer use for steering
					DemoActivity.nativeCommand(COMMAND_USE_ACCELEROMETER,  0, 0, 0, 0, 0);
				}
				return true;
			}
		});
		
        // Set flags to keep screen from dimming
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                             WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
        
        // Get Sensor Manager
		mSensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
		mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
		mSensorManager.unregisterListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();
		// update every 200 ms (NORMAL), 60 ms (UI) or 20 ms (GAME)
        mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_GAME);
    }

    @Override
    protected void onStop() {
        super.onStop();
        nativeStop();
    }

	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		/* do nothing ? */
    }

    public void onSensorChanged(final SensorEvent ev) {
		
    	//Log.d(LOG_TAG, "azimuth= " + ev.values[0] +  " pitch= " + ev.values[1] +  "roll= " + ev.values[2] );
    	nativeSensorEvent(ev.values[0],ev.values[1],ev.values[2]);
    }

    static {
        System.loadLibrary("ardrone");
    }
        
   
    /**
     * Method for pass enumerated commands to native layer
     * 
     * @param commandId
     * @param iparam1
     * @param fparam1
     * @param fparam2
     * @param fparam3
     * @param fparam4
     */
    private static native void nativeCommand(int commandId, int iparam1, float fparam1, float fparam2, float fparam3, float fparam4);
    
    private static native void nativeStop();
	private static native void nativeSensorEvent(float x, float y, float z);
}



