#include "stdafx.h"
#include "WeaselTSF.h"
#include "CandidateList.h"
#include "ResponseParser.h"

STDAPI WeaselTSF::DoEditSession(TfEditCookie ec)
{
	// get commit string from server
	std::wstring commit;
	weasel::Config config;
	auto context = std::make_shared<weasel::Context>();
	weasel::ResponseParser parser(&commit, context.get(), &_status, &config, &_cand->style());

	bool ok = m_client.GetResponseData(std::ref(parser));

	_UpdateLanguageBar(_status);

	if (ok)
	{
		if (!commit.empty())
		{
			// For auto-selecting, commit and preedit can both exist.
			// Commit and close the original composition first.
			if (!_IsComposing()) {
				_StartComposition(_pEditSessionContext, GetBit(3) && !config.inline_preedit);	// _bitset[3]: _fCUASWorkaroundEnabled
			}
			_InsertText(_pEditSessionContext, commit);
			_EndComposition(_pEditSessionContext, false);
		}
		if (_status.composing && !_IsComposing())
		{
			_StartComposition(_pEditSessionContext, GetBit(3) && !config.inline_preedit);		// _bitset[3]: _fCUASWorkaroundEnabled
		}
		else if (!_status.composing && _IsComposing())
		{
			_EndComposition(_pEditSessionContext, true);
		}
		if (_IsComposing() && config.inline_preedit && !GetBit(11))	// _bitset[11]: _SkipFirstDownEventInPreedit
		{
			_ShowInlinePreedit(_pEditSessionContext, context);
			SetBit(13);						// _bitset[13]: _InlinePreedit
		}
		if (!GetBit(10))						// _bitset[10]: _FirstComposition
		{
			_UpdateCompositionWindow(_pEditSessionContext);
		}
	}
	_UpdateUI(*context, _status);

	return TRUE;
}

