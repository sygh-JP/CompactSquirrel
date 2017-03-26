#pragma once

// Squirrel は Lua と異なり、整数型と実数型の区別があるが、倍精度浮動小数はデフォルトで提供されていないので注意。
// SQUSEDOUBLE というシンボルをビルド時に定義しておけば Squirrel でも float の代わりに double が使えるようになるらしい。

class MyVector2F
{
public:
	float x, y;
public:
	MyVector2F()
		: x(), y()
	{}
	explicit MyVector2F(float inX, float inY)
		: x(inX), y(inY)
	{}
public:
	MyVector2F& operator+=(const MyVector2F& other)
	{
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	MyVector2F& operator-=(const MyVector2F& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	MyVector2F& operator*=(float f)
	{
		this->x *= f;
		this->y *= f;
		return *this;
	}
	MyVector2F& operator/=(float f)
	{
		return ((*this) *= (1.0f / f));
	}
public:
	float GetLengthSq() const
	{
		return x * x + y * y;
	}
	float GetLength() const
	{
		return sqrt(this->GetLengthSq());
	}
public:
	MyVector2F Clone()
	{
		return *this;
	}
public:
	void Print() const
	{
		printf("(%f, %f)\n", x, y);
	}
public:
	static MyVector2F Create(float inX, float inY)
	{
		return MyVector2F(inX, inY);
	}
};

inline float GetDistance(const MyVector2F& v1, const MyVector2F& v2)
{
	return MyVector2F(v1.x - v2.x, v1.y - v2.y).GetLength();
}


inline void BindMyVector2F(Sqrat::Table& myTable)
{
	using namespace Sqrat;

	// 今回はデフォルト VM は使わない。
	auto vm = myTable.GetVM();

	// テーブルに C++ クラスをバインド。
	// C# のようなプロパティもサポートしている。
	// HACK: コピーコンストラクタや引数付きコンストラクタはどうやってバインドする？
	// どうやら Sqrat 0.8 時点ではまだ引数付きコンストラクタのバインドがサポートされていないらしいが、将来的に対応する予定らしい。
	// ちなみに Luabind ではともにバインド可能。C++ コンパイラが自動生成する暗黙のコピーコンストラクタをバインドすることもできる。
	// http://scrat.sourceforge.net/binding.html#limitations
	myTable.Bind(
		_SC("MyVector2F"),
		Class<MyVector2F>(vm, Sqrat::string(), true)
		.Func(_SC("Add"), &MyVector2F::operator+=)
		.Func(_SC("Sub"), &MyVector2F::operator-=)
		.Func(_SC("Mul"), &MyVector2F::operator*=)
		.Func(_SC("Div"), &MyVector2F::operator/=)
		.Prop(_SC("LengthSq"), &MyVector2F::GetLengthSq)
		.Prop(_SC("Length"), &MyVector2F::GetLength)
		.Func(_SC("Clone"), &MyVector2F::Clone)
		.Func(_SC("Print"), &MyVector2F::Print)
		.StaticFunc(_SC("Create"), &MyVector2F::Create)
		.Var(_SC("x"), &MyVector2F::x)
		.Var(_SC("y"), &MyVector2F::y)
		);

	// テーブルにグローバル関数をバインド。
	// 引数の型や数が異なる関数オーバーロードは Overload() で渡せる？　名前を変える必要がある？
	myTable.Func(_SC("GetDistance"), &GetDistance);
}

inline void DoMyTest(MyVector2F v1)
{
	// Sqrat を使ってバインドするときにメンバー関数（およびメンバー演算子オーバーロード）へのポインタを取得しているため、
	// 最適化を有効にしたときのメンバー関数呼び出しのインライン化が阻害される可能性を危惧したが、
	// 少なくとも VC++ 2012 の場合、C++ 側で通常使用される箇所ではインライン展開されることを確認（/FAs）。
	// ちなみに v1 を関数引数ではなくローカル変数として定数割り当てすると、
	// 下記の一連の算術演算はすべてコンパイル時に事前計算されて定数が埋め込まれる模様。

	v1.Print();

	v1 -= MyVector2F(3.0f, 1.0f);
	v1 /= 2.0f;

	v1.Print();
}
