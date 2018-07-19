#include "PushNotification.h"

using namespace PushNotification;

#ifdef flagMAIN

CONSOLE_APP_MAIN {
	::PushNotification::PushNotification n;
	
	n.WhenActivated << [] {LOG("The user clicked in this notification"); Exit();};
	n.WhenActivatedIdx << [](int i) {LOG("The user clicked on action #" << i); Exit();};
	n.WhenDismissed << [](int i) {
		switch (i) {
        case UserCanceled:
            LOG("The user dismissed this notification");
            Exit(1);
            break;
        case TimedOut:
            LOG("The notification has timed out");
            Exit(2);
            break;
        case ApplicationHidden:
            LOG("The application hid the notification using ToastNotifier.hide()");
            Exit(3);
            break;
        default:
            LOG("Toast not activated");
            Exit(4);
            break;
        }
	};
	n.WhenFailed << [] {LOG("Error showing current notification"); Exit();};
	
	n.SetApp("Notification test");
	n.SetMessage("Hello world!");
	n.SetImage(Images::if_terminal_298878());
	
	n.Push();
}


using namespace PushNotification;

class CustomHandler : public NotificationBase {
public:
    void NotificationActivated() const {
        
        Exit();
    }

    void NotificationActivated(int actionIndex) const {
        LOG("The user clicked on action #" << actionIndex);
        Exit();
    }

    void NotificationDismissed(WinToastDismissalReason state) const {
        
    }

    void NotificationFailed() const {
        LOG("Error showing current notification");
        Exit(5);
    }
};




/*
#define COMMAND_ACTION		L"--action"
#define COMMAND_AUMI		L"--aumi"
#define COMMAND_APPNAME		L"--appname"
#define COMMAND_APPID		L"--appid"
#define COMMAND_EXPIREMS	L"--expirems"
#define COMMAND_TEXT		L"--text"
#define COMMAND_HELP		L"--help"
#define COMMAND_IMAGE		L"--image"
#define COMMAND_SHORTCUT	L"--only-create-shortcut"
#define COMMAND_AUDIOSTATE  L"--audio-state"
#define COMMAND_ATTRIBUTE   L"--attribute"

void print_help() {
	std::wcout << "WinToast Console Example [OPTIONS]" << std::endl;
	std::wcout << "\t" << COMMAND_ACTION << L" : Set the actions in buttons" << std::endl;
	std::wcout << "\t" << COMMAND_AUMI << L" : Set the App User Model Id" << std::endl;
	std::wcout << "\t" << COMMAND_APPNAME << L" : Set the default appname" << std::endl;
	std::wcout << "\t" << COMMAND_APPID << L" : Set the App Id" << std::endl;
	std::wcout << "\t" << COMMAND_EXPIREMS << L" : Set the default expiration time" << std::endl;
	std::wcout << "\t" << COMMAND_TEXT << L" : Set the text for the notifications" << std::endl;
	std::wcout << "\t" << COMMAND_IMAGE << L" : set the image path" << std::endl;
    std::wcout << "\t" << COMMAND_ATTRIBUTE << L" : set the attribute for the notification" << std::endl;
	std::wcout << "\t" << COMMAND_SHORTCUT << L" : create the shortcut for the app" << std::endl;
    std::wcout << "\t" << COMMAND_AUDIOSTATE << L" : set the audio state: Default = 0, Silent = 1, Loop = 2" << std::endl;
    std::wcout << "\t" << COMMAND_HELP << L" : Print the help description" << std::endl;
}


int wmain(int argc, LPWSTR *argv)
{
	
}*/


#endif

