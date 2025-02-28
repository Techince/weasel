#include "pch.h"
#include "VersionHelpers.hpp"
#include "DirectWriteResources.h"

using namespace weasel;

#define STYLEORWEIGHT	(L":[^:]*[^a-f0-9:]+[^:]*")	

std::vector<std::wstring> ws_split(const std::wstring& in, const std::wstring& delim)
{
	std::wregex re{ delim };
	return std::vector<std::wstring> {
		std::wsregex_token_iterator(in.begin(), in.end(), re, -1),
			std::wsregex_token_iterator()
	};
}

std::vector<std::string> c_split(const char* in, const char* delim)
{
	std::regex re{ delim };
	return std::vector<std::string> {
		std::cregex_token_iterator(in, in + strlen(in), re, -1),
			std::cregex_token_iterator()
	};
}

std::vector<std::wstring> wc_split(const wchar_t* in, const wchar_t* delim)
{
	std::wregex re{ delim };
	return std::vector<std::wstring> {
		std::wcregex_token_iterator(in, in + wcslen(in), re, -1),
			std::wcregex_token_iterator()
	};
}

DirectWriteResources::DirectWriteResources(weasel::UIStyle& style, UINT dpi) :	
	dpiScaleX_(0),
	dpiScaleY_(0),
	pD2d1Factory(NULL),
	pDWFactory(NULL),
	pRenderTarget(NULL),
	pPreeditTextFormat(NULL),
	pTextFormat(NULL),
	pLabelTextFormat(NULL),
	pCommentTextFormat(NULL),
	pTextLayout(NULL),
	m_pBrush{ nullptr },
	_style{ style }
{
	D2D1_TEXT_ANTIALIAS_MODE mode = _style.antialias_mode <= 3 ? (D2D1_TEXT_ANTIALIAS_MODE)(_style.antialias_mode) : D2D1_TEXT_ANTIALIAS_MODE_FORCE_DWORD; // prepare d2d1 resources
	HRESULT hResult = S_OK;
	// create factory
	if (pD2d1Factory == NULL)
		hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, pD2d1Factory.GetAddressOf());
	// create IDWriteFactory
	if (pDWFactory == NULL)
		hResult = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(pDWFactory.GetAddressOf()));
	/* ID2D1HwndRenderTarget */
	if (pRenderTarget == NULL)
	{
		const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
		const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, format);
		pD2d1Factory->CreateDCRenderTarget(&properties, &pRenderTarget);
		pRenderTarget->SetTextAntialiasMode(mode);
		pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_pBrush);
	//get the dpi information
	dpiScaleX_ = dpiScaleY_ = dpi;
	dpiScaleX_ /= 72.0f;
	dpiScaleY_ /= 72.0f;

	InitResources(style, dpi);
}

DirectWriteResources::~DirectWriteResources()
{
	pPreeditTextFormat.Reset();
	pTextFormat.Reset();
	pLabelTextFormat.Reset();
	pCommentTextFormat.Reset();
	pTextLayout.Reset();
	m_pBrush.Reset();
	pRenderTarget.Reset();
	pDWFactory.Reset();
	pD2d1Factory.Reset();
}

