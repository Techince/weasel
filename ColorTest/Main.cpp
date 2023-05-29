#include <Windows.h>
#include <iostream>
#include <string>
#include <format>

using std::cout;	using std::format;
int main()
{
	DWORD color = ::GetSysColor(COLOR_WINDOWTEXT);

	cout << format("当前颜色：{}\n", color);

	while (true)
	{
		if (color != ::GetSysColor(COLOR_WINDOWTEXT))
		{
			color = ::GetSysColor(COLOR_WINDOWTEXT);
			cout << format("变化了，当前颜色：{}\n", color);
		}
	}

	return 0;
}