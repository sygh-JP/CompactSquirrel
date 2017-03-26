#pragma once

// _CRTDBG_MAP_ALLOC が定義されていると、strdup は _strdup_dbg のエイリアスとしてマクロ定義される。
#if defined(_MSC_VER) && !defined(strdup)
namespace Sqrat
{
	// VC++ 2012 以降で POSIX の strdup() を使おうとすると C4996 のエラーになる。
	// 名前空間に注意して、ISO C++ の _strdup() で置き換える。
	inline char* strdup(const char* src)
	{
		return _strdup(src);
	}
}
#endif

#pragma warning(push)
// 64bit ビルドでは SQInteger は 64bit 整数となる。
// Sqrat には SQInteger を int で受けている箇所がいくつかあるので、そのせいで C4244 の警告が出る。
// 本当は Sqrat 側を直すべきだが、暫定対処で警告を一時的に無効にする。
#pragma warning(disable : 4244)
#include <sqrat.h> // squirrel.h もインクルードされる。
#include <sqstdaux.h> // sqstd_seterrorhandlers()
#include <sqstdstring.h> // sqstd_register_stringlib()
#include <sqstdmath.h> // sqstd_register_mathlib()
//#include <sqstdsystem.h> // sqstd_register_systemlib()
#pragma warning(pop)

#pragma comment(lib, "squirrel.lib")
#pragma comment(lib, "sqstdlib.lib")
