
#include <stdio.h>
#include <stdlib.h>

main(int argc, char *argv[])
{
	wchar_t foo[32];
	mbstowcs(foo, argv[1], sizeof foo);
	wprintf(L"<%ls>", foo);
	return 0;
}

