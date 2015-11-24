package com.ct_dd;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class Select extends Main{
	private Button ok,back;
	public static ProgressDialog d;
	private EditText et1,et2;
	private static Builder s1,s2,s3;
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.select);
        et1=(EditText)findViewById(R.id.et1);
        et2=(EditText)findViewById(R.id.et2);
        ok=(Button)findViewById(R.id.ok);
        back=(Button)findViewById(R.id.back);
        
        s1 = new AlertDialog.Builder(Select.this)
        .setTitle(R.string.atitle)
        .setMessage(R.string.smsg4)
        .setPositiveButton(R.string.yes,
        		new DialogInterface.OnClickListener() {
        	@Override
			public void onClick(DialogInterface arg0, int arg1) {
			}
		});
        
        s2 = new AlertDialog.Builder(Select.this)
        .setTitle(R.string.atitle)
        .setMessage(R.string.smsg3)
        .setPositiveButton(R.string.yes,
        		new DialogInterface.OnClickListener() {
        	@Override
			public void onClick(DialogInterface arg0, int arg1) {
			}
		});
        
        s3 = new AlertDialog.Builder(Select.this)
        .setTitle(R.string.atitle)
        .setMessage(R.string.smsg5)
        .setPositiveButton(R.string.yes,
        		new DialogInterface.OnClickListener() {
        	@Override
			public void onClick(DialogInterface arg0, int arg1) {
			}
		});
        
        ok.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View arg0) {
				int num=Integer.parseInt(et1.getText().toString());
				String space=et2.getText().toString();
				try {
					start();
					fdisk(p,num,space);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
        });
        
        back.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View v) {
				Intent intent=new Intent();
				intent.setClass(Select.this, Main.class);
				startActivityForResult(intent,0);
				Select.this.finish();
			}
        });
	}
	
	//-----------------------fdisk command--------------------------
    public void fdisk(final String p,final int n,final String s)throws IOException{
    	d=ProgressDialog.show(Select.this, getString(R.string.dtitle),
                getString(R.string.d3body),true);
    	new Thread(){
    		public void run(){		
    			try{
    				ArrayList<String> list=new ArrayList<String>();
    				list.add("sh /system/bin/shfdisk.sh "+p+" "+n+" "+s);
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
    				if(sb.toString().contains("fdisk=0"))
    				{
    					d.dismiss();
    				Runnable runnable=new Runnable(){
						@Override
						public void run() {
							Message msg=Handler2.obtainMessage();
							msg.what=2;
							msg.sendToTarget();
						}
    				};new Thread(runnable).start();
    				}
				if(sb.toString().contains("fdisk=1"))
				{
					d.dismiss();
					Message msg=Handler2.obtainMessage();
					msg.what=1;
					msg.sendToTarget();
				}
    				if(sb.toString().contains("no sata"))
    				{
    					d.dismiss();
    				Runnable runnable=new Runnable(){
						@Override
						public void run() {
							Message msg=Handler2.obtainMessage();
							msg.what=3;
							msg.sendToTarget();
						}
    				};new Thread(runnable).start();
    				}
    			}catch(Exception ex){
    				Runnable runnable=new Runnable(){
						@Override
						public void run() {
							Message msg=Handler2.obtainMessage();
							msg.what=1;
							msg.sendToTarget();
						}
    				};new Thread(runnable).start();
    			}
    		}
    	}.start();
    }
    static Handler Handler2=new Handler(){
    	@Override
    	public void handleMessage(Message msg){
    		if(msg.what==1){
    			d.dismiss();
    			s1.show();
    		}else if(msg.what==3){
    			s3.show();
    		}
    		else{s2.show();};
    		super.handleMessage(msg);
    	}
    };
} 
