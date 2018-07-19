#ifndef _PushNotification_Common_h_
#define _PushNotification_Common_h_

namespace PushNotification {

enum WinToastDismissalReason {
	UserCanceled = 0, //ToastDismissalReason::ToastDismissalReason_UserCanceled,
	ApplicationHidden = 1, //ToastDismissalReason::ToastDismissalReason_ApplicationHidden,
	TimedOut = 2 //ToastDismissalReason::ToastDismissalReason_TimedOut
};

class NotificationBase {

public:
	virtual void NotificationActivated() const = 0;
	virtual void NotificationActivated(int actionIndex) const = 0;
	virtual void NotificationDismissed(WinToastDismissalReason state) const = 0;
	virtual void NotificationFailed() const = 0;
	
	bool is_fail = false;
	
};

enum Results {
	ToastClicked,					// user clicked on the toast
	ToastDismissed,					// user dismissed the toast
	ToastTimeOut,					// toast timed out
	ToastHided,						// application hid the toast
	ToastNotActivated,				// toast was not activated
	ToastFailed,					// toast failed
	SystemNotSupported,				// system does not support toasts
	UnhandledOption,				// unhandled option
	MultipleTextNotSupported,		// multiple texts were provided
	InitializationFailure,			// toast notification manager initialization failure
	ToastNotLaunched				// toast could not be launched
};

}

#endif
