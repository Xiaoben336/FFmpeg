package com.example.zjf.ffmpeg;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
	private static final String TAG = "MainActivity";
	private EditText etVideoPath,etBgmPath,etOutPath;
	private Button btnAddBgm;
	private FFmpeg ffmpeg;
	private String videoPath,bgmPath,outPath;
	String BASE_PATH = Environment.getExternalStorageDirectory().getPath();

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		ffmpeg = new FFmpeg();
		initView();
		btnAddBgm.setOnClickListener(this);
	}

	private void initView() {
		etVideoPath = (EditText) findViewById(R.id.etVideoPath);
		etBgmPath = (EditText)findViewById(R.id.etBgmPath);
		etOutPath = (EditText) findViewById(R.id.etOutPath);
		btnAddBgm = (Button) findViewById(R.id.btnAddBgm);
	}


	@Override
	public void onClick(View v) {
		switch (v.getId()) {
			case R.id.btnAddBgm:
					videoPath = BASE_PATH + File.separator + etVideoPath.getText().toString();
					bgmPath = BASE_PATH + File.separator + etBgmPath.getText().toString();
					outPath = BASE_PATH + File.separator + etOutPath.getText().toString();
					new Thread(new Runnable() {
						@Override
						public void run() {
							Log.d(TAG,"videoPath === " + videoPath +"   bgmPath === " + bgmPath);
							ffmpeg.addBgm(videoPath,bgmPath,outPath);
						}
					}).start();
				break;
				default:
					break;
		}
	}
}
