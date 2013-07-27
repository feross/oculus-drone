package com.parrot.ARDrone;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;


public class DemoGLSurfaceView extends GLSurfaceView {
	private DemoRenderer mRenderer;
	private static final String LOG_TAG = "ARDrone view";

	public DemoGLSurfaceView(Context context) {
        super(context);
    }
	
	public DemoGLSurfaceView(Context context, AttributeSet attrs) {
		super( context, attrs);
	}

	public void initialize( Context context ) {
        mRenderer = new DemoRenderer();
        setRenderer(mRenderer);

        // receive events
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
	}
	
    @Override
    public boolean onTrackballEvent(final MotionEvent ev) {
        queueEvent(new Runnable() {
                public void run() {
                	// Log.v(LOG_TAG, "track event=" + ev);
                	nativeTrackballEvent(ev.getEventTime(),
                                         ev.getAction(),
                                         ev.getX(),
										 ev.getY());
                }
            });
        return true;
    }
    

    @Override
    public boolean dispatchTouchEvent(final MotionEvent ev) {
        queueEvent(new Runnable() {
                public void run() {
					// Log.v(LOG_TAG, "touch event=" + ev);
                    nativeMotionEvent(ev.getEventTime(),
                                      ev.getAction(),
                                      ev.getX(),
									  ev.getY());
				}
            });
        return true;
    }

    private static native void nativePause();
    private static native void nativeMotionEvent(long eventTime, int action,
												 float x, float y);
	private static native void nativeTrackballEvent(long eventTime,
													int action,
													float x, float y);
	//private static native void nativeKeyEvent(int action);
}