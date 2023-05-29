#pragma once
#include <string>
#include <codecvt>

#pragma warning(disable:4996)

// UTF-8 conversion
inline const char* wcstoutf8(const WCHAR* wstr)
{
	int buffer_len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	static char* buffer;
	if(buffer)
		delete []buffer;
	buffer = new char[buffer_len+1];
	memset(buffer, 0, buffer_len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer, buffer_len, NULL, NULL);
	return buffer;
}

inline const WCHAR* utf8towcs(const char* utf8_str)
{
	int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
	static WCHAR* wbuffer;
	if(wbuffer)
		delete []wbuffer;
	wbuffer = new WCHAR[nLen + 1];
	memset(wbuffer, 0, sizeof(WCHAR)*(nLen + 1));
	MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wbuffer, nLen);
	return wbuffer;
}

inline int utf8towcslen(const char* utf8_str, int utf8_len)
{
	return MultiByteToWideChar(CP_UTF8, 0, utf8_str, utf8_len, NULL, 0);
}

inline std::wstring getUsername() {
	DWORD len = 0;
	GetUserName(NULL, &len);

	if (len <= 0) {
		return L"";
	}

	wchar_t *username = new wchar_t[len + 1];

	GetUserName(username, &len);
	if (len <= 0) {
		delete[] username;
		return L"";
	}
	auto res = std::wstring(username);
	delete[] username;
	return res;
}

// data directories
std::wstring WeaselUserDataPath();

const char* weasel_shared_data_dir();
const char* weasel_user_data_dir();

inline std::string to_string(const std::wstring& input, int code_page = CP_ACP)
{
	auto len = WideCharToMultiByte(code_page, 0, input.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string temp;

	if (len > 0)
	{
		temp.resize(len);
		WideCharToMultiByte(code_page, 0, input.c_str(), -1, &temp[0], len, nullptr, nullptr);
		temp = temp.data();
	}

	return temp;
}

inline std::wstring to_wstring(const std::string& input, int code_page = CP_ACP)
{
	auto len = MultiByteToWideChar(code_page, 0, input.c_str(), -1, nullptr, 0);
	std::wstring temp;

	if (len > 0)
	{
		temp.resize(len);
		MultiByteToWideChar(code_page, 0, input.c_str(), -1, &temp[0], len);
		temp = temp.data();
	}

	return temp;
}

// resource
std::string GetCustomResource(const char *name, const char *type);