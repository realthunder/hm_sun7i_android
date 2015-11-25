package com.ct_dd;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import android.os.Bundle;
import android.os.FileObserver;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ToggleButton;

public class Main extends Activity {
	static final String TAG="sata";
	public ProgressDialog d1=null,d2=null;
	private static Button btnmkfs,btndd,btnmkfs2;
	private Button btnfdisk;
	private ToggleButton tbtn1;
	private static Builder a1_y,a1_n,a2_y,a2_n,a3;
	static String c=null;
	static String p=null;
	private static String sata_path="/sys/devices/platform/sw_ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/";
						 
	private static FileObserver sataFileObserver1,sataFileObserver2,sataFileObserver3,sataFileObserver4;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.main);
        btnmkfs=(Button)findViewById(R.id.btnmkfs);
        btnmkfs2=(Button)findViewById(R.id.btnmkfs2);
        btnfdisk=(Button)findViewById(R.id.btnfdisk);
        btndd=(Button)findViewById(R.id.btndd);
        tbtn1=(ToggleButton)findViewById(R.id.tbtn1);
        int i=Settings.System.getInt(this.getContentResolver(),"CT-Tools",0);
        if(i==1)
        	tbtn1.setChecked(true);
        else
        	tbtn1.setChecked(false);
        Log.d(TAG,"start----------");
    	try {
			check();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
    	
        a1_y=new AlertDialog.Builder(Main.this)
        .setTitle(R.string.atitle)
        .setMessage(R.string.msg2)
        .setPositiveButton(R.string.yes,
        		new DialogInterface.OnClickListener() {
        	@Override
			public void onClick(DialogInterface arg0, int arg1) {
			}
		});
        
		a1_n=new AlertDialog.Builder(Main.this)
		.setTitle(R.string.atitle)
		.setMessage(R.string.msg3)
		.setPositiveButton(R.string.yes,
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
					}
				});
		a2_y=new AlertDialog.Builder(Main.this)
        .setTitle(R.string.atitle)
        .setMessage(R.string.msg4)
        .setPositiveButton(R.string.yes,
        		new DialogInterface.OnClickListener() {
        	@Override
			public void onClick(DialogInterface arg0, int arg1) {
			}
		});
        
		a2_n=new AlertDialog.Builder(Main.this)
		.setTitle(R.string.atitle)
		.setMessage(R.string.msg5)
		.setPositiveButton(R.string.yes,
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
					}
				});
		
		a3=new AlertDialog.Builder(Main.this)
		.setTitle(R.string.atitle)
		.setMessage(R.string.smsg5)
		.setPositiveButton(R.string.yes,
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
					}
				});
		
        btnmkfs.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View v) {
				Log.d(TAG,"click on------");
				new AlertDialog.Builder(Main.this)
				.setTitle(R.string.atitle)
				.setMessage(R.string.msg1)
				.setPositiveButton(R.string.yes,new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						try {
							start();
							mkfs(p);
						} catch (IOException e) {
							e.printStackTrace();
						}
						c=null;p=null;
					}
				}).show();
			}        	
        });


        btnmkfs2.setOnClickListener(new OnClickListener(){
                        @Override
                        public void onClick(View v) {
                                Log.d(TAG,"click on------");
                                new AlertDialog.Builder(Main.this)
                                .setTitle(R.string.atitle)
                                .setMessage(R.string.msg1)
                                .setPositiveButton(R.string.yes,new DialogInterface.OnClickListener() {                        
                                        @Override
                                        public void onClick(DialogInterface dialog, int which) {
                                                try {
                                                        start();
                                                        mkfs2(p);
                                                } catch (IOException e) {
                                                        e.printStackTrace();
                                                }
                                                c=null;p=null;
                                        }
                                }).show();
                        }
        });

        
        btnfdisk.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View v) {
				Intent intent=new Intent();
				intent.setClass(Main.this, Select.class);
				startActivityForResult(intent,0);
			}        	
        });
        
        btndd.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View v) {
						try {
							start();
							cpSystemFile(p,1);
						} catch (IOException e) {
							e.printStackTrace();
						}
						c=null;p=null;
					}       	
        });
        tbtn1.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View arg0) {	
				if(tbtn1.isChecked()==true){
					try {
						magic("w");
					} catch (IOException e) {
						e.printStackTrace();
					}
				}else{
					try {
						magic("d");
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				Settings.System.putInt(getContentResolver(), "CT-Tools", tbtn1.isChecked()?1:0);
			}			
        });
     
    }
    
  //-----------------------------sda--------------------------
  	static class sda extends FileObserver{
  		static final String TAG="sata";
  		public sda(String o)
  		{
  			super(sata_path+o);
  			
  			Log.d(TAG,"-----sda create-----");
  		}
  		@Override
  		public void onEvent(int event,String path)
  		{
  			Log.d(TAG,"-------feel sda--------");
  			p="sda";
  			sataFileObserver1.stopWatching();
  			sataFileObserver2.stopWatching();
  			sataFileObserver3.stopWatching();
  			sataFileObserver4.stopWatching();
  		}
  	}
  	
  	//--------------------------sdb------------------------------------
  	static class sdb extends FileObserver{
  		static final String TAG="sdb";
  		public sdb(String o)
  		{
  			super(sata_path+o);
  			
  			Log.d(TAG,"-----sdb create-----");
  		}        
  		@Override
  		public void onEvent(int event,String path)
  		{
  			Log.d(TAG,"-------feel sdb--------");
  			p="sdb";
  			sataFileObserver1.stopWatching();
  			sataFileObserver2.stopWatching();
  			sataFileObserver3.stopWatching();
  			sataFileObserver4.stopWatching();	
  		}
  	}
  	
  	
  	//----------------------------sdc-----------------------------
  	static class sdc extends FileObserver{
  		static final String TAG="sdc";
  		public sdc(String o)
  		{
  			super(sata_path+o);
  			
  			Log.d(TAG,"-----sdc create-----");
  		}
  		@Override
  		public void onEvent(int event,String path)
  		{
  			Log.d(TAG,"-------feel sdc--------");
  			p="sdc";
  			sataFileObserver1.stopWatching();
  			sataFileObserver2.stopWatching();
  			sataFileObserver3.stopWatching();
  			sataFileObserver4.stopWatching();
  		}
  	}
  	
  	//-----------------------------------sdd----------------------
  	static class sdd extends FileObserver{
  		static final String TAG="sdd";
  		public sdd(String o)
  		{
  			super(sata_path+o);
  			
  			Log.d(TAG,"-----sdd create-----");
  		}
  		@Override
  		public void onEvent(int event,String path)
  		{
  			Log.d(TAG,"-------feel sdd--------");
  			p="sdd";
  			sataFileObserver1.stopWatching();
  			sataFileObserver2.stopWatching();
  			sataFileObserver3.stopWatching();
  			sataFileObserver4.stopWatching();
  		}
  	}

	//-----------------------mkfs command--------------------------
	public void mkfs(final String s)throws IOException{
		d1=ProgressDialog.show(Main.this, getString(R.string.dtitle),
				getString(R.string.d1body),true);
		new Thread(){
			public void run(){
				try{
					ArrayList<String> list=new ArrayList<String>();
					list.add("sh /system/bin/shmkfs.sh "+s);
					ArrayList<String> commands=list;
					Process suPro=Runtime.getRuntime().exec("su");
					DataOutputStream os=new DataOutputStream(suPro.getOutputStream());
					InputStream is=suPro.getInputStream();
					InputStreamReader isread=new InputStreamReader(is);
					BufferedReader br=new BufferedReader(isread);
					String line="";
					StringBuilder sb=new StringBuilder(line);
					for(String currentCommand:commands){
						Log.d(TAG,currentCommand);
						os.writeBytes(currentCommand+"\n");
						os.flush();
						os.writeBytes("exit\n");
						os.flush();
					}
					while((line=br.readLine())!=null){
						sb.append(line);
						sb.append("\n");
						Log.d(TAG,sb.toString());
					}
					if(sb.toString().contains("mkfs=0"))
					{
						d1.dismiss();
						Runnable runnable=new Runnable(){
							@Override
								public void run() {
									Message msg=Handler.obtainMessage();
									msg.what=1;
									msg.sendToTarget();
								}	
						};new Thread(runnable).start();}
					if(sb.toString().contains("mkfs=1"))
					{
						d1.dismiss();
						Runnable runnable=new Runnable(){
							@Override
								public void run() {
									Message msg=Handler.obtainMessage();
									msg.what=2;
									msg.sendToTarget();
								}	
						};new Thread(runnable).start();}   
				}catch(Exception ex){   
					d1.dismiss();   
					Runnable runnable=new Runnable(){  
						@Override
							public void run() {
								Message msg=Handler.obtainMessage();
								msg.what=2;
								msg.sendToTarget();
							}
					};new Thread(runnable).start();
				}
			}
		}.start();
	}

	//-----------------------mkfs2 command--------------------------
	public void mkfs2(final String s)throws IOException{
		d1=ProgressDialog.show(Main.this, getString(R.string.dtitle),
				getString(R.string.d1body),true);
		new Thread(){
			public void run(){
				try{
					ArrayList<String> list=new ArrayList<String>();
					list.add("sh /system/bin/shmkfs2.sh "+s);
					ArrayList<String> commands=list;
					Process suPro=Runtime.getRuntime().exec("su");
					DataOutputStream os=new DataOutputStream(suPro.getOutputStream());
					InputStream is=suPro.getInputStream();
					InputStreamReader isread=new InputStreamReader(is);
					BufferedReader br=new BufferedReader(isread);
					String line="";
					StringBuilder sb=new StringBuilder(line);
					for(String currentCommand:commands){
						Log.d(TAG,currentCommand);
						os.writeBytes(currentCommand+"\n");
						os.flush();
						os.writeBytes("exit\n");
						os.flush();
					}
					while((line=br.readLine())!=null){
						sb.append(line);
						sb.append("\n");
						Log.d(TAG,sb.toString());
					}
					if(sb.toString().contains("mkfs=0"))
					{
						d1.dismiss();
						Runnable runnable=new Runnable(){
							@Override
								public void run() {
									Message msg=Handler.obtainMessage();
									msg.what=1;
									msg.sendToTarget();
								}	
						};new Thread(runnable).start();}
					if(sb.toString().contains("mkfs=1"))
					{
						d1.dismiss();
						Runnable runnable=new Runnable(){
							@Override
								public void run() {
									Message msg=Handler.obtainMessage();
									msg.what=2;
									msg.sendToTarget();
								}	
						};new Thread(runnable).start();}   
				}catch(Exception ex){   
					d1.dismiss();   
					Runnable runnable=new Runnable(){  
						@Override
							public void run() {
								Message msg=Handler.obtainMessage();
								msg.what=2;
								msg.sendToTarget();
							}
					};new Thread(runnable).start();
				}
			}
		}.start();
	}

    //-----------------------copy system file to sata--------------------------
    public void cpSystemFile(final String s,final int n)throws IOException{
    	d2=ProgressDialog.show(Main.this, getString(R.string.dtitle),
    			                          getString(R.string.d2body),true);
    	new Thread(){
    		public void run(){
    			try{
    				ArrayList<String> list=new ArrayList<String>();
    				list.add("sh /system/bin/shcp.sh "+s+" "+n);
    				ArrayList<String> commands=list;
    				Process suPro=Runtime.getRuntime().exec("su");
    				DataOutputStream os=new DataOutputStream(suPro.getOutputStream());
    				InputStream is=suPro.getInputStream();
    				InputStreamReader isread=new InputStreamReader(is);
    				BufferedReader br=new BufferedReader(isread);
    				String line="";
    				StringBuilder sb=new StringBuilder(line);
    				for(String currentCommand:commands){
    					Log.d(TAG,currentCommand);
    					os.writeBytes(currentCommand+"\n");
    					os.flush();
    					os.writeBytes("exit\n");
    					os.flush();
    				}
    				while((line=br.readLine())!=null){
    					sb.append(line);
    					sb.append("\n");
    					Log.d(TAG,sb.toString());
    				}
    				if(sb.toString().contains("all finish cp=0"))
    				{
    					d2.dismiss();
    					Runnable runnable=new Runnable(){
    						@Override
    						public void run() {
    							Message msg=Handler.obtainMessage();
    							msg.what=3;
    							msg.sendToTarget();
    						}
        				};new Thread(runnable).start();
    				}
    				if(sb.toString().contains("cp=1")){
    					d2.dismiss();
    					Runnable runnable=new Runnable(){
    						@Override
    						public void run() {
    							Message msg=Handler.obtainMessage();
    							msg.what=4;
    							msg.sendToTarget();
    						}
        				};new Thread(runnable).start();
    				}
    				if(sb.toString().contains("no sata")){
    					d2.dismiss();
    					Runnable runnable=new Runnable(){
    						@Override
    						public void run() {
    							Message msg=Handler.obtainMessage();
    							msg.what=5;
    							msg.sendToTarget();
    						}
        				};new Thread(runnable).start();
    				}
    			}catch(Exception ex){
    				d2.dismiss();
    				Runnable runnable=new Runnable(){
						@Override
						public void run() {
							Message msg=Handler.obtainMessage();
							msg.what=4;
							msg.sendToTarget();
						}
    				};new Thread(runnable).start();
    			}
    		}
    	}.start();
    }
    
    static Handler Handler=new Handler(){
    	@Override
    	public void handleMessage(Message msg){
    		if(msg.what==1){
    			a1_y.show();
    		}
    		if(msg.what==2){
    			a1_n.show();
    		}
    		if(msg.what==3){
    			a2_y.show();
    		}
    		if(msg.what==4){
    			a2_n.show();
    		}
    		if(msg.what==5){
    			a3.show();
    		}
    		super.handleMessage(msg);
    	}
    };
    
    //---------------------ls command-------------------
    public static void ls(final String bl) throws IOException{
    	ArrayList<String> list=new ArrayList<String>();
		list.add("cd "+sata_path+bl+" && "+"ls "+sata_path+bl);
		ArrayList<String> commands=list;
		Process suPro=Runtime.getRuntime().exec("su");
		DataOutputStream os=new DataOutputStream(suPro.getOutputStream());
		for(String currentCommand:commands){
			os.writeBytes(currentCommand+"\n");
			os.flush();
			}
			os.writeBytes("exit\n");
			os.flush();
    }
    
    //-------------add sata magic-----------------------------------------
    public static void magic(String o) throws IOException{
    	try{
    	ArrayList<String> list=new ArrayList<String>();
		list.add("cubie_sata_magic -"+o);
		ArrayList<String> commands=list;
		Process suPro=Runtime.getRuntime().exec("su");
		DataOutputStream os=new DataOutputStream(suPro.getOutputStream());
		InputStream is=suPro.getInputStream();
		InputStreamReader isread=new InputStreamReader(is);
		BufferedReader br=new BufferedReader(isread);
		String line="";
		StringBuilder sb=new StringBuilder(line);
		for(String currentCommand:commands){
			Log.d(TAG,currentCommand);
			os.writeBytes(currentCommand+"\n");
			os.flush();
			}
			os.writeBytes("exit\n");
			os.flush();
		    Log.d(TAG,"magic");
		    while((line=br.readLine())!=null){
				sb.append(line);
				sb.append("\n");
				Log.d(TAG,sb.toString());
		    }
		    }catch(Exception ex){}
    }
    
    //----------------check system-----------------
    public static void check() throws IOException{
    	try{
    		ArrayList<String> list=new ArrayList<String>();
    		list.add("mount |grep 'system'");
    		ArrayList<String> commands=list;
    		Process suPro=Runtime.getRuntime().exec("su");
    		DataOutputStream os=new DataOutputStream(suPro.getOutputStream());
    		InputStream is=suPro.getInputStream();
    		InputStreamReader isread=new InputStreamReader(is);
    		BufferedReader br=new BufferedReader(isread);
    		String line="";
    		StringBuilder sb=new StringBuilder(line);
    		for(String currentCommand:commands){
    			Log.d(TAG,currentCommand);
    			os.writeBytes(currentCommand+"\n");
    			os.flush();
			}
			os.writeBytes("exit\n");
			os.flush();		 
		    while((line=br.readLine())!=null){
				sb.append(line);
				sb.append("\n");
				Log.d(TAG,sb.toString());
			}
		    if(sb.toString().contains("/dev/block/sda1 /system ext4 rw,nodev,noatime,nobarrier,data=ordered 0 0"))
			{
		    	Log.d(TAG,"in 1");
	            btnmkfs.setEnabled(false);
	            btndd.setEnabled(false);
			}
		    if(sb.toString().contains("/dev/block/system /system ext4 rw,nodev,noatime,nobarrier,data=ordered 0 0"))
		    {
		    	Log.d(TAG,"in 2");
		    	btnmkfs.setEnabled(true);
		    	btndd.setEnabled(true);
		    }
		    }catch(Exception ex){}
    }
    
    //----------start check path of sata----------------
    public static void start() throws IOException{
    	sataFileObserver1=new sda("sda");
    	sataFileObserver1.startWatching();
    	sataFileObserver2=new sdb("sdb");
    	sataFileObserver2.startWatching();
    	sataFileObserver3=new sdc("sdc");
    	sataFileObserver3.startWatching();
    	sataFileObserver4=new sdd("sdd");
    	sataFileObserver4.startWatching();
    	ls("sda");
    	ls("sdb");
    	ls("sdc");
    	ls("sdd");
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return true;   
    }    
} 
