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
	final boolean debug = false;
	VideoChat vc = null;
	VideoChat.View mView = null;

	Button btnPlay, btnStop;
	EditText txtRTMPUrl;
	EditText txtLog;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		// create render
		mView = new VideoChat.View(getApplication());
		addContentView(mView, new LayoutParams(LayoutParams.MATCH_PARENT, 480));

		// controls
		txtRTMPUrl = (EditText) findViewById(R.id.editText1);
		txtLog = (EditText) findViewById(R.id.editText2);
		txtRTMPUrl.setText("user123");
		btnPlay = (Button) findViewById(R.id.button1);
		btnStop = (Button) findViewById(R.id.button2);

		if (debug){
			txtRTMPUrl.setVisibility(View.INVISIBLE);
			txtLog.setVisibility(View.INVISIBLE);
			btnStop.setVisibility(View.INVISIBLE); // default invisible stop
		}else{
			btnStop.setVisibility(View.INVISIBLE); // default invisible stop			
		}
		btnPlay.setVisibility(View.VISIBLE); // default invisible stop
		txtLog.setText("fuck,I'm not a empty!!!");
		
		
		// Play
		btnPlay.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if (!debug) startPlay();
			}

		});

		// Stop
		btnStop.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if (!debug) stopPlay();
			}
		});

	}
	
	
	VideoChat.BufferingUpdateListener bufferUPdateListener = new VideoChat.BufferingUpdateListener() {
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
	};
	
	VideoChat.CompletionListener completionListener = new VideoChat.CompletionListener() {
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
	};
	
	VideoChat.ErrorListener errorListener = new VideoChat.ErrorListener() {
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
	};
	
	private void startPlay() {
		stopPlay();
		
		// Create VideoChat Instance
		vc = new VideoChat();	
		if (!debug){
			vc.addBufferingUpdateListener(bufferUPdateListener);
			vc.addCompletionListener(completionListener);
			vc.addErrorListener(errorListener);
			btnPlay.setVisibility(View.INVISIBLE);
			btnStop.setVisibility(View.VISIBLE);
		}
		vc.setRender(mView);
		vc.setDataSource("admin", txtRTMPUrl.getText().toString());
		vc.play();
	}
	

	private void stopPlay() {
		if (vc != null) {
			vc.release();
			vc = null;
			if (!debug) {
				btnPlay.setVisibility(View.VISIBLE);
				btnStop.setVisibility(View.INVISIBLE);
			}
		}
	}
	
	/*
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}
	 */
	
	protected void onResume()
	{
		super.onResume();
		
		Thread t = new Thread(new Runnable(){
			@Override
			public void run() {
				// TODO Auto-generated method stub
				try {
					Thread.sleep(3000);
					//while (mView == null || mView.render.getHandler() == 0) Thread.sleep(10);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				//btnPlay.performClick();
				
				//startPlay();
			}
		});
		t.run();
	}
	
	protected void onPause()
	{
		stopPlay();
		super.onPause();
	}
	
	
}
