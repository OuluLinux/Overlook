#ifndef _Ci_Common_h_
#define _Ci_Common_h_

namespace Ci {

// TODO ref
class StringReader {
	
public:
	StringReader() {}
	StringReader(String& s);
	int Peek();
	int Read();
	bool IsEmpty() const;
	void Clear();
	void Set(const String& s);
	
	void operator=(const StringReader& sr);
	
};

// TODO ref
class SharedString : public String {
	
};

template <class T>
class Stack {
	
public:
	
	T Pop();
	void Push(const T& o);
	int GetCount() const;
	
	T& operator[](int i);
	int GetCount();
	
	T& Peek();
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
	void* data = NULL;
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
			default: return data == o.data;
		}
	}
	
	String ToString() const {
		switch (type) {
			case O_NULL: return "NULL";
			case O_BOOL: return b ? "true" : "false";
			case O_BYTE: return IntStr(byt);
			case O_INT: return IntStr(i);
			case O_STRING: return s;
			default: return Format("%X", (int64)data);
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
	
public:
	
	T& Add(const T& s);
	
};

}

#endif