HRESULT DirectWriteResources::InitResources(std::wstring_view label_font_face, int label_font_point,
	std::wstring_view font_face, int font_point,
	std::wstring_view comment_font_face, int comment_font_point, bool vertical_text)
{
	// prepare d2d1 resources
	pPreeditTextFormat.Reset();
	pTextFormat.Reset();
	pLabelTextFormat.Reset();
	pCommentTextFormat.Reset();
	DWRITE_WORD_WRAPPING wrapping = ((_style.max_width == 0 && _style.layout_type != UIStyle::LAYOUT_VERTICAL_TEXT)
		|| (_style.max_height == 0 && _style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT))
		? DWRITE_WORD_WRAPPING_NO_WRAP : DWRITE_WORD_WRAPPING_CHARACTER;
	DWRITE_FLOW_DIRECTION flow = _style.vertical_text_left_to_right ? DWRITE_FLOW_DIRECTION_LEFT_TO_RIGHT : DWRITE_FLOW_DIRECTION_RIGHT_TO_LEFT;

	HRESULT hResult = S_OK;
	std::vector<std::wstring> fontFaceStrVector;

	// text font text format set up
	fontFaceStrVector = ws_split(font_face.data(), L",");
	// set main font a invalid font name, to make every font range customizable
	const std::wstring _mainFontFace = L"_InvalidFontName_";
	DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL;
	DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL;
	// setup font weight and font style by the first unit of font_face setting string
	_ParseFontFace(font_face.data(), fontWeight, fontStyle);
	fontFaceStrVector[0] = std::regex_replace(fontFaceStrVector[0], std::wregex(STYLEORWEIGHT, std::wregex::icase), L"");
	hResult = pDWFactory->CreateTextFormat(_mainFontFace.data(), NULL,
		fontWeight, fontStyle, DWRITE_FONT_STRETCH_NORMAL,
		font_point * dpiScaleX_, L"", reinterpret_cast<IDWriteTextFormat**>(pTextFormat.GetAddressOf()));
	if (pTextFormat != NULL)
	{
		if (vertical_text)
		{
			pTextFormat->SetFlowDirection(flow);
			pTextFormat->SetReadingDirection(DWRITE_READING_DIRECTION_TOP_TO_BOTTOM);
			pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		}
		else
			pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

		pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		pTextFormat->SetWordWrapping(wrapping);
		_SetFontFallback(pTextFormat, fontFaceStrVector);
	}
	decltype(fontFaceStrVector)().swap(fontFaceStrVector);

	fontFaceStrVector = ws_split(font_face.data(), L",");
	//_ParseFontFace(fontFaceStrVector[0], fontWeight, fontStyle);
	fontFaceStrVector[0] = std::regex_replace(fontFaceStrVector[0], std::wregex(STYLEORWEIGHT, std::wregex::icase), L"");
	hResult = pDWFactory->CreateTextFormat(_mainFontFace.data(), NULL,
		fontWeight, fontStyle, DWRITE_FONT_STRETCH_NORMAL,
		font_point * dpiScaleX_, L"", reinterpret_cast<IDWriteTextFormat**>(pPreeditTextFormat.GetAddressOf()));
	if (pPreeditTextFormat != NULL)
	{
		if (vertical_text)
		{
			pPreeditTextFormat->SetFlowDirection(flow);
			pPreeditTextFormat->SetReadingDirection(DWRITE_READING_DIRECTION_TOP_TO_BOTTOM);
			pPreeditTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		}
		else
			pPreeditTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		pPreeditTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		pPreeditTextFormat->SetWordWrapping(wrapping);
		_SetFontFallback(pPreeditTextFormat, fontFaceStrVector);
	}
	decltype(fontFaceStrVector)().swap(fontFaceStrVector);

	// label font text format set up
	fontFaceStrVector = ws_split(label_font_face.data(), L",");
	// setup weight and style of label_font_face
	_ParseFontFace(fontFaceStrVector[0], fontWeight, fontStyle);
	fontFaceStrVector[0] = std::regex_replace(fontFaceStrVector[0], std::wregex(STYLEORWEIGHT, std::wregex::icase), L"");
	hResult = pDWFactory->CreateTextFormat(_mainFontFace.data(), NULL,
		fontWeight, fontStyle, DWRITE_FONT_STRETCH_NORMAL,
		label_font_point * dpiScaleX_, L"", reinterpret_cast<IDWriteTextFormat**>(pLabelTextFormat.GetAddressOf()));
	if (pLabelTextFormat != NULL)
	{
		if (vertical_text)
		{
			pLabelTextFormat->SetFlowDirection(flow);
			pLabelTextFormat->SetReadingDirection(DWRITE_READING_DIRECTION_TOP_TO_BOTTOM);
			pLabelTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		}
		else
			pLabelTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		pLabelTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		pLabelTextFormat->SetWordWrapping(wrapping);
		_SetFontFallback(pLabelTextFormat, fontFaceStrVector);
	}
	decltype(fontFaceStrVector)().swap(fontFaceStrVector);

	// comment font text format set up
	fontFaceStrVector = ws_split(comment_font_face.data(), L",");
	// setup weight and style of label_font_face
	_ParseFontFace(fontFaceStrVector[0], fontWeight, fontStyle);
	fontFaceStrVector[0] = std::regex_replace(fontFaceStrVector[0], std::wregex(STYLEORWEIGHT, std::wregex::icase), L"");
	hResult = pDWFactory->CreateTextFormat(_mainFontFace.data(), NULL,
		fontWeight, fontStyle, DWRITE_FONT_STRETCH_NORMAL,
		comment_font_point * dpiScaleX_, L"", reinterpret_cast<IDWriteTextFormat**>(pCommentTextFormat.GetAddressOf()));
	if (pCommentTextFormat != NULL)
	{
		if (vertical_text)
		{
			pCommentTextFormat->SetFlowDirection(flow);
			pCommentTextFormat->SetReadingDirection(DWRITE_READING_DIRECTION_TOP_TO_BOTTOM);
			pCommentTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		}
		else
			pCommentTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		pCommentTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		pCommentTextFormat->SetWordWrapping(wrapping);
		_SetFontFallback(pCommentTextFormat, fontFaceStrVector);
	}
	decltype(fontFaceStrVector)().swap(fontFaceStrVector);
	return hResult;
}

