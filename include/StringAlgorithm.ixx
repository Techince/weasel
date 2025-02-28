export module StringAlgorithm;
import <algorithm>;
import <clocale>;
import <cwctype>;
import <numeric>;
import <set>;
import <string>;
import <vector>;

export
{
	inline bool ends_with(const std::wstring& wstr, const std::wstring& wsub)
	{
		if (wstr.size() < wsub.size())
			return false;
		else
			return std::equal(wsub.rbegin(), wsub.rend(), wstr.rbegin());
	}

	inline bool iequals(const std::wstring& str1, const std::wstring& str2)
	{
		return std::equal(str1.begin(), str1.end(), str2.begin(), [](const wchar_t& wc1, const wchar_t& wc2)
			{
				return std::towlower(wc1) == std::towlower(wc2);
			});
	}

	inline void ireplace_last(std::wstring& input, const std::wstring& search, const std::wstring& sub)
	{
		std::size_t pos = input.rfind(search);
		if (pos != std::wstring::npos)
			input.replace(pos, search.length(), sub);
	}

	inline std::string join(const std::set<std::string>& list, const std::string& delim)
	{
		return std::accumulate(list.begin(), list.end(), std::string(), [&delim](std::string_view str1, const std::string_view str2)
			{
				return str1.empty() ? str2.data() : str1.data() + delim + str2.data();
			});
	}

	inline std::vector<std::wstring>& split(std::vector<std::wstring>& result, const std::wstring& input, const wchar_t* delim)
	{
		result.clear();
		size_t current = 0;
		size_t next = std::wstring::npos;
		do
		{
			current = next + 1;
			next = input.find_first_of(delim, next + 1);
			result.push_back(input.substr(current, next - current));
		} while (next != std::wstring::npos);
		return result;
	}

	inline bool starts_with(const std::wstring& wstr, const std::wstring& wsub)
	{
		if (wstr.size() < wsub.size())
			return false;
		else
			return std::equal(wsub.begin(), wsub.end(), wstr.begin());
	}

	inline void to_lower(std::wstring& wstr)
	{
		std::setlocale(LC_ALL, "");
		std::transform(wstr.cbegin(), wstr.cend(), wstr.begin(), std::towlower);
	}

	inline std::string to_lower(std::string_view str)
	{
		std::string temp;
		std::setlocale(LC_ALL, "");
		std::transform(str.begin(), str.end(), temp.begin(), std::tolower);
		return temp;
	}
}