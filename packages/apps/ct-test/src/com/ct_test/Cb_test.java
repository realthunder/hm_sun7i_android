package com.cb_test2;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.ConnectivityManager;
import android.net.ethernet.EthernetManager;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.FileObserver;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.text.format.DateFormat;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;


public class Cb_test extends Activity {
	private static TextView usb1_yn,usb2_yn,internet_yn,sdcard_yn,sata_yn,battery_yn,time_yn;
	private Button ceshi,/*uninstall,*/shift1,/*shift2,*/shift3,shift4;
	private static FileObserver usb1FileObserver,usb2FileObserver,sataFileObserver,sdFileObserver;
	private MediaPlayer mediaPlayer;
	private static AudioManager audiom;
	private BroadcastReceiver mReceiver = null;
	//private WifiManager wifiManager;
	private static final int msgKey1 = 1;
	private EthernetManager mEthManager;
	private static String TAG = "CB";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_cb_test);
		usb1_yn=(TextView)findViewById(R.id.usb1_yn);
		internet_yn=(TextView)findViewById(R.id.internet_yn);
		sdcard_yn=(TextView)findViewById(R.id.sdcard_yn);
		sata_yn=(TextView)findViewById(R.id.sata_yn);
		battery_yn=(TextView)findViewById(R.id.battery_yn);
		time_yn=(TextView)findViewById(R.id.time_yn);
		
		ceshi=(Button)findViewById(R.id.ceshi);		
		//uninstall=(Button)findViewById(R.id.uninstall);
		shift1=(Button)findViewById(R.id.shift_1);
		//shift2=(Button)findViewById(R.id.shift_2);
		shift3=(Button)findViewById(R.id.shift_3);
		shift4=(Button)findViewById(R.id.shift_4);
		
		audiom=(AudioManager)getSystemService(Context.AUDIO_SERVICE);
		//wifiManager = (WifiManager) this.getSystemService(Context.WIFI_SERVICE);
		
		new TimeThread().start();
		mEthManager = EthernetManager.getInstance();
		mEthManager.setEnabled(true);
		
		//----------------自动播放音乐---------------------
		mediaPlayer=MediaPlayer.create(this, R.raw.music);
		mediaPlayer.start();
		registerReceivers();
				
		/*//-----------------usb 1监听-----------------------------
		  final Runnable usb1runnable = new Runnable(){	
		  @Override	
		  public void run() {
		  String TAG="TEST usb1";
		  if(null == usb1FileObserver) {	
		  usb1FileObserver = new usb1listener("/sys/block/sda");	
		  usb1FileObserver.startWatching();	
		  Log.d(TAG,"--------start listener----------");	
		  }	
		  }  
		  };
		
		  //-----------------usb 2监听----------------------------
		  final Runnable usb2runnable = new Runnable(){	
		  @Override	
		  public void run() {
		  String TAG="TEST usb2";
		  if(null == usb2FileObserver) {	
		  usb2FileObserver = new usb2listener("/sys/block/sdb");	
		  usb2FileObserver.startWatching();	
		  Log.d(TAG,"--------start listener----------");	
		  }	
		  }  
		  };
		
		  //----------------sd card监听------------------------
		  final Runnable sdrunnable = new Runnable(){	
		  @Override	
		  public void run() {
		  String TAG="TEST sdcard";
		  if(null == sdFileObserver) {	
		  sdFileObserver = new sdlistener("/sys/block/mmcblk0");	
		  sdFileObserver.startWatching();	
		  Log.d(TAG,"--------start listener----------");	
		  }	
		  }  
		  };
		
		  //----------------sata监听----------------------------
		  final Runnable satarunnable = new Runnable(){	
		  @Override	
		  public void run() {
		  String TAG="TEST sata";
		  if(null == sataFileObserver) {	
		  sataFileObserver = new satalistener("/sys/block/sdc");	
		  sataFileObserver.startWatching();	
		  Log.d(TAG,"--------start listener----------");	
		  }	
		  }  
		  };*/
		
		//------------按键开始测试---------------------------
		ceshi.setOnClickListener(new Button.OnClickListener(){	
				@Override	
				public void onClick(View v) {					
					/*new Thread(usb2runnable).start();
					  new Thread(satarunnable).start();
					  new Thread(usb1runnable).start();
					  new Thread(sdrunnable).start();
					  new Thread(sdrunnable).start();*/
					mediaPlayer.pause();
					/*usb1FileObserver = new usb1listener("/sys/block/sda");	
					  usb1FileObserver.startWatching();
					  usb2FileObserver = new usb2listener("/sys/block/sdb");	
					  usb2FileObserver.startWatching();*/
					sdFileObserver = new sdlistener("/sys/block/mmcblk0");	
					sdFileObserver.startWatching();
					sataFileObserver = new satalistener("/sys/block/sda");	
					sataFileObserver.startWatching();
					try {
						/*execCommand("cd /sys/block/sdb && ls");
						  execCommand("cd /sys/block/sdc && ls");*/
						execCommand("cd /sys/block/sda && ls");
						execCommand("cd /sys/block/mmcblk0 && ls");
					} catch (Exception e) {
						e.printStackTrace();
					}
					if(isOpenNetwork()){	
						internet_yn.setText("OK");	
					}else{	
						internet_yn.setText("NO");	
					}
						
					//wifiManager.setWifiEnabled(true);	  
							
					registerReceiver(breceiver,new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
				}			
			});
				
		//--------------------按键卸载---------------------
		/*uninstall.setOnClickListener(new Button.OnClickListener(){	
		  @Override	
		  public void onClick(View v) {					
		  unInstall();	
		  }			
		  });*/
		
		//--------------------按键切换音频CODEC--------------
		shift1.setOnClickListener(new Button.OnClickListener(){
				@Override
				public void onClick(View v) {
					audiom.setParameters("audio_devices_out_active=AUDIO_CODEC");
					mediaPlayer.start();
				}					
			});
		
		//--------------------按键切换音频HDMI--------------
		/*shift2.setOnClickListener(new Button.OnClickListener(){
		  @Override
		  public void onClick(View v) {
		  audiom.setParameters("audio_devices_out_active=AUDIO_HDMI");
		  mediaPlayer.start();
		  }					
		  });*/
				
		//--------------------按键切换音频SPDIF--------------
		shift3.setOnClickListener(new Button.OnClickListener(){
				@Override
				public void onClick(View v) {
					audiom.setParameters("audio_devices_out_active=AUDIO_SPDIF");
					mediaPlayer.start();
				}					
			});
		
		//-------------------按键切换到wifi，蓝牙界面-----------
		shift4.setOnClickListener(new Button.OnClickListener(){
				@Override
				public void onClick(View v) {
					set();
				}					
			});
	}
	
	public class TimeThread extends Thread {
		@Override
		public void run () {
			do {
				try {
					Thread.sleep(1000);
					Message msg = new Message();
					msg.what = msgKey1;
					timeHandler.sendMessage(msg);
				}
				catch (InterruptedException e) {
					e.printStackTrace();
				}
			} while(true);
		}
	}

	private Handler timeHandler = new Handler() {
			@Override
			public void handleMessage (Message msg) {
				super.handleMessage(msg);
				switch (msg.what) {
				case msgKey1:
					long sysTime = System.currentTimeMillis();
					CharSequence sysTimeStr = DateFormat.format("hh:mm:ss", sysTime);
					time_yn.setText(sysTimeStr);
					break;
                
				default:
					break;
				}
			}
		};
	
	//--------------------电池-------------------------
	private BroadcastReceiver breceiver=new BroadcastReceiver(){
			public void onReceive(Context context, Intent intent) {
				String action=intent.getAction();
				Log.d("Battery" ,"---------Changed------");
				if(Intent.ACTION_BATTERY_CHANGED.equals(action)){
					int level=intent.getIntExtra("level", 0);
					int scale=intent.getIntExtra("scale", 100);
					battery_yn.setText(String.valueOf(level*100/scale)+"%");
				}
			}		
		};
	
	//---------------------注册广播---------------------
	private void registerReceivers() { 		
		mReceiver = new BroadcastReceiver() {          	
				@Override  
				public void onReceive(Context context, Intent intent) {                  	
					String TAG="Test";
					Log.d(TAG, "enter------"); 
					final String action = intent.getAction();  
					Log.d(TAG, "getaction------"); 
					Log.d(TAG, "receive action = " + action);
					final String path = intent.getData().getPath();  
					Log.d(TAG, "external storage path = " + path);
					if(Intent.ACTION_BATTERY_CHANGED.equals(action)){
						battery_yn.setText("OK");
					}
					if(Intent.ACTION_MEDIA_CHECKING.equals(action)&&(path.contains("/mnt/usbhost0")||path.contains("/mnt/usbhost1"))){
						usb1_yn.setText("OK");
					}
					if(Intent.ACTION_MEDIA_REMOVED.equals(action)&&(path.contains("/mnt/usbhost0")||path.contains("/mnt/usbhost1"))){
						usb1_yn.setText("NO");
					}
				}
			};

		final IntentFilter filter = new IntentFilter();  
		filter.addAction(Intent.ACTION_BATTERY_CHANGED);
		filter.addAction(Intent.ACTION_MEDIA_CHECKING);
		filter.addAction(Intent.ACTION_MEDIA_REMOVED); 
		filter.addDataScheme("file");  
		registerReceiver(mReceiver, filter); 
	}
	
	//--------------判断网络是否连接上----------------------
	private boolean isOpenNetwork() {  
		ConnectivityManager connManager = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);  
		if(connManager.getActiveNetworkInfo() != null) {  
		        return connManager.getActiveNetworkInfo().isAvailable();  
		}  		    
		return false;  
	}  
    
	//-----------------监听usb 1--------------------------
	static class usb1listener extends FileObserver {     	
		static final String TAG = "usb1";			
		public usb1listener(String path)   
			{   
				super(path);   
				Log.w(TAG, "create");   			
			}	
		@Override    
		public void onEvent(int event, String path) { 		
			Log.d(TAG, "------------enter---------");
			Runnable runnable = new Runnable(){
					@Override
					public void run() {
						Message msg = usb1Handler.obtainMessage();  
						msg.what = 1;  
						msg.sendToTarget();  
					}  
				};new Thread(runnable).start();
			Log.d(TAG, "----------usb1 ok---------");
		}      	
	}  	
	static Handler usb1Handler = new Handler() {  
			@Override  
			public void handleMessage(Message msg) {  
				if(msg.what == 1) {  
					usb1_yn.setText("OK"); 
				}  
				super.handleMessage(msg);
				usb1FileObserver.stopWatching();
			}  
		};	
		
	//-----------------监听usb 2--------------------------
	static class usb2listener extends FileObserver {     	
		static final String TAG = "usb2";			
		public usb2listener(String path)   
			{   
				super(path);   
				Log.w(TAG, "create");   			
			}	
		@Override    
		public void onEvent(int event, String path) { 		
			Log.d(TAG, "------------enter---------");
			Runnable runnable = new Runnable(){
					@Override
					public void run() {
						Message msg = usb2Handler.obtainMessage();  
						msg.what = 1;  
						msg.sendToTarget();  
					}  
				};new Thread(runnable).start();
			Log.d(TAG, "----------SD-CARD ok---------");
		}      	
	}  	
	static Handler usb2Handler = new Handler() {  
			@Override  
			public void handleMessage(Message msg) {  
				if(msg.what == 1) {  
					usb2_yn.setText("OK"); 
				}  
				super.handleMessage(msg); 
				usb2FileObserver.stopWatching();
			}  
		};
    
	//-----------------监听sata--------------------------
	static class satalistener extends FileObserver {     	
		static final String TAG = "sata";			
		public satalistener(String path)   
			{   
				super(path);   
				Log.w(TAG, "create");   			
			}	
		@Override    
		public void onEvent(int event, String path) { 		
			Log.d(TAG, "------------enter---------");
			Runnable runnable = new Runnable(){
					@Override
					public void run() {
						Message msg = sataHandler.obtainMessage();  
						msg.what = 1;  
						msg.sendToTarget();  
					}  
				};new Thread(runnable).start();
			Log.d(TAG, "----------SATA ok---------");
		}      	
	}  	
	static Handler sataHandler = new Handler() {  
			@Override  
			public void handleMessage(Message msg) {  
				if(msg.what == 1) {  
					sata_yn.setText("OK"); 
				}  
				super.handleMessage(msg);
				sataFileObserver.stopWatching();
			}  
		};
    
	//-----------------监听sata--------------------------
	static class sdlistener extends FileObserver {     	
		static final String TAG = "sdcard";			
		public sdlistener(String path)   
			{   
				super(path);   
				Log.w(TAG, "create");   			
			}	
		@Override    
		public void onEvent(int event, String path) { 		
			Log.d(TAG, "------------enter---------");
			Runnable runnable = new Runnable(){
					@Override
					public void run() {
						Message msg = sdHandler.obtainMessage();  
						msg.what = 1;  
						msg.sendToTarget();  
					}  
				};new Thread(runnable).start();
			Log.d(TAG, "----------sdcard ok---------");
		}      	
	}  	
	static Handler sdHandler = new Handler() {  
			@Override  
			public void handleMessage(Message msg) {  
				if(msg.what == 1) {  
					sdcard_yn.setText("OK"); 
				}  
				super.handleMessage(msg);
				sdFileObserver.stopWatching();
			}  
		};
    
	//----------------命令----------------------
	public void execCommand(String p1)throws Exception{
		AaRootAccess acc = new AaRootAccess();
		acc.execute(p1);
	}
    
	//-----------------卸载apk的方法------------------------
	public void unInstall(){
		Uri packageURI = Uri.parse("package:com.cb_test2");
		Intent intent = new Intent(Intent.ACTION_DELETE,packageURI);
		startActivity(intent);
	} 

	//-----------------跳转wifi设置界面---------------------
	public void set(){
		Intent intent = new Intent(Settings.ACTION_WIFI_SETTINGS);
		startActivity(intent);
	}
  
	//-----------------取消注册--------------------------
	private void unregisterReceivers() {  
		if (mReceiver != null) {
			unregisterReceiver(mReceiver);
			mReceiver = null;
		}
	}  
    
	//---------------------onDestroy------------------------
	public void onDestroy() { 

		Log.d(TAG, "onDestroy");  

		unregisterReceivers();

		/* battery */
		if (breceiver != null) {
			unregisterReceiver(breceiver);
			breceiver = null;
		}

		if (usb1FileObserver != null) {
			usb1FileObserver.stopWatching();
			usb1FileObserver = null;
		}

		if (usb2FileObserver != null) {
			usb2FileObserver.stopWatching();
			usb2FileObserver = null;
		}

		if (sataFileObserver != null) {
			sataFileObserver.stopWatching();
			sataFileObserver = null;
		}

		if (sdFileObserver != null) {
			sdFileObserver.stopWatching();
			sdFileObserver = null;
		}

		super.onDestroy();
	} 
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.cb_test, menu);
		return true;
	}
}
