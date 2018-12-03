#ifndef WEBDRIVERXX_SESSION_H
#define WEBDRIVERXX_SESSION_H

#include "element.h"
#include "window.h"
#include "by.h"
#include "capabilities.h"
#include "keys.h"
#include "js_args.h"
#include "resource.h"
#include "keyboard.h"
#include "shared.h"
#include "factories_impl.h"
#include "picojson.h"
#include <string>

namespace webdriverxx {

class Client;

class Session { // copyable
public:	
	Capabilities GetCapabilities() const;
	std::string GetSource() const;
	std::string GetTitle() const;
	std::string GetUrl() const;
	std::string GetScreenshot() const; // Base64 PNG

	const Session& Navigate(const std::string& url) const;
	const Session& Get(const std::string& url) const; // Same as Navigate
	const Session& Forward() const;
	const Session& Back() const;
	const Session& Refresh() const;

	const Session& Execute(const std::string& script, const JsArgs& args = JsArgs()) const;
	template<typename T>
	T Eval(const std::string& script, const JsArgs& args = JsArgs()) const;
	const Session& ExecuteAsync(const std::string& script, const JsArgs& args = JsArgs()) const;
	template<typename T>
	T EvalAsync(const std::string& script, const JsArgs& args = JsArgs()) const;

	const Session& SetFocusToFrame(const Element& frame) const;
	const Session& SetFocusToFrame(const std::string& id) const;
	const Session& SetFocusToFrame(int number) const;
	const Session& SetFocusToDefaultFrame() const;
	const Session& SetFocusToParentFrame() const;

	std::vector<Window> GetWindows() const;
	Window GetCurrentWindow() const;
	const Session& CloseCurrentWindow() const;
	const Session& SetFocusToWindow(const std::string& window_name) const;
	const Session& SetFocusToWindow(const Window& window) const;
	
	Element GetActiveElement() const;

	Element FindElement(const By& by) const;
	std::vector<Element> FindElements(const By& by) const;

	std::vector<Cookie> GetCookies() const;
	const Session& SetCookie(const Cookie& cookie) const;
	const Session& DeleteCookies() const;
	const Session& DeleteCookie(const std::string& name) const;

	std::string GetAlertText() const;
	const Session& SendKeysToAlert(const std::string& text) const;
	const Session& AcceptAlert() const;
	const Session& DismissAlert() const;

	const Session& SendKeys(const std::string& keys) const;
	const Session& SendKeys(const Shortcut& shortcut) const;

	const Session& MoveToTopLeftOf(const Element&, const Offset& = Offset()) const;
	const Session& MoveToCenterOf(const Element&) const;
	const Session& MoveTo(const Offset&) const;
	const Session& Click(mouse::Button = mouse::LeftButton) const;
	const Session& DoubleClick() const;
	const Session& ButtonDown(mouse::Button = mouse::LeftButton) const;
	const Session& ButtonUp(mouse::Button = mouse::LeftButton) const;

	const Session& SetTimeoutMs(timeout::Type type, int milliseconds);
	const Session& SetImplicitTimeoutMs(int milliseconds);
	const Session& SetAsyncScriptTimeoutMs(int milliseconds);

	void DeleteSession() const; // No need to delete sessions created by WebDriver or Client
	virtual ~Session() {}

private:
	friend class Client; // Only Client can create Sessions

	explicit Session(const detail::Shared<detail::Resource>& resource);

	Window MakeWindow(const std::string& handle) const;
	detail::Keyboard GetKeyboard() const;
	template<typename T>
	void InternalEval(const std::string& webdriver_command,
		const std::string& script, const JsArgs& args,
		T& result) const;
	void InternalEval(const std::string& webdriver_command,
		const std::string& script, const JsArgs& args,
		Element& result) const;
	picojson::value InternalEvalJsonValue(const std::string& command,
		const std::string& script, const JsArgs& args) const;
	const Session& InternalSetFocusToFrame(const picojson::value& id) const;
	const Session& InternalMoveTo(const Element*, const Offset*) const;
	const Session& InternalMouseButtonCommand(const char* command, mouse::Button button) const;

private:
	detail::Shared<detail::Resource> resource_;
	detail::Shared<detail::SessionFactory> factory_;
};

} // namespace webdriverxx

#include "session.inl"

#endif
