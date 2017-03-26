#pragma once

delegate void WriteStringDelegate(Platform::String^ str);

// Squirrel, Sqrat のテスト用クラス。
// Squirrel VM の RAII を兼ねる。
class MySquirrel
{
public:
	typedef std::shared_ptr<MySquirrel> TSharedPtr;
private:
	HSQUIRRELVM m_vm;
	std::shared_ptr<Sqrat::Table> m_myTable;
private:
	static WriteStringDelegate^ m_outputWriter;
	static WriteStringDelegate^ m_errorWriter;
	// C++/CLI ではネイティブ型にマネージ型を含める際、gcroot クラス テンプレートによるラッパーが必要だったが、
	// C++/CX では、通常の C++ クラスにそのまま ref クラスのハンドル変数をメンバーとして含めることができる。
	// ただし property もしくは event にしたければ、ref クラス内に含める必要がある。
	// 今回は ref クラスにはしないので、普通に getter/setter を書く。
public:
	MySquirrel();
	~MySquirrel();
	void Bind();
	bool DoScriptString(const char* script);
	bool DoScriptString(const wchar_t* script);
public:
	static WriteStringDelegate^ GetOutputWriter()
	{ return m_outputWriter; }
	static WriteStringDelegate^ GetErrorWriter()
	{ return m_errorWriter; }

	static void SetOutputWriter(WriteStringDelegate^ writer)
	{ m_outputWriter = writer; }
	static void SetErrorWriter(WriteStringDelegate^ writer)
	{ m_errorWriter = writer; }
private:
	static void PrintLine(const wchar_t* srcText);
};
