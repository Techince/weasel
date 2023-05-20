#pragma once
#include <WeaselIPC.h>
#include <WeaselUI.h>
#include <map>
#include <string>

#include <rime_api.h>

typedef std::map<std::string, bool> AppOptions;
typedef std::map<std::string, AppOptions> AppOptionsByAppName;

class RimeWithWeaselHandler :
	public weasel::RequestHandler
{
public:
	RimeWithWeaselHandler(weasel::UI *ui);
	virtual ~RimeWithWeaselHandler();
	virtual void Initialize();
	virtual void Finalize();
	virtual RimeSessionId FindSession(RimeSessionId session_id);
	virtual RimeSessionId AddSession(LPWSTR buffer, EatLine eat = 0);
	virtual RimeSessionId RemoveSession(RimeSessionId session_id);
	virtual BOOL ProcessKeyEvent(weasel::KeyEvent keyEvent, RimeSessionId session_id, EatLine eat);
	virtual void CommitComposition(RimeSessionId session_id);
	virtual void ClearComposition(RimeSessionId session_id);
	virtual void FocusIn(DWORD param, RimeSessionId session_id);
	virtual void FocusOut(DWORD param, RimeSessionId session_id);
	virtual void UpdateInputPosition(RECT const& rc, RimeSessionId session_id);
	virtual void StartMaintenance();
	virtual void EndMaintenance();
	virtual void SetOption(RimeSessionId session_id, const std::string &opt, bool val);

	void OnUpdateUI(std::function<void()> const &cb);

private:
	void _Setup();
	bool _IsDeployerRunning();
	void _UpdateUI(RimeSessionId session_id);
	void _LoadSchemaSpecificSettings(const std::string& schema_id);
	bool _ShowMessage(weasel::Context& ctx, weasel::Status& status);
	bool _Respond(RimeSessionId session_id, EatLine eat);
	void _ReadClientInfo(RimeSessionId session_id, LPWSTR buffer);
	void _GetCandidateInfo(weasel::CandidateInfo &cinfo, RimeContext &ctx);
	void _GetStatus(weasel::Status &stat, RimeSessionId session_id);
	void _GetContext(weasel::Context &ctx, RimeSessionId session_id);

	bool _IsSessionTSF(RimeSessionId session_id);

	AppOptionsByAppName m_app_options;
	weasel::UI* m_ui;  // reference
	RimeSessionId m_active_session;
	bool m_disabled;
	bool m_vista_greater;
	std::string m_last_schema_id;
	weasel::UIStyle m_base_style;
	weasel::UIStyle m_base_style_dark;

	std::function<void()> _UpdateUICallback;

	static void OnNotify(void* context_object,
		                 RimeSessionId session_id,
                         const char* message_type,
                         const char* message_value);
	static std::string m_message_type;
	static std::string m_message_value;
};