HRESULT DirectWriteResources::InitResources(UIStyle& style, UINT dpi = 0)
{
	_style = style;
	if (dpi)
	{
		dpiScaleX_ = dpi / 72.0f;
		dpiScaleY_ = dpi / 72.0f;
	}
	return InitResources(style.label_font_face, style.label_font_point, style.font_face, style.font_point,
		style.comment_font_face, style.comment_font_point, style.layout_type == UIStyle::LAYOUT_VERTICAL_TEXT);
}

HRESULT weasel::DirectWriteResources::CreateTextLayout(std::wstring_view text, const int nCount, IDWriteTextFormat1* const txtFormat, const float width, const float height)
{
	return pDWFactory->CreateTextLayout(text.data(), nCount, txtFormat, width, height, reinterpret_cast<IDWriteTextLayout**>(pTextLayout.GetAddressOf()));
}

HRESULT weasel::DirectWriteResources::CreateBrush(const D2D1_COLOR_F& color)
{
	return pRenderTarget->CreateSolidColorBrush(color, &m_pBrush);
}

HRESULT weasel::DirectWriteResources::GetLayoutOverhangMetrics(DWRITE_OVERHANG_METRICS* overhangMetrics)
{
	return pTextLayout->GetOverhangMetrics(overhangMetrics);
}

HRESULT weasel::DirectWriteResources::GetLayoutMetrics(DWRITE_TEXT_METRICS* metrics)
{
	return pTextLayout->GetMetrics(metrics);
}

HRESULT weasel::DirectWriteResources::SetLayoutReadingDirection(const DWRITE_READING_DIRECTION& direct)
{
	return pTextLayout->SetReadingDirection(direct);
}

HRESULT weasel::DirectWriteResources::SetLayoutFlowDirection(const DWRITE_FLOW_DIRECTION& direct)
{
	return pTextLayout->SetFlowDirection(direct);
}

void weasel::DirectWriteResources::DrawRect(D2D1_RECT_F* const rect, const float strokeWidth, ID2D1StrokeStyle* const sstyle)
{
	pRenderTarget->DrawRectangle(rect, m_pBrush.Get(), strokeWidth, sstyle);
}

