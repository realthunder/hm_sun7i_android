package com.cb_test2;

import java.io.IOException;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.util.ArrayList;
import android.util.Log;

public abstract class RootAccess {
    private static final String TAG = "RootAccess";
    protected abstract ArrayList<String> runCommandsWithRootAccess(String p1);

    //Check for Root Access
    public static boolean hasRootAccess() {
        boolean rootBoolean = false;
        Process suProcess;
        
        try {
            suProcess = Runtime.getRuntime().exec("su");

            DataOutputStream os = new DataOutputStream(suProcess.getOutputStream());
            DataInputStream is = new DataInputStream(suProcess.getInputStream());

            if (os != null && is != null) {
                // Getting current user's UID to check for Root Access
                os.writeBytes("id\n");
                os.flush();

                String outputSTR = is.readLine();
                boolean exitSu = false;
                if (outputSTR == null) {
                    rootBoolean = false;
                    exitSu = false;
                    Log.d(TAG, "Can't get Root Access or Root Access deneid by user");
                } else if (outputSTR.contains("uid=0")) {
                    //If is contains uid=0, It means Root Access is granted
                    rootBoolean = true;
                    exitSu = true;
                    Log.d(TAG, "Root Access Granted");
                } else {
                    rootBoolean = false;
                    exitSu = true;
                    Log.d(TAG, "Root Access Rejected: " + is.readLine());
                }

                if (exitSu) {
                    os.writeBytes("exit\n");
                    os.flush();
                }
            }
        } catch (Exception e) {
            rootBoolean = false;
            Log.d(TAG, "Root access rejected [" + e.getClass().getName() + "] : " + e.getMessage());
        }

        return rootBoolean;
    }

    //Execute commands with ROOT Permission
    public final boolean execute(String p1) {
        boolean rootBoolean = false;

	Log.w(TAG, "p1");

        try {
            ArrayList<String> commands = runCommandsWithRootAccess(p1);
            if ( commands != null && commands.size() > 0) {
		Log.w(TAG, "p2");
                Process suProcess = Runtime.getRuntime().exec("su");

                DataOutputStream os = new DataOutputStream(suProcess.getOutputStream());
                
                // Execute commands with ROOT Permission
                for (String currentCommand : commands) {

		    Log.w(TAG, "currentstring=" + currentCommand);
		    
                    os.writeBytes(currentCommand + "\n");
                    os.flush();
		    Log.w(TAG, "p3");
                }

                os.writeBytes("exit\n");
                os.flush();
		Log.w(TAG, "p4");

                try {
                    int suProcessRetval = suProcess.waitFor();
                    if ( suProcessRetval != 255) {
                        // Root Access granted
			Log.w(TAG, "p5");
                        rootBoolean = true;
                    } else {
                        // Root Access denied
			Log.w(TAG, "p6");
                        rootBoolean = false;
                    }
                } catch (Exception ex) {
		    Log.w(TAG, "p7");
                    Log.e(TAG, "Error executing Root Action", ex);

                }
            }
        } catch (IOException ex) {
            Log.w(TAG, "Can't get Root Access", ex);
        } catch (SecurityException ex) {
            Log.w(TAG, "Can't get Root Access", ex);
        } catch (Exception ex) {
            Log.w(TAG, "Error executing operation", ex);
        }

	Log.w(TAG, "p8");
        return rootBoolean;
    }


}