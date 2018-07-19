#ifndef _PushNotification_PushNotification_h
#define _PushNotification_PushNotification_h

#include <Core/Core.h>
#include <Draw/Draw.h>
using namespace Upp;

#define IMAGECLASS Images
#define IMAGEFILE <PushNotification/Images.iml>
#include <Draw/iml_header.h>

#include "Common.h"

namespace PushNotification {
	


class PushNotification : public NotificationBase {
	Image img;
	WString msg;
	WString appname = "UppApp";
	Vector<WString> actions;
	int expiration = 15000;
	bool is_silent = false;
	
protected:
    virtual void NotificationActivated() const {WhenActivated();}
    virtual void NotificationActivated(int i) const {WhenActivatedIdx(i);}
	virtual void NotificationDismissed(WinToastDismissalReason state) const {WhenDismissed(state);}
	virtual void NotificationFailed() const {WhenFailed();}

public:
	typedef PushNotification CLASSNAME;
	PushNotification();
	
	PushNotification& SetApp(String s) {appname = WString(s); return *this;}
	PushNotification& SetMessage(String s) {msg = WString(s); return *this;}
	PushNotification& SetImage(Image i) {img = i; return *this;}
	PushNotification& AddAction(String s) {actions.Add(WString(s)); return *this;}
	PushNotification& SetSilent(bool b=true) {is_silent = b; return *this;}
	PushNotification& SetExpiration(int seconds) {expiration = seconds * 1000; return *this;}
	
	void Push();
	void StartPush() {Thread::Start(THISBACK(Push));}
	
	Event<> WhenActivated, WhenFailed;
	Event<int> WhenActivatedIdx, WhenDismissed;
	
};

}

#endif
