#include "stdafx.h"
#include "WeaselIPC.h"
#include "WeaselTSF.h"
#include "KeyEvent.h"

void WeaselTSF::_ProcessKeyEvent(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if (!_IsKeyboardOpen())
		return;

	_EnsureServerConnected();
	weasel::KeyEvent ke;
	GetKeyboardState(_lpbKeyState);
	if (!ConvertKeyEvent(wParam, lParam, _lpbKeyState, ke))
	{
		/* Unknown key event */
		*pfEaten = FALSE;
	}
	else
    	{
		*pfEaten = (BOOL) m_client.ProcessKeyEvent(ke);
    	}
}

STDAPI WeaselTSF::OnSetFocus(BOOL fForeground)
{
	if (fForeground)
		m_client.FocusIn();
	else {
		m_client.FocusOut();
		_AbortComposition();
	}

	return S_OK;
}

/* Some apps sends strange OnTestKeyDown/OnKeyDown combinations:
 *  Some sends OnKeyDown() only. (QQ2012)
 *  Some sends multiple OnTestKeyDown() for a single key event. (MS WORD 2010 x64)
 *
 * We assume every key event will eventually cause a OnKeyDown() call.
 * We use _fTestKeyDownPending to omit multiple OnTestKeyDown() calls,
 *  and for OnKeyDown() to check if the key has already been sent to the server.
 */

STDAPI WeaselTSF::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	ReSetBit(1);					// _bitset[1]: _fTestKeyUpPending		
	if (GetBit(0))					// _bitset[0]: _fTestKeyDownPending
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	if (lParam == 0x4000'0001)
	{
		SetBit(12);				// _bitset[12]: _AutoCADTest
		if (_pComposition)
		{
			SetBit(0);			// _bitset[0]:  _fTestKeyDownPending
			*pfEaten = TRUE;
			return S_OK;
		}
	}
	_ProcessKeyEvent(wParam, lParam, pfEaten);
	if (!_pComposition && wParam <= 'Z' && 'A' <= wParam)
	{
		SetBit(10);				// _bitset[10]: _FirstComposition
		_UpdateComposition(pContext);		
	}
	if (*pfEaten)
	{
		SetBit(0);				// _bitset[0]:  _fTestKeyDownPending
	}
	return S_OK;
}

STDAPI WeaselTSF::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	ReSetBit(1);					// _bitset[1]: _fTestKeyUpPending
	ReSetBit(11);					// _bitset[11]: _SkipFirstDownEventInPreedit
	if (GetBit(0))					// _bitset[0]: _fTestKeyDownPending
	{
		ReSetBit(0);				// _bitset[0]: _fTestKeyDownPending
		*pfEaten = TRUE;
		if (lParam == 0)
		{
			SetBit(12);			// _bitset[12]: _AutoCADTest
		}
		if (GetBit(10))				// _bitset[10]: _FirstComposition
		{
			ReSetBit(10);			// _bitset[10]: _FirstComposition
			SetBit(11);			// _bitset[11]: _SkipFirstDownEventInPreedit
		}
	}
	else
	{
		if (lParam == 0x4000'0001)
		{
			*pfEaten = TRUE;
			SetBit(12);			// _bitset[12]: _AutoCADTest
			_EndComposition(pContext, true);
		}
		else
		{
			_ProcessKeyEvent(wParam, lParam, pfEaten);
		}		
	}
	_UpdateComposition(pContext);
	return S_OK;
} 

STDAPI WeaselTSF::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	ReSetBit(0);					// _bitset[0]: _fTestKeyDownPending
	if (GetBit(1))					// _bitset[1]: _fTestKeyUpPending
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	_ProcessKeyEvent(wParam, lParam, pfEaten);
	_UpdateComposition(pContext);
	if (*pfEaten)
	{
		SetBit(1);				// _bitset[1]: _fTestKeyUpPending
	}
	return S_OK;
}

STDAPI WeaselTSF::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	ReSetBit(0);					// _bitset[0]: _fTestKeyDownPending
	if (GetBit(1))					// _bitset[1]: _fTestKeyUpPending
	{
		ReSetBit(1);				// _bitset[1]: _fTestKeyUpPending
		*pfEaten = TRUE;
	}
	else
	{
		_ProcessKeyEvent(wParam, lParam, pfEaten);
		_UpdateComposition(pContext);
	}
	return S_OK;
}

STDAPI WeaselTSF::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten)
{
	*pfEaten = FALSE;
	return S_OK;
}

BOOL WeaselTSF::_InitKeyEventSink()
{
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;
	HRESULT hr;

	if (_pThreadMgr->QueryInterface(&pKeystrokeMgr) != S_OK)
		return FALSE;

	hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *) this, TRUE);
	
	return (hr == S_OK);
}

void WeaselTSF::_UninitKeyEventSink()
{
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;
	
	if (_pThreadMgr->QueryInterface(&pKeystrokeMgr) != S_OK)
		return;

	pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);
}

BOOL WeaselTSF::_InitPreservedKey()
{
	return TRUE;
#if 0
	com_ptr<ITfKeystrokeMgr> pKeystrokeMgr;
	if (_pThreadMgr->QueryInterface(pKeystrokeMgr.GetAddressOf()) != S_OK)
	{
		return FALSE;
	}
	TF_PRESERVEDKEY preservedKeyImeMode;

	/* Define SHIFT ONLY for now */
	preservedKeyImeMode.uVKey = VK_SHIFT;
	preservedKeyImeMode.uModifiers = TF_MOD_ON_KEYUP;

	auto hr = pKeystrokeMgr->PreserveKey(
		_tfClientId,
		GUID_IME_MODE_PRESERVED_KEY,
		&preservedKeyImeMode, L"", 0);
	
	return SUCCEEDED(hr);
#endif
}

void WeaselTSF::_UninitPreservedKey()
{
}
