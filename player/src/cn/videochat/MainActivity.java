package cn.videochat;

import cn.videochat.VideoChat.OnEventCallback;
import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {
	VideoChat vc;
	Button btnListen, btnStop;
	EditText txtRTMPUrl;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.activity_main);
		txtRTMPUrl = (EditText)findViewById(R.id.editText1);
		
		vc = new VideoChat();
		vc.Init();
		vc.setOnEventCallback(new OnEventCallback() {
			@Override
			public void onEvent(int event) {
			}
		});
		
		btnListen = (Button) findViewById(R.id.button1);
		btnListen.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				vc.OpenPlayer(txtRTMPUrl.getText().toString() + " live=1");
			}
		});

		btnStop = (Button) findViewById(R.id.button2);
		btnStop.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				vc.ClosePublisher();
				vc.ClosePlayer();
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
		super.onDestroy();

	}
}
