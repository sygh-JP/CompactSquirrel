// Lua は UTF-8 を使うことで Unicode 対応できるが、ワイド文字・ワイド文字列には対応していない。
// Windows, iOS, Android など、C++ 側の文字列に UTF-16 用の wchar_t を使う場合、変換処理が必要。
// Squirrel はネイティブで wchar_t 対応させることができるため、Win32 API や COM とも相性がよい。

// SQUNICODE は _UNICODE が定義されているときに定義される。
// Visual C++ であれば [Unicode 文字セットを使用する] にしておけば自動的に定義される。
// 必ず Squirrel のライブラリをビルドしたときの条件と同じにすること。

// 2014年5月時点で Squirrel 3.0.6 が最新版だが、API 関数の仕様が昔と比べてかなり変わっているので注意（2.x を想定した Web 上の情報も大半が古くなっている）。
// 古くてメンテナンスのされていないバインダーは、新しい Squirrel 用には使えない可能性がある。
// Squirrel の C++ バインダーは Sqrat の他にも Luabind によく似た Squadd などがあるが、Squadd は2005年の Ver.0.93 を最後に開発が停止している。
// SqPlus も2008年で開発が止まっている模様。
// 今のところ Luabind のように本家 Squirrel の API を併用できる Sqrat が最も良さげ。
// Sqrat の安定版（といっても万年ベータ扱いらしいが）は2013年の Ver.0.8 だが、開発はまだ継続されている模様。
// 2015年9月には 0.9.2 がリリースされているが、その後の音沙汰はなし。 SourceForge でバグ報告は受け付けているらしい。

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <cstdlib>
#include <crtdbg.h>

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <clocale>

#include "MySqratIncludes.hpp"

#include <tchar.h>
#include <conio.h>
#include <Windows.h>

namespace
{
	// テキスト前景色・背景色の設定を行なう。とりあえず Windows 環境前提。
	WORD SetConsoleTextColor(WORD newAttributes)
	{
		auto hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		_ASSERTE(hStdOut);
		CONSOLE_SCREEN_BUFFER_INFO screenBufInfo = {};
		BOOL result = false;
		result = ::GetConsoleScreenBufferInfo(hStdOut, &screenBufInfo);
		_ASSERTE(result);
		const auto oldAttributes = screenBufInfo.wAttributes;
		result = ::SetConsoleTextAttribute(hStdOut, newAttributes);
		_ASSERTE(result);
		return oldAttributes; // 復元用に、前回の設定値を返す。
	}

	// Squirrel に print() 関数の実装として登録する標準文字列出力関数。
	// Squirrel の print() 自体は可変個引数を直接受け取らず、
	// 別途 format() 関数を使う必要があるので、C/C++ 側も引数1個でいいような気がするが、
	// コールバック関数のシグネチャが要求しているので仕方ない。
	void MyPrintFunc(HSQUIRRELVM vm, const SQChar* format, ...)
	{
		va_list args = nullptr;
		va_start(args, format);
#ifdef SQUNICODE
		vwprintf(format, args);
#else
		vprintf(format, args);
#endif
		va_end(args);
	}

	void MyPrintLine(const SQChar* str)
	{
		//_putws(L"C++ " __FUNCTIONW__ L"()\n");

#ifdef SQUNICODE
		wprintf(L"%s\n", str);
#else
		printf("%s\n", str);
#endif
	}

	// スクリプトの実行時エラーが発生したときにコールバックされる。
	void MyErrPrintFunc(HSQUIRRELVM vm, const SQChar* format, ...)
	{
		const auto oldAttributes = SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // 赤前景＋黒背景。

		va_list args = nullptr;
		va_start(args, format);
#ifdef SQUNICODE
		vfwprintf(stderr, format, args);
#else
		vfprintf(stderr, format, args);
#endif
		va_end(args);

		SetConsoleTextColor(oldAttributes);
	}

	// スクリプトのコンパイル エラーが発生したときにコールバックされる。
	void MyCompileErrorFunc(HSQUIRRELVM v, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
	{
		const auto oldAttributes = SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // 黄色前景＋黒背景。

		_ftprintf(stderr, _T("Compile Error!! (%Id, %Id) : %s\n"), line, column, desc);

		SetConsoleTextColor(oldAttributes);
	}
}

#include "MySqHelpers.hpp"
#include "MyVector.hpp"

// C++ ソースにスクリプトやシェーダーのソースコード文字列を埋め込む場合、
// Visual C++ 2013 以降などの新しいコンパイラであれば、C++11 の Raw string literals を使ったほうがよい。
// ややこしいエスケープの記述も不要。
// マクロの # 演算子を使って文字列化する方法もあるが、
// 結局改行のエスケープ シーケンスは入れておかないとコンパイル エラー発生箇所を特定するのが不便になる。

namespace
{
	const SQChar* const MyTestScript1 =
		_SC("print(\"hoge\\n\");\n");

