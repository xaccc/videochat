package cn.videochat;

import cn.videochat.VideoChat.BufferingUpdateListener;
import cn.videochat.VideoChat.CompletionListener;
import cn.videochat.VideoChat.ErrorListener;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout.LayoutParams;

public class MainActivityDebug extends Activity implements OnClickListener, ErrorListener, CompletionListener, BufferingUpdateListener {
	
	final boolean debugmode = false;
	
	private VideoChat vc = null;
	private VideoChat.View mView = null;

	private Button btnPlay, btnStop;
	private EditText txtUserId;
	private EditText txtLog;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		// create render
		mView = new VideoChat.View(getApplication());
		addContentView(mView, new LayoutParams(LayoutParams.MATCH_PARENT, 480));

		// controls
		txtUserId = (EditText) findViewById(R.id.txtUrl);
		txtLog = (EditText) findViewById(R.id.txtInfo);
		txtUserId.setText("user123");
		btnPlay = (Button) findViewById(R.id.btnPlay);
		btnStop = (Button) findViewById(R.id.btnStop);
		
		txtLog.setEnabled(false);
		txtLog.setText("fuck,I'm not a empty!!!");
		
		if (debugmode){
			txtUserId.setVisibility(View.INVISIBLE);
			txtLog.setVisibility(View.INVISIBLE);
			btnPlay.setVisibility(View.INVISIBLE);
			btnStop.setVisibility(View.INVISIBLE); // default invisible stop
		}else{
			btnPlay.setVisibility(View.VISIBLE); // default invisible stop
			btnStop.setVisibility(View.INVISIBLE); // default invisible stop			
		}
		
		// Play
		btnPlay.setOnClickListener(this);
		// Stop
		btnStop.setOnClickListener(this);


		if (debugmode) {
			this.getCurrentFocus().post(new Runnable() {
				@Override
				public void run() {
					try {
						Thread.sleep(3000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					startPlay("user123");
				}
			});
		}
		
		// test fuck
		startPlay("user123");
	}

	
	private void startPlay(String room_id) {
		stopPlay();
		
		// Create VideoChat Instance
		vc = new VideoChat();	
		
		vc.addBufferingUpdateListener(this);
		vc.addCompletionListener(this);
		vc.addErrorListener(this);
		
		vc.setRender(mView);
		vc.setDataSource("admin", room_id);
		vc.play();

		if (!debugmode){
			btnPlay.setVisibility(View.INVISIBLE);
			btnStop.setVisibility(View.VISIBLE);
		}
	}
	

	private void stopPlay() {
		if (vc != null) {
			vc.release();
			vc = null;
			if (!debugmode) {
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
	
	protected void onPause()
	{
		stopPlay();
		super.onPause();
	}


	@Override
	public void onClick(View arg0) {
		switch(arg0.getId())
		{
		case R.id.btnPlay:
			if(!debugmode) startPlay(txtUserId.getText().toString());
			break;
		case R.id.btnStop:
			if(!debugmode) stopPlay();
			break;
		}
	}


	@Override
	public void onBufferingBegin(VideoChat player) {
		txtLog.post(new Runnable(){
			@Override
			public void run() {
				// TODO Auto-generated method stub
				int maxlength = Math.min(100, txtLog.getText().length()-1);
				txtLog.setText("开始缓冲...\n" + txtLog.getText().subSequence(0, maxlength));
			}
		});
	}


	@Override
	public void onBufferingComplete(VideoChat player) {
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
	public void onBufferingUpdate(VideoChat player, int bufferingProgress) {
		// TODO Auto-generated method stub
	}


	@Override
	public void onCompletionListener(VideoChat player) {
		txtLog.post(new Runnable(){
			@Override
			public void run() {
				// TODO Auto-generated method stub
				int maxlength = Math.min(100, txtLog.getText().length()-1);
				txtLog.setText("播放完成！\n" + txtLog.getText().subSequence(0, maxlength));
			}
		});
	}


	@Override
	public boolean onError(VideoChat player, int what, int extra) {
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

}

