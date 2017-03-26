//
// MainPage.xaml.cpp
// MainPage クラスの実装。
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace StoreAppSquirrelTest1;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// 空白ページのアイテム テンプレートについては、http://go.microsoft.com/fwlink/?LinkId=234238 を参照してください

MainPage::MainPage()
{
	InitializeComponent();

	m_sq = std::make_shared<MySquirrel>();

	m_sq->Bind();

	m_sq->SetOutputWriter(ref new WriteStringDelegate([this](Platform::String^ str)
	{
		if (this->textboxOutput != nullptr)
		{
			this->textboxOutput->Text += str;
		}
	}));

	m_sq->SetErrorWriter(ref new WriteStringDelegate([this](Platform::String^ str)
	{
		if (this->textboxError != nullptr)
		{
			this->textboxError->Text += str;
		}
	}));

#if 1
	this->textboxInput->Text =
		L"// エスプレッソ\r\n"
		L"// 実行可能\r\n"
		L"print(\"hoge日本語ソース\\n\");\r\n"
		L"local x = 10.1;\r\n"
		L"println(x);\r\n"
		L"local y = 1 | 2;\r\n"
		L"println(y);\r\n"
		L"z <- \"foobar\";\r\n"
		L"println(z + (sin(PI)).tointeger());\r\n"
		L"w <- 0xFFFF + 1;\r\n"
		L"println(w);\r\n"
		L"for (local i = 0; i < 3; ++i) { println(rand()); }\r\n"
		L"r = 0.5;\r\n" // 実行時エラー。
		L"println(r);\r\n" // 実行時エラー。
		;
	// Squirrel では未定義の変数は使えない。
	// ただし今回は簡単のため、スクリプトを実行する際に毎回 VM のスロットを初期化してはいないので、
	// 以前実行したスクリプトで同名グローバル変数を定義していた場合はそのまま使えてしまう。
#endif
}

/// <summary>
/// このページがフレームに表示されるときに呼び出されます。
/// </summary>
/// <param name="e">このページにどのように到達したかを説明するイベント データ。Parameter 
/// プロパティは、通常、ページを構成するために使用します。</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// 未使用のパラメーター
}

void MainPage::buttonExecute_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
#if 0
	// NG。
	const auto scriptString0 = this->textboxInput->Text->Data();
	const std::wstring scriptString = scriptString0;
	const auto scriptStringPtr = scriptString.c_str();
#elif 1
	// OK。
	const auto scriptText = this->textboxInput->Text;
	const auto scriptStringPtr = scriptText->Data();
#elif 0
	// OK。
	const auto scriptText = this->textboxInput->Text;
	const std::wstring scriptString = scriptText->Data();
	const auto scriptStringPtr = scriptString.c_str();
#else
	// OK。
	const std::wstring scriptString = this->textboxInput->Text->Data();
	const auto scriptStringPtr = scriptString.c_str();
#endif
	// NOTE: 文法は正しいはずの箇所でどうも変なエラー（[the index 'Ī' does not exist] [expected ';'] など）が発生する。
	// どうやら原因はメモリーの無効化らしい。WinRT 自体のメモリー管理は .NET のような GC ではなく、COM 同様の参照カウント方式だが、
	// Control のプロパティから Platform::String::Data() で取得した一時ポインタを直接ポインタ変数に保持してはいけない。
	// つまり Data() はピン止めをしているわけではない。
	// ポインタとは別に Platform::String^ 一時変数も用意しておくか、
	// ユーザー定義の文字列バッファ（std::wstring や ATL::CStringW など）にすぐにコピーする必要がある。
	// http://msdn.microsoft.com/en-us/library/hh825860.aspx
	// const wchar_t* 一時変数を経由してコピーした場合、取得していた一時ポインタが無効になっていることがある。
	// 要するに Text プロパティを取得して Platform::String^ オブジェクトを取得するとき、
	// そのハンドルをローカル変数に保存せずに Data() 経由で const wchar_t* 内部ポインタのみを保存しようとしていたのが間違い。
	// Platform::String^ 一時ハンドルの寿命が const wchar_t* 一時ポインタの寿命。
	// Text プロパティで得られたオブジェクトの寿命は TextBox^ の寿命と同じではない。
	// WinRT の UI 経由で取得した文字列を C++ で処理する際は常に注意する必要がある。
	_ASSERTE(m_sq);
	m_sq->DoScriptString(scriptStringPtr);

	//::OutputDebugStringW(L"Not implemented!!\n");

	// WPF とは違い、Windows ストア アプリ（Windows ランタイム アプリ）には System.Windows.Media.Brushes クラスがないので、
	// コード ビハインドで Windows::UI::Xaml::Control.Background などを設定する場合は
	// Windows::UI::Colors を使って Windows::UI::Xaml::Media::SolidColorBrush を ref new するしかない？
	// WinForms, WPF, WinRT はそれぞれで似たようなデータ型が定義されていて、ある程度類推はしやすいが、
	// Web 検索するときは名前空間をすべて指定しないとなかなか思うように正確な情報がヒットしないのが厄介（しかも WPF や WinRT という名称は名前空間に使われていない）。
}


void MainPage::buttonClearOutput_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->textboxOutput->Text = L"";
}


void MainPage::buttonClearError_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->textboxError->Text = L"";
}

// PointerEntered/PointerExited のタイミングで TextBox::Background の輝度（不透明度？）が変動するようなデフォルト スタイルになっているらしく、
// 特に TextBox の面積が広い場合はちらちらして非常に目障り。
// WinRT では簡単に背景色を固定することができないようなので、とりあえず Transparent にしておく方法があるが、
// WinRT には WPF/Silverlight のような CaretBrush プロパティがなく、キャレットの色は常に黒らしいので、
// TextBox の背面に暗い色のオブジェクトがある場合は、TextBox が Transparent だとキャレットが判別できなくなるという最悪仕様。
// WinRT は貧弱すぎる。
// http://stackoverflow.com/questions/12377609/change-caret-color-in-winrt
#if 0
void MainPage::textboxInput_PointerEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	::OutputDebugStringW(__FUNCTIONW__ L"\n");
}


void MainPage::textboxInput_PointerExited(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	::OutputDebugStringW(__FUNCTIONW__ L"\n");
}


void MainPage::textboxInput_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	::OutputDebugStringW(__FUNCTIONW__ L"\n");
}
#endif
