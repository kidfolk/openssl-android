package org.kidfolk.androidRDP;

public class RdesktopNative {

	public boolean g_redirect = false;

	public static native String getenv();

	public static native int rdp_connect(String server, int flags, String domain,
			String password, String shell, String directory, boolean g_redirect);

	static {
		System.loadLibrary("rdesktop");
	}

}