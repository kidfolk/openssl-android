package org.kidfolk.androidRDP;

import java.nio.ByteBuffer;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.widget.ImageView;

public class AndroidRDPActivity extends Activity {
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(org.kidfolk.androidRDP.R.layout.main);
		handler = new Handler(){

			@Override
			public void handleMessage(Message msg) {
				
				runOnUiThread(new Runnable() {

					@Override
					public void run() {
						imageView.invalidate();
					}

				});
			}
			
		};
		
		DisplayMetrics metrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(metrics);
		setResolution(metrics.widthPixels, metrics.heightPixels);
		setResolution(1024, 768);
//		bitmap = Bitmap
//		.createBitmap(metrics.widthPixels, metrics.heightPixels, Config.RGB_565);
		bitmap = Bitmap
		.createBitmap(1024, 768, Config.RGB_565);
		canvas = new Canvas(bitmap);
		imageView = (ImageView) findViewById(R.id.image);
		imageView.setImageResource(R.drawable.icon);
		imageView.setImageBitmap(bitmap);
		setUsername("kidfolk");
		setPassword("xwjshow");
		setUsername("xuwj");
		setPassword("1");
		setServerDepth(16);
		// rdpdr_init();
		Thread rdesktopMainThread = new Thread(new Runnable() {

			@Override
			public void run() {
				//while (true) {
					rdp_reset_state();
//					int result = rdp_connect("192.168.3.115", 59, "", "", "",
//							"", false);
					int result = rdp_connect("192.168.3.207", 59, "", "", "",
							"", false);
					if (result == 0) {
						return;
					}
					rdp_main_loop();

					rdp_disconnect();
				//}

			}

		});
		rdesktopMainThread.start();
		cache_save_state();

	}

	@Override
	protected void onPause() {
		super.onPause();
		//rdp_disconnect();
	}
	
	private native void setResolution(int width,int height);

	private native String getenv();

	private native void setUsername(String username);

	private native void setPassword(String password);
	
	private native void setServerDepth(int server_depth);

	private native int rdp_connect(String server, int flags, String domain,
			String password, String shell, String directory, boolean g_redirect);

	private native int rdpdr_init();

	private native void rdp_main_loop();

	private native void rdp_disconnect();

	private native void rdp_reset_state();

	private native void cache_save_state();

	private native void rdp_send_client_window_status(int status);

	private native void rdp_send_input(int time, short message_type,
			short device_flags, short param1, short param2);

	private void renderBitmap(int x, int y, int cx, int cy, int width,
			int height, byte[] data) {
	
		ByteBuffer tmpBuffer = ByteBuffer.wrap(data);
		Bitmap tmpBitmap = Bitmap.createBitmap(width, height, Config.RGB_565);
		tmpBitmap.copyPixelsFromBuffer(tmpBuffer);
		canvas.drawBitmap(tmpBitmap, x, y, null);
		canvas.save(Canvas.ALL_SAVE_FLAG);
		canvas.restore();

		Message msg = new Message();
		this.handler.sendMessage(msg);

	}
	
	private void renderRect(int x,int y,int cx,int cy,int color){
		//Log.v(TAG, "renderRect");
		Rect rect = new Rect(x,y,x+cx,y+cy);
		Paint paint = new Paint();
		paint.setColor(color);
		paint.setStyle(Paint.Style.FILL);
		canvas.drawRect(rect, paint);
		
		Message msg = new Message();
		this.handler.sendMessage(msg);
	}

	private void renderLine(int startx, int starty, int endx, int endy,
			int color, int width, byte style) {
		//Log.v(TAG, "renderLine");
		Paint paint = new Paint();
		paint.setColor(color);
		paint.setStrokeWidth(width);
		paint.setStyle(Paint.Style.FILL);
		
		canvas.drawLine(startx, starty, endx, endy, paint);
		
		Message msg = new Message();
		this.handler.sendMessage(msg);
	}
	
	private void setPixel(int x, int y, int color)
    {
    	bitmap.setPixel(x, y, color);
    	
    	Message message = new Message();    
        this.handler.sendMessage(message);
    }

	private int count = 0;

	private ImageView imageView;

	private static Bitmap bitmap ;

	private static Canvas canvas ;
	
	private Handler handler;

	private static final String TAG = "AndroidRDPActivity";

	static {
		System.loadLibrary("rdesktop");
	}
}