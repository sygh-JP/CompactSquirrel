#include "pch.h"
#include "MySquirrelTest.h"

// ヘッダーだけでは Unicode ビルドの Squirrel かどうか判断できない（結局 SQUNICODE の定義はホスト側にも依存する）が、
// Windows ストア アプリでは Unicode ビルドしか使えないので、
// そのスタティック ライブラリや DLL のプロジェクトも Unicode ビルドされていることが保証されるはず。
// 要するに下記のチェックはなくても問題ない。
#ifndef SQUNICODE
#error Squirrel is not Unicode version!!
#endif
static_assert(std::is_same<SQChar, wchar_t>::value, "SQChar is not wchar_t!!");


WriteStringDelegate^ MySquirrel::m_outputWriter;
WriteStringDelegate^ MySquirrel::m_errorWriter;

namespace
{
	const size_t PrintTempBufCount = 1024;

	// Squirrel の print() の実装。
	void MyPrintFunc(HSQUIRRELVM vm, const SQChar* format, ...)
	{
		// HACK: ATL が使える Pro 以上のエディションならば、vswprintf_s() よりも CStringW::FormatV() を使うとよい。
		// ちなみに最近の Embarcadero RAD Studio（旧 Borland C++ Builder）では、
		// Visual C++ のセキュア CRT 互換の関数（末尾に _s が付くもの）が実装されているらしい。
		// MS の提唱している仕様は一応正当で、
		// C/C++ のセキュリティ対策としては、今後は当然すべての処理系にてセキュア関数が実装されるべきだが、
		// Clang や gcc ではどうなるのか不明……

		auto writer = MySquirrel::GetOutputWriter();
		if (writer)
		{
			wchar_t buf[PrintTempBufCount] = {};
			va_list args = nullptr;
			va_start(args, format);
			vswprintf_s(buf, format, args);
			va_end(args);
			writer(ref new Platform::String(buf));
		}
	}

	// スクリプトの実行時エラーが発生したときにコールバックされる。
	void MyErrPrintFunc(HSQUIRRELVM vm, const SQChar* format, ...)
	{
		auto writer = MySquirrel::GetErrorWriter();
		if (writer)
		{
			wchar_t buf[PrintTempBufCount] = {};
			va_list args = nullptr;
			va_start(args, format);
			vswprintf_s(buf, format, args);
			va_end(args);
			writer(ref new Platform::String(buf));
		}
	}

	// スクリプトのコンパイル エラーが発生したときにコールバックされる。
	void MyCompileErrorFunc(HSQUIRRELVM v, const SQChar* desc, const SQChar* source, SQInteger line, SQInteger column)
	{
		auto writer = MySquirrel::GetErrorWriter();
		if (writer)
		{
			wchar_t buf[PrintTempBufCount] = {};
			swprintf_s(buf, L"Compile Error!! (%Id, %Id) : %s\r\n", line, column, desc);
			writer(ref new Platform::String(buf));
		}
	}
}

MySquirrel::MySquirrel()
	: m_vm()
{
	const SQInteger iniStackSize = 1024;
	m_vm = sq_open(iniStackSize);
	_ASSERTE(m_vm != nullptr);

	m_myTable = std::make_shared<Sqrat::Table>(m_vm);

	// 標準のエラーハンドラーを設定。
	sqstd_seterrorhandlers(m_vm);

	sq_setprintfunc(m_vm, MyPrintFunc, MyErrPrintFunc);
	sq_setcompilererrorhandler(m_vm, MyCompileErrorFunc);
}

MySquirrel::~MySquirrel()
{
	m_myTable.reset();
	if (m_vm)
	{
		sq_close(m_vm);
		m_vm = nullptr;
	}
}

// Linux 系の環境や、Windows 版でもごく一部のライブラリ（Scintilla や FBX SDK など）においては
// UTF-8 エンコードされた文字列を char 配列で内部的に扱うようになっている。
// UTF-8 テキスト ファイルを読み込むときにも、UTF-8 エンコードされた char 配列に接する機会がある。

