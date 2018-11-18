#ifndef _Ci_Common_h_
#define _Ci_Common_h_

namespace Ci {

// TODO ref
class StringReader {
	String s;
	int i = 0;
public:
	StringReader() {}
	StringReader(const String& s) {this->s = s;}
	int Peek() {if (i >= s.GetCount()) return -1; return s[i];}
	int Read() {return s[i++];}
	bool IsEmpty() const {return s.IsEmpty();}
	void Clear() {s = ""; i = 0;}
	void Set(const String& s) {this->s = s; i = 0;}
	
};

class TextWriter {
	Stream* s = NULL;
	FileOut fout;
public:
	
	void Write(char c) {s->Put(c);}
	void Write(String str) {s->Put(str);}
	void WriteLine() {s->Put('\n');}
	void WriteLine(String str) {s->Put(str); s->Put('\n');}
	
	void OpenFile(String fname) {fout.Open(fname); s = &fout;}
	void Close() {fout.Close();}
};

// TODO ref
class SharedString : public String {
	
};

template <class T>
class Stack {
	Vector<T> v;
	
public:
	
	T Pop() {return v.Pop();}
	void Push(const T& o) {v.Add(o);}
	int GetCount() const {return v.GetCount();}
	
	T& operator[](int i) {return v[i];}
	int GetCount() {return v.GetCount();}
	
	T& Peek() {return v.Top();}
};

typedef Exc ParseException;
typedef Exc InvalidOperationException;
typedef Exc NotImplementedException;
typedef Exc NotSupportedException;
typedef Exc ArgumentException;

enum {O_NULL, O_CLASS, O_BOOL, O_BYTE, O_INT, O_STRING, O_ENUM};

struct Object {
	int type = O_NULL;
	bool b = false;
	byte byt = 0;
	int i = 0;
	String s;
	Vector<Object*> objs;
	Vector<byte> bytes;
	
	Object() {}
	Object(bool b) {this->b = b; type = O_BOOL;}
	Object(const Vector<Object*>& objs) {this->objs <<= objs;}
	virtual ~Object() {}
	
	bool Equals(const Object& o) const {
		if (o.type != type) return false;
		switch (type) {
			case O_NULL: return true;
			case O_BOOL: return b == o.b;
			case O_BYTE: return byt == o.byt;
			case O_INT: return i == o.i;
			case O_STRING: return s == o.s;
			default: return this == &o;
		}
	}
	
	virtual String ToString() {
		switch (type) {
			case O_NULL: return "null";
			case O_BOOL: return b ? "true" : "false";
			case O_BYTE: return IntStr(byt);
			case O_INT: return IntStr(i);
			case O_STRING: return s;
			default: return Format("%X", (int64)this);
		}
	}
	
	int GetInt() const {
		switch (type) {
			case O_NULL: return 0;
			case O_BOOL: return b;
			case O_BYTE: return byt;
			case O_INT: return i;
			default: Panic("Invalid value"); throw Exc("");
		}
	}
	
	uint8 GetUnsignedByte() const {
		switch (type) {
			case O_NULL: return 0;
			case O_BOOL: return b;
			case O_BYTE: return byt;
			case O_INT: return i;
			default: Panic("Invalid value"); throw Exc("");
		}
	}
	
	char GetChar() const {
		switch (type) {
			case O_NULL: return 0;
			case O_BOOL: return b;
			case O_BYTE: return byt;
			case O_INT: return i;
			default: Panic("Invalid value"); throw Exc("");
		}
	}
	
	
};



typedef enum {
	EndOfFile,
	Id,
	IntConstant,
	StringConstant,
	Semicolon,
	Dot,
	Comma,
	LeftParenthesis,
	RightParenthesis,
	LeftBracket,
	RightBracket,
	LeftBrace,
	RightBrace,
	Plus,
	Minus,
	Asterisk,
	Slash,
	Mod,
	AndToken,
	OrToken,
	XorToken,
	Not,
	ShiftLeft,
	ShiftRight,
	Equal,
	NotEqual,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
	CondAndToken,
	CondOrToken,
	CondNot,
	Assign,
	AddAssign,
	SubAssign,
	MulAssign,
	DivAssign,
	ModAssign,
	AndAssign,
	OrAssign,
	XorAssign,
	ShiftLeftAssign,
	ShiftRightAssign,
	Increment,
	Decrement,
	QuestionMark,
	Colon,
	DocComment,
	PasteTokens,
	AbstractToken,
	Break,
	Case,
	Class,
	Const,
	Continue,
	Default,
	Delegate,
	Delete,
	Do,
	Else,
	Enum,
	For,
	Goto,
	If,
	InternalToken,
	Macro,
	Native,
	New,
	OverrideToken,
	PublicToken,
	Return,
	StaticToken,
	Switch,
	Throw,
	VirtualToken,
	Void,
	While,
	EndOfLine,
	PreIf,
	PreElIf,
	PreElse,
	PreEndIf
} CiToken;


enum CiPriority
{
	CondExpr,
	CondOrPrior,
	CondAndPrior,
	OrPrior,
	XorPrior,
	AndPrior,
	Equality,
	Ordering,
	Shift,
	Additive,
	Multiplicative,
	Prefix,
	Postfix
};

enum CiVisibility
{
	Dead,
	Private,
	InternalVisib,
	PublicVisib
};

typedef enum
{
	NotYet,
	InProgress,
	Done
} CiWriteStatus;

typedef enum
{
	StaticCallType,
	NormalCallType,
	AbstractCallType,
	VirtualCallType,
	OverrideCallType
} CiCallType;



template <class T>
class PtrIndex {
	Vector<T> v;
	
public:
	
	bool Add(const T& s) {
		for(int i = 0; i < v.GetCount(); i++)
			if (v[i] == s) return false;
		v.Add(s);
		return true;
	}
	void Remove(T ptr) {
		for(int i = 0; i < v.GetCount(); i++)
			if (v[i] == ptr)
				v.Remove(i);
	}
	
	bool HasPtr(T ptr) {
		for(int i = 0; i < v.GetCount(); i++)
			if (v[i] == ptr) return true;
		return false;
	}
	int GetCount() const {return v.GetCount();}
	T& operator[](int i) {return v[i];}
};


template <class K, class V>
class PtrMap {
	Vector<K> k;
	Vector<V> v;
public:
	
	bool Add(const K& k, const V& v) {
		for(int i = 0; i < this->k.GetCount(); i++)
			if (this->k[i] == k) return false;
		this->k.Add(k);
		this->v.Add(v);
	}
	void Remove(K k) {
		for(int i = 0; i < this->k.GetCount(); i++)
			if (this->k[i] == k)
				{this->k.Remove(i); this->v.Remove(i);}
	}
	int Find(K k) {
		for(int i = 0; i < this->k.GetCount(); i++)
			if (this->k[i] == k) return i;
		return -1;
	}
	V& operator[](int i) {return v[i];}
	
	bool HasPtr(K k) {
		return Find(k) != -1;
	}
	
};

template <class T>
bool HasPtr(const Vector<T*>& ptrs, T* ptr) {
	for(int i = 0; i < ptrs.GetCount(); i++)
		if (ptrs[i] == ptr)
			return true;
	return false;
}

}

#endif
