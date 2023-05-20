#pragma once
#include <WeaselIPC.h>
#include <map>
#include <Winnt.h> // for security attributes constants
#include <aclapi.h> // for ACL
#include <boost/thread.hpp>
#include <PipeChannel.h>

#include "SecurityAttribute.h"

namespace weasel
{
	class PipeServer;

	typedef CWinTraits<WS_DISABLED, WS_EX_TRANSPARENT> ServerWinTraits;

	class ServerImpl :
		public CWindowImpl<ServerImpl, CWindow, ServerWinTraits>
	//class ServerImpl
	{
	public:
		DECLARE_WND_CLASS (WEASEL_IPC_WINDOW)

		BEGIN_MSG_MAP(WEASEL_IPC_WINDOW)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_QUERYENDSESSION, OnQueryEndSystemSession)
			MESSAGE_HANDLER(WM_ENDSESSION, OnEndSystemSession)
			MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		END_MSG_MAP()

		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnQueryEndSystemSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEndSystemSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		PARAM OnCommand(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnEcho(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnStartSession(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnEndSession(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnKeyEvent(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnShutdownServer(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnFocusIn(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnFocusOut(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnUpdateInputPosition(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnStartMaintenance(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnEndMaintenance(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnCommitComposition(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);
		PARAM OnClearComposition(WEASEL_IPC_COMMAND uMsg, PARAM wParam, PARAM lParam);

	public:
		ServerImpl();
		~ServerImpl();

		HWND Start();
		int Stop();
		int Run();

		void SetRequestHandler(RequestHandler* pHandler)
		{
			m_pRequestHandler = pHandler;
		}
		void AddMenuHandler(UINT uID, CommandHandler &handler)
		{
			m_MenuHandlers[uID] = handler;
		}

	private:
		void _Finailize();
		template<typename _Resp>
		void HandlePipeMessage(PipeMessage pipe_msg, _Resp resp);

		std::unique_ptr<PipeServer> channel;
		std::unique_ptr<boost::thread> pipeThread;
		RequestHandler *m_pRequestHandler;  // reference
		std::map<UINT, CommandHandler> m_MenuHandlers;
		HMODULE m_hUser32Module;
		SecurityAttribute sa;
	};


}
