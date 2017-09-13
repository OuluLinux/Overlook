#include "CommandBridge.h"

#define IMAGECLASS CommandBridgeImg
#define IMAGEFILE <CommandBridge/CommandBridge.iml>
#include <Draw/iml_source.h>

bool is_tray;
bool is_cons_toggled;
TrayApp* last_tray;

void ToggleWindow(CommandBridge* cons) {
	is_cons_toggled = true;
	if (is_tray)
		last_tray->Close();
	else
		cons->PostClose();
}

GUI_APP_MAIN {
	
	if (!CommandLine().IsEmpty()) {
		if (CommandLine()[0] == "ftpd") {
			FTPServer f;
			f.Start();
			while(!Thread::IsShutdownThreads()) Sleep(1000);
			return;
		}
	}
	
	String keyfile = ConfigFile("keys.key");
	RestoreKeys(LoadFile(keyfile));
	
	CommandBridge cons;
	
	cons.AddConsole();
	
	
	#ifdef flagDEBUG
	Ctrl::RegisterSystemHotKey(K_CTRL|K_SHIFT|K_X, callback1(ToggleWindow, &cons));
	is_tray = false;
	#else
	// TODO: find correct way to get the key under escape, this is that ('§' key) in finnish windows system
	Ctrl::RegisterSystemHotKey(65756, callback1(ToggleWindow, &cons));
	is_tray = true;
	#endif
	
	bool is_exit = false;
	is_cons_toggled = true;
	while (!is_exit && !Thread::IsShutdownThreads()) {
		if (!is_cons_toggled)
			break;
		is_cons_toggled = false;
		
		if (is_tray) {
			TrayApp tray;
			last_tray = &tray;
			tray.Run();
			is_exit = tray.IsExit();
		}
		else {
			cons.PostTopMost();
			cons.Run();
			cons.CloseTopCtrls();
		}
		
		is_tray = !is_tray;
		
		#ifdef flagDEBUG
		is_exit = true;
		#endif
	}
	
	SaveFile(keyfile, StoreKeys());
}
