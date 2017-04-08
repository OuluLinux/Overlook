#ifndef _CommandBridge_CommandBridge_h
#define _CommandBridge_CommandBridge_h

#include <CtrlLib/CtrlLib.h>
#include <TabBar/TabBar.h>
using namespace Upp;

#define KEYGROUPNAME "AK"
#define KEYNAMESPACE AKKeys
#define KEYFILE      <CommandBridge/CommandBridge.key>
#include             <CtrlLib/key_header.h>

using namespace AKKeys;

#include "Images.h"
#include "Console.h"

class TrayApp : public TrayIcon {
	
	bool is_exit;
	
public:
	typedef TrayApp CLASSNAME;

	TrayApp();
	
	bool IsExit() const {return is_exit;}
	void Exit() {is_exit = true; Break();}
	void Close() {Break();}
	
	virtual void LeftDouble();
	virtual void LeftDown();
	virtual void Menu(Bar& bar);
};


class CommandBridge : public TopWindow {
	
	MenuBar menu;
	
protected:
	ArrayMap<int, Console> cons;
	TabBar tabs;
	int id_counter;
	
	void ShowConsole(int i);
	
public:
	typedef CommandBridge CLASSNAME;
	CommandBridge();
	
	Console& NewConsole();
	Console* GetActiveConsole();
	void AddConsole();
	void CloseTab();
	void PreviousTab();
	void NextTab();
	void TabClosed(Value tab);
	void MainMenu(Bar& menu);
	void AppMenu(Bar& menu);
	void ViewMenu(Bar& menu);
	void SetupMenu(Bar& menu);
	void RefreshMenu();
	void SetLang(int lang);
	void RefreshTitle(int id);
	void ViewChange(int id);
	
	virtual bool Key(dword key, int count);

	void PostClose() {PostCallback(THISBACK(Close0));}
	void Close0() {Close();}
	void PostTopMost() {PostCallback(THISBACK(TopMost0));}
	void TopMost0() {TopMost();}
	
};

#endif