void weasel::DirectWriteResources::DrawTextLayoutAt(const D2D1_POINT_2F& point)
{
	pRenderTarget->DrawTextLayout(point, pTextLayout.Get(), m_pBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}

void weasel::DirectWriteResources::SetDpi(UINT dpi)
{
	dpiScaleX_ = dpi / 72.0f;
	dpiScaleY_ = dpi / 72.0f;

	pPreeditTextFormat.Reset();
	pTextFormat.Reset();
	pLabelTextFormat.Reset();
	pCommentTextFormat.Reset();
	InitResources(_style);
}

static std::wstring _MatchWordsOutLowerCaseTrim1st(const std::wstring& str, const std::wstring& pat)
{
	std::wstring mat{};
	std::wsmatch mc;
	std::wregex pattern{ pat, std::wregex::icase};
	auto iter{ str.cbegin() };
	auto end{ str.cend() };
	while (std::regex_search(iter, end, mc, pattern))
	{
		for (const auto& m : mc)
		{
			mat = m;
			mat = mat.substr(1);
			break;
		}
		iter = mc.suffix().first;
	}
	std::wstring res;
	std::transform(mat.begin(), mat.end(), std::back_inserter(res), ::tolower);
	return res;
}

void DirectWriteResources::_ParseFontFace(std::wstring_view fontFaceStr, DWRITE_FONT_WEIGHT& fontWeight, DWRITE_FONT_STYLE& fontStyle)
{
	const std::wstring patWeight(L"(:thin|:extra_light|:ultra_light|:light|:semi_light|:medium|:demi_bold|:semi_bold|:bold|:extra_bold|:ultra_bold|:black|:heavy|:extra_black|:ultra_black)");
	const std::map<std::wstring, DWRITE_FONT_WEIGHT> _mapWeight = {
		{L"thin",			DWRITE_FONT_WEIGHT_THIN},
		{L"extra_light",	DWRITE_FONT_WEIGHT_EXTRA_LIGHT},
		{L"ultra_light",	DWRITE_FONT_WEIGHT_ULTRA_LIGHT},
		{L"light",			DWRITE_FONT_WEIGHT_LIGHT},
		{L"semi_light",		DWRITE_FONT_WEIGHT_SEMI_LIGHT},
		{L"medium",			DWRITE_FONT_WEIGHT_MEDIUM},
		{L"demi_bold",		DWRITE_FONT_WEIGHT_DEMI_BOLD},
		{L"semi_bold",		DWRITE_FONT_WEIGHT_SEMI_BOLD},
		{L"bold",			DWRITE_FONT_WEIGHT_BOLD},
		{L"extra_bold",		DWRITE_FONT_WEIGHT_EXTRA_BOLD},
		{L"ultra_bold",		DWRITE_FONT_WEIGHT_ULTRA_BOLD},
		{L"black",			DWRITE_FONT_WEIGHT_BLACK},
		{L"heavy",			DWRITE_FONT_WEIGHT_HEAVY},
		{L"extra_black",	DWRITE_FONT_WEIGHT_EXTRA_BLACK},
		{L"normal",			DWRITE_FONT_WEIGHT_NORMAL},
		{L"ultra_black",	DWRITE_FONT_WEIGHT_ULTRA_BLACK}
	};
	std::wstring weight = _MatchWordsOutLowerCaseTrim1st(fontFaceStr.data(), patWeight);
	auto it = _mapWeight.find(weight);
	fontWeight = (it != _mapWeight.end()) ? it->second : DWRITE_FONT_WEIGHT_NORMAL;

	const std::wstring patStyle(L"(:italic|:oblique|:normal)");
	const std::map<std::wstring, DWRITE_FONT_STYLE> _mapStyle = {
		{L"italic",		DWRITE_FONT_STYLE_ITALIC},
		{L"oblique",	DWRITE_FONT_STYLE_OBLIQUE},
		{L"normal",		DWRITE_FONT_STYLE_NORMAL},
	};
	std::wstring style = _MatchWordsOutLowerCaseTrim1st(fontFaceStr.data(), patStyle);
	auto it2 = _mapStyle.find(style);
	fontStyle = (it2 != _mapStyle.end()) ? it2->second : DWRITE_FONT_STYLE_NORMAL;
}

void DirectWriteResources::_SetFontFallback(ComPtr<IDWriteTextFormat1> textFormat, std::span<std::wstring> fontVector)
{
	ComPtr<IDWriteFontFallback> pSysFallback;
	pDWFactory->GetSystemFontFallback(pSysFallback.GetAddressOf());
	ComPtr<IDWriteFontFallback> pFontFallback;
	ComPtr<IDWriteFontFallbackBuilder> pFontFallbackBuilder;
	pDWFactory->CreateFontFallbackBuilder(pFontFallbackBuilder.GetAddressOf());
	std::vector<std::wstring> fallbackFontsVector;
	for (UINT32 i = 0; i < fontVector.size(); i++)
	{
		fallbackFontsVector = ws_split(fontVector[i], L":");
		std::wstring _fontFaceWstr, firstWstr, lastWstr;
		if (fallbackFontsVector.size() == 3)
		{
			_fontFaceWstr = fallbackFontsVector[0];
			firstWstr = fallbackFontsVector[1];
			lastWstr = fallbackFontsVector[2];
			if (lastWstr.empty())
				lastWstr = L"10ffff";
			if (firstWstr.empty())
				firstWstr = L"0";
		}
		else if (fallbackFontsVector.size() == 2)	// fontName : codepoint
		{
			_fontFaceWstr = fallbackFontsVector[0];
			firstWstr = fallbackFontsVector[1];
			if (firstWstr.empty())
				firstWstr = L"0";
			lastWstr = L"10ffff";
		}
		else if (fallbackFontsVector.size() == 1)	// if only font defined, use all range
		{
			_fontFaceWstr = fallbackFontsVector[0];
			firstWstr = L"0";
			lastWstr = L"10ffff";
		}
		UINT first = 0, last = 0x10ffff;
		try {
			first = std::stoi(firstWstr.c_str(), 0, 16);
		}
		catch (...) {
			first = 0;
		}
		try {
			last = std::stoi(lastWstr.c_str(), 0, 16);
		}
		catch (...) {
			last = 0x10ffff;
		}
		DWRITE_UNICODE_RANGE range = { first, last };
		const  WCHAR* familys = { _fontFaceWstr.c_str() };
		pFontFallbackBuilder->AddMapping(&range, 1, &familys, 1);
		decltype(fallbackFontsVector)().swap(fallbackFontsVector);
	}
	// add system defalt font fallback
	pFontFallbackBuilder->AddMappings(pSysFallback.Get());
	pFontFallbackBuilder->CreateFontFallback(pFontFallback.GetAddressOf());
	textFormat->SetFontFallback(pFontFallback.Get());
	decltype(fallbackFontsVector)().swap(fallbackFontsVector);
	pFontFallback.Reset();
	pSysFallback.Reset();
	pFontFallbackBuilder.Reset();
}