#pragma once

namespace MySqHelpers
{

	struct MyStringLexerArg
	{
		const SQChar* SrcProgram;
	};

	// スクリプトを1文字読むごとにコールバックされる。0を返せば終了。
	inline SQInteger MyStringLexer(SQUserPointer ptr)
	{
		_ASSERTE(ptr != nullptr);
		auto arg = reinterpret_cast<MyStringLexerArg*>(ptr);
		_ASSERTE(arg->SrcProgram != nullptr);
		const auto cc = *(arg->SrcProgram);
		++(arg->SrcProgram);
		return cc;
	}

	// luaL_dostring() 相当関数。Sqrat を使う場合は Script クラスを使ってもよい。
	inline bool DoScriptString(HSQUIRRELVM hvm, const SQChar* srcProgram)
	{
#if 0
		MyStringLexerArg arg = { srcProgram };
		if (SQ_SUCCEEDED(sq_compile(hvm, MyStringLexer, &arg, _SC(""), true)))
#else
		// スクリプト文字列を直接処理してくれる便利関数が用意されている。
		// 内部的には MyStringLexer とほぼ同じことをやっている。
		if (SQ_SUCCEEDED(sq_compilebuffer(hvm, srcProgram, scstrlen(srcProgram), _SC(""), true)))
#endif
		{
			sq_pushroottable(hvm); // VM コンテキストを this として push する。
			sqstd_register_stringlib(hvm);
			sqstd_register_mathlib(hvm);
			// format() 関数を使うためには string ライブラリの初期化と VM への明示的登録が必要となる。
			// 同様に、各種数学関数を使うには math ライブラリを登録する。
			// sq_pushroottable() の後で呼ぶ必要がある。
			// さもないと「the index 'format' does not exist」などのエラーメッセージが出る。
			// HACK: Sqrat の Script クラスを使ってコンパイル＆実行する場合はどうやって登録する？
			auto retVal = sq_call(hvm, 1, false, true);
			return SQ_SUCCEEDED(retVal);
		}
		return false;
	}

}
