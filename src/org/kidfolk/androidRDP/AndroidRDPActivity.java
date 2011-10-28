package org.kidfolk.androidRDP;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.Date;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

public class AndroidRDPActivity extends Activity {
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		imageView = (ImageView) findViewById(R.id.image);
		imageView.setImageResource(R.drawable.icon);
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
		
	}

	private native String getenv();

	private native void setUsername(String username);

	private native void setPassword(String password);

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


	private void renderBitmap( int x, int y, int cx, int cy,
			int width, int height, byte[] data) {
//		Log.v(TAG, "getBitmapBytesFormNative in java");
//		Log.v(TAG, "x : " + x);
//		Log.v(TAG, "y : " + y);
//		Log.v(TAG, "width : " + width);
//		Log.v(TAG, "height : " + height);
//		Log.v(TAG, "data length : " + data.length);
		
		
	    Bitmap bitmap = Bitmap.createBitmap(width, height, Config.RGB_565);
		ByteBuffer buffer = ByteBuffer.wrap(data);
		bitmap.copyPixelsFromBuffer(buffer);
		
	}
	
	private ImageView imageView;
	
	private static final String TAG = "AndroidRDPActivity";

	static {
		System.loadLibrary("rdesktop");
	}
}