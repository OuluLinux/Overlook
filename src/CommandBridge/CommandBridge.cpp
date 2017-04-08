#include "CommandBridge.h"

#define KEYGROUPNAME "AK"
#define KEYNAMESPACE AKKeys
#define KEYFILE      <CommandBridge/CommandBridge.key>
#include             <CtrlLib/key_source.h>

CommandBridge::CommandBridge() {
	Icon(CommandBridgeImg::icon());
	
	id_counter = 0;
	
	Size scr_sz = GetScreenSize();
	SetRect(1, 1, scr_sz.cx - 1, scr_sz.cy / 2);
	
	AddFrame(menu);
	AddFrame(tabs);
	
	RefreshMenu();
	
	tabs.MinTabCount(0);
	tabs.Crosses();
	tabs.WhenClose << THISBACK(TabClosed);
	
	InitWord();
}

Console& CommandBridge::NewConsole() {
	int id = id_counter++;
	Console& c = cons.Add(id);
	c.SetBridge(*this);
	c.WhenTitle << THISBACK1(RefreshTitle, id);
	c.WhenViewChange << THISBACK1(ViewChange, id);
	tabs.AddKey(id, "Console", CommandBridgeImg::icon(), Null, true);
	ShowConsole(id);
	return c;
}

void CommandBridge::AddConsole() {
	NewConsole();
}

void CommandBridge::CloseTab() {
	int active = tabs.GetCursor();
	if (active < 0 || active >= tabs.GetCount()) return;
	// TabBar doesn't let close last tab, so close it manually
	if (tabs.GetCount() == 1) {
		Value key = tabs.GetKey(0);
		tabs.Clear();
		TabClosed(key);
	}
	else
		tabs.Close(active);
}

void CommandBridge::PreviousTab() {
	int active = tabs.GetCursor() - 1;
	if (active < 0) active += tabs.GetCount();
	if (active < 0 || active >= tabs.GetCount()) return;
	tabs.SetCursor(active);
}

void CommandBridge::NextTab() {
	int active = tabs.GetCursor() + 1;
	if (active < 0) active += tabs.GetCount();
	if (active < 0 || active >= tabs.GetCount()) return;
	tabs.SetCursor(active);
}

void CommandBridge::ShowConsole(int id) {
	for(int i = 0; i < cons.GetCount(); i++) {
		RemoveChild(&cons[i]);
	}
	Add(cons.Get(id).SizePos());
}

void CommandBridge::TabClosed(Value tab) {
	int pos, i = tab;
	
	pos = cons.Find(i);
	RemoveChild(&cons[pos]);
	cons.Remove(pos);
}

Console* CommandBridge::GetActiveConsole() {
	int active = tabs.GetCursor();
	if (active < 0 || active >= tabs.GetCount()) return 0;
	int i = tabs.GetKey(active);
	int pos = cons.Find(i);
	return &cons[pos];
}

void CommandBridge::MainMenu(Bar& menu) {
	menu.Add(t_("App"), THISBACK(AppMenu));
	menu.Add(t_("View"), THISBACK(ViewMenu));
	menu.Add(t_("Setup"), THISBACK(SetupMenu));
	
	Console* cons = GetActiveConsole();
	if (cons){
		String cons_menu = cons->GetMenuTitle();
		if (!cons_menu.IsEmpty()) {
			menu.Add(t_(cons_menu), callback(cons, &Console::ConsoleMenu));
		}
	}
}

void CommandBridge::SetLang(int lang)
{
	SetLanguage(lang);
	RefreshMenu();
}

void CommandBridge::RefreshTitle(int id) {
	int active = tabs.GetCursor();
	if (active < 0 || active >= tabs.GetCount()) return;
	int i = tabs.GetKey(active);
	int pos = cons.Find(i);
	Console& c = cons[pos];
	String title = c.GetTitle();
	tabs.SetValue(active, title);
}

void CommandBridge::ViewChange(int id) {
	int active = tabs.GetCursor();
	if (active < 0 || active >= tabs.GetCount()) return;
	active = tabs.GetKey(active);
	if (active != id) return;
	RefreshMenu();
}

void CommandBridge::RefreshMenu() {
	menu.Clear();
	menu.Set(THISBACK(MainMenu));
}

void CommandBridge::AppMenu(Bar& menu) {
	menu.Add(AK_OPENCMD, THISBACK(AddConsole));
}

void CommandBridge::ViewMenu(Bar& menu) {
	menu.Add(AK_CLOSETAB, THISBACK(CloseTab));
	menu.Add(AK_PREVTAB, THISBACK(PreviousTab));
	menu.Add(AK_NEXTTAB, THISBACK(NextTab));
}

void CommandBridge::SetupMenu(Bar& menu)
{
	menu.Add(AK_ENGLISH, THISBACK1(SetLang, LNGC_('E','N','U','S', CHARSET_UTF8)))
	    .Radio(GetCurrentLanguage() == LNGC_('E','N','U','S', CHARSET_UTF8));
	menu.Add(AK_FINNISH, THISBACK1(SetLang, LNGC_('F','I','F','I', CHARSET_UTF8)))
	    .Radio(GetCurrentLanguage() == LNGC_('F','I','F','I', CHARSET_UTF8));
	menu.Separator();
	menu.Add(AK_KEYS, callback(EditKeys));
}

bool CommandBridge::Key(dword key, int count) {
	
	return false;
}


















TrayApp::TrayApp() {
	Icon(CommandBridgeImg::icon());
	Tip("This is U++ TrayIcon");
	
	is_exit = false;
	
}

void TrayApp::LeftDouble() {
	PromptOK("CommandBridge is running!");
}

void TrayApp::LeftDown() {
	Info("CommandBridge", "You have clicked the CommandBridge!");
}

void TrayApp::Menu(Bar& bar) {
	bar.Add("Info..", THISBACK(LeftDouble));
	bar.Separator();
	bar.Add("Exit", THISBACK(Exit));
}
