package com.example.zjf.ffmpeg;

public class FFmpeg {
	static {
		System.loadLibrary("avutil-55");
		System.loadLibrary("avcodec-57");
		System.loadLibrary("avformat-57");
		System.loadLibrary("avdevice-57");
		System.loadLibrary("swresample-2");
		System.loadLibrary("swscale-4");
		System.loadLibrary("postproc-54");
		System.loadLibrary("avfilter-6");
		System.loadLibrary("native-lib");
	}
	public native int addBgm(String videoUrl, String musicUrl,String outUrl);
	//public native int addBgm_(String videoUrl, String musicUrl,String outUrl);
}
