package com.cb_test2;

import java.util.ArrayList;

public class AaRootAccess extends RootAccess {
	public ArrayList<String> runCommandsWithRootAccess(String p1) {
	 ArrayList<String> list = new ArrayList<String>(); 	 
	list.add(p1);
	return list;
    }
}