	const SQChar* const MyTestScript2 =
		_SC("print(\"日本語ソース\\n\");\n");

	const SQChar* const MyTestScript3 =
		_SC("function println(str)\n")
		_SC("{ print(str + \"\\n\"); }\n")
		_SC("println(1 + 0.5);\n")
		//_SC("hogehoge = 1;\n") // 未定義変数の使用はコンパイル エラーではなく実行時エラーになる。
		//_SC("local hoge = ;\n") // こちらは実行時エラーではなくコンパイル エラーになる。
		;

	// 単項の + 演算子は Lua 同様に文法エラーらしい。
	// 数値を文字列と結合すると、暗黙的に文字列化されるらしい（弱い型付け）。
	const SQChar* const MyTestScript4 =
		_SC("v1 <- MyTable.MyVector2F();\n")
		_SC("v1.x = cos(0);\n")
		_SC("v1.y = 2;\n")
		_SC("v1.Print();\n")
		_SC("print(format(\"Length = %f, LengthSq = %f\\n\", v1.Length, v1.LengthSq));\n")
		_SC("v2 <- MyTable.MyVector2F().Create(-4, -2);\n")
		_SC("v2.Mul(0.5);\n")
		_SC("v2.Print();\n")
		_SC("print(format(\"Length = %f, LengthSq = %f\\n\", v2.Length, v2.LengthSq));\n")
		_SC("print(\"Dist(v1, v2) = \" + MyTable.GetDistance(v1, v2) + \"\\n\");\n")
		_SC("v3 <- v2.Clone();\n")
		//_SC("v3 <- v2;\n") // 代入による初期化だと、値のコピーではなく参照のコピーになる。Lua と同じ。
		_SC("v3.Add(v1);\n")
		_SC("v3.Print();\n")
		//_SC("v2.Print();\n")
		;
}

// Sqrat の詳しい日本語解説が下記にある。
// http://www.ruche-home.net/program/embed/sqrat/about
// Sqrat 公式サイトの情報も英語だが充実していて分かりやすい。
// http://scrat.sourceforge.net/binding.html
// API マニュアルや標準ライブラリのドキュメントもきっちり PDF 化されている。
// http://www.squirrel-lang.org/doc/sqstdlib3.pdf

int _tmain(int argc, TCHAR* argv[])
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	setlocale(LC_ALL, "");

	//DoMyTest(MyVector2F(1.0f, 2.0f));

	using namespace Sqrat;

	// 試しに Sqrat::Script を使ってみるテスト関数。
	// HACK: 簡単で便利だがコンパイル エラー発生行／桁などを特定する機能がない？
	auto tryExecScript = [](HSQUIRRELVM hvm, const SQChar* scriptSrc)
	{
		try
		{
			Script myScript(hvm);
			myScript.CompileString(scriptSrc);
			myScript.Run();
			return true;
		}
		catch (const Exception& ex)
		{
			_tprintf(_T("Script Error: %s\n"), ex.Message().c_str());
			return false;
		}
	};

	// 初期スタックサイズを指定して、Squirrel の VM を作成。
	// HACK: 簡単な RAII クラスを作るとよい。
	HSQUIRRELVM vm = sq_open(1024);
	_ASSERTE(vm != nullptr);
	{
		// 標準のエラーハンドラーを設定。
		sqstd_seterrorhandlers(vm);

		sq_setprintfunc(vm, MyPrintFunc, MyErrPrintFunc);
		sq_setcompilererrorhandler(vm, MyCompileErrorFunc);

		// Squirrel スクリプトで定義している関数と同名の関数を C++ 側でバインドしておくとどうなる？　どちらが優先される？
		// → Squirrel スクリプトのほうらしい。
		RootTable(vm).Func(_SC("println"), &MyPrintLine);

		MySqHelpers::DoScriptString(vm, MyTestScript1);
		MySqHelpers::DoScriptString(vm, MyTestScript2);
#if 1
		const bool retVal = MySqHelpers::DoScriptString(vm, MyTestScript3);
#else
		const bool retVal = tryExecScript(vm, MyTestScript3);
#endif
		if (retVal)
		{
			// VM の解放後に Sqrat ラッパーオブジェクトを解放しようとするとアクセス違反になるので注意。
			Function funcPrintLine = RootTable(vm).GetFunction(_SC("println"));
			funcPrintLine.Execute(_SC("C++ 側から Squirrel 関数を実行しています。"));
		}

		Table myTable(vm);

		BindMyVector2F(myTable);

		// ルートテーブルに myTable を "MyTable" としてバインド。ファクトリや名前空間のような役目を果たすことになる。
		RootTable(vm).Bind(_SC("MyTable"), myTable);

#if 1
		MySqHelpers::DoScriptString(vm, MyTestScript4);
#else
		tryExecScript(vm, MyTestScript4);
#endif
	}
	// VM を解放。
	sq_close(vm);
	vm = nullptr;

	puts("Press any...");
	_getch();

	return 0;
}
