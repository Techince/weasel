#include <Windows.h>
#include <iostream>
#include <string>
#include <format>

using std::cout;	using std::format;
int main()
{
	DWORD color = ::GetSysColor(COLOR_WINDOWTEXT);

	cout << format("��ǰ��ɫ��{}\n", color);

	while (true)
	{
		if (color != ::GetSysColor(COLOR_WINDOWTEXT))
		{
			color = ::GetSysColor(COLOR_WINDOWTEXT);
			cout << format("�仯�ˣ���ǰ��ɫ��{}\n", color);
		}
	}

	return 0;
}