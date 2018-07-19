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

}
