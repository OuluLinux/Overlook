#include "PushNotification.h"

#define IMAGECLASS Images
#define IMAGEFILE <PushNotification/Images.iml>
#include <Draw/iml_source.h>

#include <vector>
#include <plugin/png/png.h>

namespace PushNotification {

PushNotification::PushNotification() {
	
}

int PushToast(std::wstring appname, std::wstring msg, std::vector<std::wstring> actions, std::wstring imgpath, bool silent, NotificationBase* base, int expiration_);

void PushNotification::Push() {
	std::vector<std::wstring> actions;
    for(int i = 0; i < this->actions.GetCount(); i++) {
		actions.push_back(actions[i]);
    }
    
    String imgpath;
    if (img.GetSize() != Size(0,0)) {
        PNGEncoder png;
        png.Bpp(24);
        imgpath = GetExeDirFile("push_notification_tmp.png");
		png.SaveFile(imgpath, img);
    }
	
	PushToast(appname, msg, actions, WString(imgpath), is_silent, this, expiration);
	
	if (is_fail) {
		WhenFailed();
	}
	else {
		
	}
}














NotificationQueue::NotificationQueue() {
	
}

NotificationQueue& NotificationQueue::Add(Image img, String msg) {
	for(int i = 0; i < pushed.GetCount(); i++) {
		NotificationQueueItem& it = pushed[i];
		if (it.msg == msg) {
			int age = GetSysTime().Get() - it.pushed.Get();
			if (age < expiration)
				return *this;
		}
	}
	
	NotificationQueueItem& it = queue.Add();
	it.msg = msg;
	it.image = img;
	it.added = GetSysTime();
	if (!running) {
		running = true;
		Thread::Start(THISBACK(Process));
	}
	return *this;
}

void NotificationQueue::Process() {
	
	while (queue.GetCount()) {
		NotificationQueueItem& it = queue[0];
		
		n.SetApp(appname);
		n.SetMessage(it.msg);
		n.SetImage(it.image);
		n.SetExpiration(not_expiration);
		n.SetSilent(is_silent);
		
		it.pushed = GetSysTime();
		
		pushed.Add(queue.Detach(0));
		
		n.Push();
	}
	
	running = false;
}


}
