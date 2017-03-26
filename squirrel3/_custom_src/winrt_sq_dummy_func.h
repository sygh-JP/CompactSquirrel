// Dummy declaration of legacy CRT functions / Win32 APIs for Windows Store App.
// We can use these dummy symbols as ad hoc implementation to compile Squirrel with some limitations.
// Specify this header as /FI option argument to "sqstdsystem.cpp".

#pragma once


#ifdef _WIN32
#include <windows.h> // To include <winapifamily.h> indirectly.
#ifdef WINAPI_FAMILY_PARTITION
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define DEFINES_MY_SQ_DUMMY_FUNCS_FOR_WINRT
#endif
#endif
#endif


#ifdef DEFINES_MY_SQ_DUMMY_FUNCS_FOR_WINRT


#include <stdio.h>

// Non-static C function has external linkage.
// To implement following dummy functions internally for static library available in both Desktop App and Store App,
// We have to use other symbols different from those in Desktop libraries.

#ifdef __cplusplus
extern "C"
{
#endif

	int MyWinRTCrtDummy_system(const char* command);
	int MyWinRTCrtDummy_wsystem(const wchar_t* command);
	//FILE* MyWinRTCrtDummy_popen(const char* command, const char* mode);
	//int MyWinRTCrtDummy_pclose(FILE* stream);
	char* MyWinRTCrtDummy_getenv(const char* varname);
	wchar_t* MyWinRTCrtDummy_wgetenv(const wchar_t* varname);

#ifdef __cplusplus
}
#endif

#define system MyWinRTCrtDummy_system
#define _wsystem MyWinRTCrtDummy_wsystem
//#define _popen MyWinRTCrtDummy_popen
//#define _pclose MyWinRTCrtDummy_pclose
#define getenv MyWinRTCrtDummy_getenv
#define _wgetenv MyWinRTCrtDummy_wgetenv

#endif