// コピーコンストラクタよりもムーブ コンストラクタが優先されることを利用する。
static std::vector<char> ConvertUtf16toUtf8(const wchar_t* srcText)
{
	_ASSERTE(srcText != nullptr);
	const int textLen = static_cast<int>(wcslen(srcText));
	const int reqSize = ::WideCharToMultiByte(CP_UTF8, 0, srcText, textLen, nullptr, 0, nullptr, nullptr);
	if (reqSize > 0)
	{
		std::vector<char> buff(reqSize + 1); // 最後の + 1 は必須らしい。終端 null を含まないサイズが返るらしい。
		::WideCharToMultiByte(CP_UTF8, 0, srcText, textLen, &buff[0], reqSize, nullptr, nullptr);
		return buff;
	}
	else
	{
		return std::vector<char>();
	}
}

static std::vector<wchar_t> ConvertUtf8toUtf16(const char* srcText)
{
	_ASSERTE(srcText != nullptr);
	const int textLen = static_cast<int>(strlen(srcText));
	const int reqSize = ::MultiByteToWideChar(CP_UTF8, 0, srcText, textLen, nullptr, 0);
	if (reqSize > 0)
	{
		std::vector<wchar_t> buff(reqSize + 1); // 最後の + 1 は必須らしい。終端 null を含まないサイズが返るらしい。
		::MultiByteToWideChar(CP_UTF8, 0, srcText, textLen, &buff[0], reqSize);
		return buff;
	}
	else
	{
		return std::vector<wchar_t>();
	}
}

// UTF-8 テキスト ファイルから読み込んだ Squirrel スクリプトを実行する場合はこちらを使う。
bool MySquirrel::DoScriptString(const char* script)
{
	_ASSERTE(m_vm != nullptr);
	const auto newText = ConvertUtf8toUtf16(script);
	if (!newText.empty())
	{
		return this->DoScriptString(&newText[0]);
	}
	else
	{
		return false;
	}
}

bool MySquirrel::DoScriptString(const wchar_t* script)
{
	_ASSERTE(m_vm != nullptr);
	_ASSERTE(script != nullptr);
	// Squirrel が Unicode ビルドされているならば、UTF-16LE 文字列をそのまま渡せる。
	return MySqHelpers::DoScriptString(m_vm, script);
}

// Squirrel に公開する関数。
void MySquirrel::PrintLine(const wchar_t* srcText)
{
	_ASSERTE(srcText != nullptr);
	std::wstring temp(srcText);
	temp += L"\r\n";

	// WinRT の Platform::String は COM BSTR ラッパーの bstr_t などと本質的には大差ないので、
	// 普段の文字列結合処理などは std::wstring や ATL::CStringW を使ったほうがパフォーマンスがよい。
	// Platform::String は ABI 境界のみに使用する。

	// Windows ストア アプリでは、char 型を使った ANSI マルチバイト文字列（Shift_JIS/CP932 など）は廃止されている。
	// ASCII のみのときは、OutputDebugStringA() で直接出力できる。非 ASCII が含まれる場合は不可。
	// Windows ストア アプリでも一部の ASCII 用 API は使用可能だが、
	// これらは従来のデスクトップ同様 UTF-8 のマルチバイト文字には対応していない。
	// 
	// なお、OutputDebugStringA() を直接使っていると、Windows ストア アプリの認証キットによる判定が不合格となるので、
	// デバッグ ビルドでのみ有効になるようにマクロでラップしてプリプロセッサで切り替えるなどの対処が必要。
	// ちなみに、OutputDebugStringW() は残っていても不合格にはならない模様。
	// 
	// デスクトップ アプリではデフォルトで OutputDebugStringW() によって非 ASCII 文字を出力することができず、
	// ロケールの明示的な設定が必要だが、ストア アプリはデフォルトで出力可能になっている。
	// ただし .NET の System.Diagnostics.Debug.WriteLine() とは違って、
	// U + 2665H などの Unicode 文字を出力することは依然として不可能。

	//::OutputDebugStringW(temp.c_str());

	auto writerFunc = m_outputWriter;
	if (writerFunc != nullptr)
	{
		writerFunc(ref new Platform::String(temp.c_str()));
	}
}

void MySquirrel::Bind()
{
	// Squirrel には組み込みの print() シンボルはあるが、println() がないので明示的に作る。

	_ASSERTE(m_vm != nullptr);
	auto rootTable = Sqrat::RootTable(m_vm);
	rootTable.Func(_SC("println"), &PrintLine);

	_ASSERTE(m_myTable);

	// TODO: 任意の C++ クラスなどを必要に応じて Sqrat でバインドする。

	// ルートテーブルに "MyTable" としてバインド。ファクトリや名前空間のような役目を果たすことになる。
	rootTable.Bind(_SC("MyTable"), *m_myTable);
}
