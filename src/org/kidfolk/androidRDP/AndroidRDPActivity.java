package org.kidfolk.androidRDP;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.ViewGroup.LayoutParams;
import android.widget.ImageView;
import android.widget.TextView;

public class AndroidRDPActivity extends Activity {
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		TextView text = (TextView) findViewById(R.id.text);
		setUsername("kidfolk");
		setPassword("xwjshow");
		rdpdr_init();
		Thread rdesktopMainThread = new Thread(new Runnable() {

			@Override
			public void run() {
				while (true) {
					rdp_reset_state();
					int result = rdp_connect("192.168.3.115", 51, "", "", "",
							"", false);
					if (result == 0) {
						return;
					}
					rdp_main_loop();
					
					rdp_disconnect();
				}

			}

		});
		rdesktopMainThread.start();
		cache_save_state();
		// String str = RdesktopNative.getenv();
		//text.setText(result + "");
	}

	private native String getenv();

	private native void setUsername(String username);

	private native void setPassword(String password);

	private native int rdp_connect(String server, int flags,
			String domain, String password, String shell, String directory,
			boolean g_redirect);

	private native int rdpdr_init();

	private native void rdp_main_loop();

	private native void rdp_disconnect();

	private native void rdp_reset_state();

	private native void cache_save_state();
	
	private native void renderBitmap(Bitmap  bitmap);

	private void getBitmapBytesFormNative(int x, int y,
			int width, int height,byte[] data){
		Log.v(TAG, "getBitmapBytesFormNative in java");
		Log.v(TAG, "x : "+x);
		Log.v(TAG, "y : "+y);
		Log.v(TAG, "width : "+width);
		Log.v(TAG, "height : "+height);
		Log.v(TAG, "data length : "+data.length);
		Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
		final ImageView view = new ImageView(this);
		LayoutParams params = new LayoutParams(LayoutParams.FILL_PARENT,LayoutParams.FILL_PARENT);
		view.setLayoutParams(params);
		view.setImageBitmap(bitmap);
		runOnUiThread(new Runnable(){

			@Override
			public void run() {
				setContentView(view);
			}
			
		});
		
	}
	
	private static final String TAG = "AndroidRDPActivity";

	static {
		System.loadLibrary("rdesktop");
	}
}