package org.kidfolk.androidRDP;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.ImageView;

public class RemoteView extends ImageView {

	public RemoteView(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
	}

	public RemoteView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		// TODO Auto-generated constructor stub
	}

	public RemoteView(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
	}

	@Override
	protected void onDraw(Canvas canvas) {
		// TODO Auto-generated method stub
		super.onDraw(canvas);

	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		int action = event.getAction();
		switch (action) {
		case MotionEvent.ACTION_DOWN:
			native_handle_mouse_button(event.getEventTime(),1,(int)event.getRawX(),(int)event.getRawY(),1);
			break;
		case MotionEvent.ACTION_UP:
			native_handle_mouse_button(event.getEventTime(),1,(int)event.getRawX(),(int)event.getRawY(),0);
			break;
		case MotionEvent.ACTION_MOVE:
			//native_handle_mouse_move((int)event.getX(),(int)event.getY());
			break;
		}
		return true;

	}
	
	//private native void native_handle_mouse_move(int x,int y);

	private native void native_handle_mouse_button(long time,int button,int x, int y, int down);

	private static final String TAG = "RemoteView";

}
