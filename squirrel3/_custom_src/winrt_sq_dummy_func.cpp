// Dummy implementation of legacy CRT functions / Win32 APIs for Windows Store App.

#include "winrt_sq_dummy_func.h"
#include <errno.h>

#ifdef DEFINES_MY_SQ_DUMMY_FUNCS_FOR_WINRT

// http://msdn.microsoft.com/library/277bwbdz.aspx
int system(const char* command)
{
	errno = ENOENT;
	return 0;
}

int _wsystem(const wchar_t* command)
{
	errno = ENOENT;
	return 0;
}

#if 0
// http://msdn.microsoft.com/library/96ayss4b.aspx
FILE* _popen(const char* command, const char* mode)
{
	return NULL;
}

// http://msdn.microsoft.com/library/25xdhsd2.aspx
int _pclose(FILE* stream)
{
	return -1;
}
#endif

// http://msdn.microsoft.com/library/tehxacec.aspx
char* getenv(const char* varname)
{
	return NULL;
}

wchar_t* _wgetenv(const wchar_t* varname)
{
	return NULL;
}

#endif
