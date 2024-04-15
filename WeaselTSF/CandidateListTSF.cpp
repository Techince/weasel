module;
#include "stdafx.h"
module WeaselTSF;
import CandidateList;
import ResponseParser;
import KeyEvent;

using namespace weasel;

void WeaselTSF::_UpdateUI(const Context& ctx, const Status& status)
{
	_cand->UpdateUI(ctx, status);
}

void WeaselTSF::_StartUI()
{
	_cand->StartUI();
}

void WeaselTSF::_EndUI()
{
	_cand->EndUI();
}

void WeaselTSF::_ShowUI()
{
	_cand->Show(TRUE);
}

void WeaselTSF::_HideUI()
{
	_cand->Show(FALSE);
}

com_ptr<ITfContext> WeaselTSF::_GetUIContextDocument()
{
	return _cand->GetContextDocument();
}

void WeaselTSF::_DeleteCandidateList()
{
	_cand->Destroy();
}

void WeaselTSF::_SelectCandidateOnCurrentPage(const int index)
{	
	if (m_hasPreedit)
	{
		m_client.SelectCandidateOnCurrentPage(index);
		// fake a empty presskey to get data back and DoEditSession	
		m_client.ProcessKeyEvent(0);
		_UpdateComposition(_pEditSessionContext);
	}
	else
	{
		SimulatingKeyboardEvents(0x31 + index);
	}
}

void WeaselTSF::_HandleMouseHoverEvent(const int index)
{
	// ToDo: if feature new api comes, replae the processes bellow
	int current_select{};
	_cand->GetSelection(reinterpret_cast<UINT*>(&current_select));

	if (index != current_select)
	{
		m_client.HighlightCandidateOnCurrentPage(index);
		_UpdateComposition(_pEditSessionContext);
	}
}

void WeaselTSF::_HandleMousePageEvent(bool* const nextPage, bool* const scrollNextPage)
{
	// from scrolling event
	if (scrollNextPage)
	{
		if (_cand->style().paging_on_scroll)
		{
			m_client.ChangePage(!(*scrollNextPage));
		}
		else
		{
			UINT current_select = 0, cand_count = 0;
			_cand->GetSelection(&current_select);
			_cand->GetCount(&cand_count);
			bool is_reposition = _cand->GetIsReposition();
			int offset = *scrollNextPage ? 1 : -1;
			offset *= is_reposition ? -1 : 1;
			int index = (int)current_select + offset;
			if (index >= 0 && index < cand_count)
				m_client.HighlightCandidateOnCurrentPage((size_t)index);
			else
			{
				weasel::KeyEvent ke{ 0, 0 };
				ke.keycode = (index < 0) ? ibus::Up : ibus::Down;
				m_client.ProcessKeyEvent(ke);
			}
		}
	}
	else // from click event
	{
		m_client.ChangePage(!(*nextPage));
	}
	_UpdateComposition(_pEditSessionContext);
}

void WeaselTSF::HandleUICallback(int* const sel, int* const hov, bool* const next, bool* const scroll_next)
{
	if (sel)
	{
		_SelectCandidateOnCurrentPage(*sel);
	}
	else if (hov)
	{
		_HandleMouseHoverEvent(*hov);
	}
	else if (next || scroll_next)
	{
		_HandleMousePageEvent(next, scroll_next);
	}
}