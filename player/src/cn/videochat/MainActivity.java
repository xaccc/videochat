package cn.videochat;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {
	VideoChat vc = null;
	VideoChat.View mView = null;

	Button btnPlay, btnStop;
	EditText txtRTMPUrl;
	EditText txtLog;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		// controls
		txtRTMPUrl = (EditText) findViewById(R.id.editText1);
		txtLog = (EditText) findViewById(R.id.editText2);
		txtRTMPUrl.setText("user123");
		btnPlay = (Button) findViewById(R.id.button1);
		btnStop = (Button) findViewById(R.id.button2);
		btnStop.setVisibility(View.INVISIBLE); // default invisible stop

		mView = new VideoChat.View(getApplication());
		addContentView(mView, new LayoutParams(LayoutParams.MATCH_PARENT, 480));

		txtLog.setText("fuck,I'm not a empty!!!");
		// Play
		btnPlay.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// Create VideoChat Instance
				vc = new VideoChat();
				vc.addBufferingUpdateListener(new VideoChat.BufferingUpdateListener() {
					@Override
					public void onBufferingUpdate(VideoChat player, int bufferingProgress) {
						// TODO Auto-generated method stub
					}
					@Override
					public void onBufferingComplete(VideoChat player) {
						// TODO Auto-generated method stub
						txtLog.post(new Runnable(){
							@Override
							public void run() {
								// TODO Auto-generated method stub
								int maxlength = Math.min(100, txtLog.getText().length()-1);
								txtLog.setText("缓冲完成，开始播放..." + "\n" + txtLog.getText().subSequence(0, maxlength));
							}
						});
					}
					@Override
					public void onBufferingBegin(VideoChat player) {
						// TODO Auto-generated method stub
						txtLog.post(new Runnable(){
							@Override
							public void run() {
								// TODO Auto-generated method stub
								int maxlength = Math.min(100, txtLog.getText().length()-1);
								txtLog.setText("开始缓冲...\n" + txtLog.getText().subSequence(0, maxlength));
							}
						});
					}
				});
				vc.addCompletionListener(new VideoChat.CompletionListener() {
					@Override
					public void onCompletionListener(VideoChat player) {
						// TODO Auto-generated method stub
						txtLog.post(new Runnable(){
							@Override
							public void run() {
								// TODO Auto-generated method stub
								int maxlength = Math.min(100, txtLog.getText().length()-1);
								txtLog.setText("播放完成！\n" + txtLog.getText().subSequence(0, maxlength));
							}
						});
					}
				});
				vc.addErrorListener(new VideoChat.ErrorListener() {
					@Override
					public boolean onError(VideoChat player, int what, int extra) {
						// TODO Auto-generated method stub
						txtLog.post(new Runnable(){
							@Override
							public void run() {
								// TODO Auto-generated method stub
								int maxlength = Math.min(100, txtLog.getText().length()-1);
								txtLog.setText("播放器错误\n" + txtLog.getText().subSequence(0, maxlength));
							}
						});
						return false;
					}
				});
				vc.setRender(mView);
				vc.setDataSource("admin", txtRTMPUrl.getText().toString());
				vc.play();
				btnPlay.setVisibility(View.INVISIBLE);
				btnStop.setVisibility(View.VISIBLE);
			}
		});

		// Stop
		btnStop.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				vc.release();
				vc = null;
				btnPlay.setVisibility(View.VISIBLE);
				btnStop.setVisibility(View.INVISIBLE);
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	@Override
	protected void onDestroy() {
		if (vc != null) {
			vc.release();
			vc = null;
		}
		super.onDestroy();
	}
	
}
