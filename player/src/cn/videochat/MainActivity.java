package cn.videochat;

import cn.videochat.VideoChat.BufferingUpdateListener;
import cn.videochat.VideoChat.CompletionListener;
import cn.videochat.VideoChat.ErrorListener;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout.LayoutParams;

public class MainActivity extends Activity implements OnClickListener, ErrorListener, CompletionListener, BufferingUpdateListener {

	private VideoChat vc = null;
	private VideoChat.View mView = null;

	private Button btnPlay, btnStop;
	private EditText txtUserId;
	private EditText txtLog;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		// create render
		mView = new VideoChat.View(getApplication());
		addContentView(mView, new LayoutParams(LayoutParams.MATCH_PARENT, 480));

		// controls
		txtUserId = (EditText) findViewById(R.id.txtUrl);
		txtLog = (EditText) findViewById(R.id.txtInfo);
		txtUserId.setText("879905");
		btnPlay = (Button) findViewById(R.id.btnPlay);
		btnStop = (Button) findViewById(R.id.btnStop);
		
		txtLog.setEnabled(false);
		txtLog.setText("fuck,I'm not a empty!!!");

		btnPlay.setVisibility(View.VISIBLE); // default invisible stop
		btnStop.setVisibility(View.INVISIBLE); // default invisible stop

		// Play
		btnPlay.setOnClickListener(this);
		// Stop
		btnStop.setOnClickListener(this);

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

		btnPlay.setVisibility(View.INVISIBLE);
		btnStop.setVisibility(View.VISIBLE);
	}

	private void stopPlay() {
		if (vc != null) {
			vc.release();
			vc = null;
			btnPlay.setVisibility(View.VISIBLE);
			btnStop.setVisibility(View.INVISIBLE);
		}
	}
	
	protected void onPause()
	{
		stopPlay();
		super.onPause();
	}


	///////////////////////////////////////////////////////////////////////////
	//
	// 事件处理
	//
	///////////////////////////////////////////////////////////////////////////

	
	@Override
	public void onClick(View arg0) {
		switch(arg0.getId())
		{
		case R.id.btnPlay:
			startPlay(txtUserId.getText().toString());
			break;
		case R.id.btnStop:
			stopPlay();
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

