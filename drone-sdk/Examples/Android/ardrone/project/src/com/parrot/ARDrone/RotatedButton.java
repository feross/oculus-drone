package com.parrot.ARDrone;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.widget.Button;

public class RotatedButton extends Button {
    public RotatedButton(Context context) {
        super(context, null);
    }

    public RotatedButton(Context context, AttributeSet attrs) {
        super(context, attrs );
    }

    public RotatedButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }
	
	@Override
	protected void onDraw( Canvas canvas ) {
	     canvas.save();
	     int bottom = this.getBottom();
	     int top = this.getTop();
	     int right = this.getRight();
	     int left = this.getLeft();
	     
	     // int rotX = left + (right - left)/2;
	     // int rotY =  top + (bottom-top)/2; 

	     /*int rotX = (right - left)/2;
	     int rotY = (bottom-top)/2; */ 
	     int rotX = this.getWidth()/2;
	     int rotY = this.getHeight()/2; 
	     
	     canvas.rotate(180, rotX, rotY); 
	     super.onDraw(canvas);
	     canvas.restore();
	}
	
}
