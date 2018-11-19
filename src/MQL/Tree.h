
namespace Ci
{



struct CiDocInline {
	String text;
	
	CiDocInline() {}
	CiDocInline(String s) {text = s;}
	virtual ~CiDocInline() {}
};

struct CiDocText : public CiDocInline
{
	
	void operator=(const CiDocText& c) {
		text = c.text;
	}
};

struct CiDocCode : public CiDocInline
{
	
	void operator=(const CiDocCode& c) {
		text = c.text;
	}
};

struct CiDocBlock {
	
	virtual ~CiDocBlock() {}
};

struct CiDocPara : public CiDocBlock
{
	Vector<CiDocInline*> children;
	
	CiDocPara() {}
	CiDocPara(const Vector<CiDocInline*>& children) {this->children <<= children;}
	void operator=(const CiDocPara& c) {
		children <<= c.children;
	}
};

struct CiDocList : public CiDocBlock
{
	Vector<CiDocPara*> items;
	
	CiDocList() {}
	CiDocList(const Vector<CiDocPara*>& items) {this->items <<= items;}
	void operator=(const CiDocList& c) {
		items <<= c.items;
	}
};

struct CiCodeDoc
{
	CiDocPara* summary = NULL;
	Vector<CiDocBlock*> details;
	
	CiCodeDoc() {}
	CiCodeDoc(CiDocPara* summary, const Vector<CiDocBlock*>& details) {this->summary = summary; this->details <<= details;}
	void operator=(const CiCodeDoc& c) {
		summary = c.summary;
		details <<= c.details;
	}
};

struct CiEnum;
struct CiConst;
struct CiField;
struct CiMethod;
struct CiClass;
struct CiDelegate;

struct ICiSymbolVisitor
{
	virtual void VisitSymbol(CiEnum* symbol) = 0;
	virtual void VisitSymbol(CiConst* symbol) = 0;
	virtual void VisitSymbol(CiField* symbol) = 0;
	virtual void VisitSymbol(CiMethod* symbol) = 0;
	virtual void VisitSymbol(CiClass* symbol) = 0;
	virtual void VisitSymbol(CiDelegate* symbol) = 0;
};

struct CiSymbol : public Object
{
	CiCodeDoc* documentation;
	CiVisibility visibility;
	String name;
	
	CiSymbol() {Object::type = O_CLASS;}
	void operator=(const CiSymbol& c) {
		documentation = c.documentation;
		visibility = c.visibility;
		name = c.name;
	}
	
	virtual void Accept(ICiSymbolVisitor* v) { throw Exc("CiSymbol exception"); }
};

struct SymbolTable {
	VectorMap<String, CiSymbol*> symbols;
	SymbolTable* parent = NULL;
	
	SymbolTable() {}
	SymbolTable(SymbolTable* p) {parent = p;}
	
	void Add(CiSymbol* sym) {symbols.Add(sym->name, sym);}
	
	CiSymbol* Lookup(const String& name) {
		int i = symbols.Find(name);
		if (i < 0) {
			if (parent)
				return parent->Lookup(name);
			return NULL;
		}
		return symbols[i];
	}
	
	int GetCount() {return symbols.GetCount();}
	CiSymbol* Get(int i) {return symbols[i];}
	
};

struct ICiTypeVisitor;
struct CiClass;

struct CiType : public CiSymbol
{
	static CiType* Null;
	static CiType* Void;
	
	CiType() {}
	CiType(String name) {this->name = name;}
	CiType(const CiType& c) {*this = c;}
	void operator=(const CiType& c) {CiSymbol::operator=(c);}
	
	virtual CiType& BaseType() { return *this; }
	virtual int ArrayLevel() { return 0; }
	virtual CiType* Ptr() { return NULL; }
	virtual CiSymbol* LookupMember(String name) {
		throw Exc("CiType " + name + " has no members");
	}
	virtual CiType* Accept(ICiTypeVisitor* v) { return this; }
	virtual bool Equals(CiType* obj) { return this == obj; }
	virtual CiClass* StorageClass() { return NULL; }
	
};

struct CiUnknownType;
struct CiStringStorageType;
struct CiClassType;
struct CiArrayType;
struct CiArrayStorageType;
struct CiDelegate;

struct ICiTypeVisitor
{
	virtual CiType* VisitType(CiUnknownType* type) = 0;
	virtual CiType* VisitType(CiStringStorageType* type) = 0;
	virtual CiType* VisitType(CiClassType* type) = 0;
	virtual CiType* VisitType(CiArrayType* type) = 0;
	virtual CiType* VisitType(CiArrayStorageType* type) = 0;
	virtual CiType* VisitType(CiDelegate* type) = 0;
};

struct CiUnknownType : public CiType
{
	virtual CiType* Accept(ICiTypeVisitor& v) { return v.VisitType(this); }
};

struct CiBoolType : public CiType
{
	CiBoolType() {}
	static CiBoolType* Value() { static CiBoolType* c; if (!c) {c = new CiBoolType(); c->name = "bool";} return c;}
	
	virtual CiSymbol* LookupMember(String name);
};

struct CiByteType : public CiType
{
	CiByteType() {}
	static CiByteType* Value() {static CiByteType* c; if (!c) {c  = new CiByteType(); c->name = "byte";} return c;}
	
	virtual CiSymbol* LookupMember(String name);
};

struct CiIntType : public CiType
{
	CiIntType() {}
	static CiIntType* Value() {static CiIntType* c; if (!c) {c  = new CiIntType(); c->name = "int";} return c;}
	
	virtual CiSymbol* LookupMember(String name);
};

struct CiStringType : public CiType
{
	virtual CiSymbol* LookupMember(String name);
};

struct CiStringPtrType : public CiStringType
{
	CiStringPtrType() {}
	static CiStringPtrType* Value() {static CiStringPtrType* c; if (!c) {c  = new CiStringPtrType(); c->name = "string";} return c;}
};

struct CiExpr;

struct CiStringStorageType : public CiStringType
{
	CiExpr* length_expr = NULL;
	int length = 0;
	
	CiStringStorageType() {}
	CiStringStorageType(CiExpr* len) {length_expr = len;}
	virtual bool Equals(CiType& obj)
	{
		CiStringStorageType* that = dynamic_cast<CiStringStorageType*>(&obj);
		return that != NULL && this->length == that->length;
	}
	virtual CiType* Ptr() {return CiStringPtrType::Value();}
	virtual CiType* Accept(ICiTypeVisitor& v) { return v.VisitType(this); }
};

enum PtrWritability
{
	Unknown,
	ReadOnly,
	ReadWrite
};

struct ICiPtrType
{
	PtrWritability writability;
	PtrIndex<ICiPtrType*> sources;
};

struct CiArrayType : public CiType
{
	CiType* element_type = NULL;
	
	virtual CiType& BaseType() {return this->element_type->BaseType();}
	virtual int ArrayLevel() { return 1 + this->element_type->ArrayLevel();}
	virtual CiSymbol* LookupMember(String name);
	virtual CiType* Accept(ICiTypeVisitor& v) { return v.VisitType(this); }
};

struct CiArrayPtrType : public CiArrayType, ICiPtrType
{
	static CiArrayPtrType* WritableByteArray;
	
	PtrWritability writability = Unknown;
	PtrIndex<ICiPtrType*> sources;
	
	CiArrayPtrType() {}
	CiArrayPtrType(CiType* type) {element_type = type;}
	virtual bool Equals(CiType& obj) {
		CiArrayPtrType* that = dynamic_cast<CiArrayPtrType*>(&obj);
		return that != NULL && this->element_type->Equals(that->element_type);
	}
};

struct CiExpr;

struct CiArrayStorageType : public CiArrayType
{
	CiExpr* length_expr = NULL;
	int length;
	
	
	CiArrayStorageType() {}
	CiArrayStorageType(CiExpr* len, CiType* el_type) {length_expr = len; element_type = el_type;}
	CiArrayStorageType(CiType* el_type, int length) {element_type = el_type; this->length = length;}
	virtual CiType* Ptr() {return new CiArrayPtrType(this->element_type); }
	virtual CiSymbol* LookupMember(String name);
	virtual CiType* Accept(ICiTypeVisitor& v) { return v.VisitType(this); }
	virtual CiClass* StorageClass() {return this->element_type->StorageClass();}
};

struct CiProperty;

struct CiLibrary
{
	static CiProperty* LowByteProperty;
	static CiProperty* SByteProperty;
	static CiMethod* MulDivMethod;
	static CiProperty* StringLengthProperty;
	static CiMethod* CharAtMethod;
	static CiMethod* SubstringMethod;
	static CiMethod* Arraycopy_toMethod;
	static CiMethod* ArrayToStringMethod;
	static CiMethod* ArrayStorageClearMethod;
};

struct CiUnknownSymbol : public CiSymbol {
	
	CiUnknownSymbol() {}
	CiUnknownSymbol(String name) {this->name = name;}
};

struct CiEnum;

struct CiEnumValue : public CiSymbol {
	CiEnum* type = NULL;
	
	CiEnumValue() {Object::type = O_ENUM;}
};

struct CiEnum : public CiType
{
	Vector<CiEnumValue*> values;
	
	virtual CiSymbol* LookupMember(String name)
	{
		CiEnumValue* value = NULL;
		for(int i = 0; i < values.GetCount(); i++) {
			if (values[i]->name == name)
				value = values[i];
		}
		if (value == NULL)
			throw ParseException(name + " not found in enum " + this->name);
		return value;
	}
	virtual void Accept(ICiSymbolVisitor* v) { v->VisitSymbol(this); }
};

struct CiField : CiSymbol
{
	CiClass* class_;
	CiType* type;
	
	CiField(CiClass* cls, CiType* type, String name) {cls = class_; this->type = type; this->name = name;}
	virtual void Accept(ICiSymbolVisitor* v) { v->VisitSymbol(this); }
};

struct CiProperty : CiSymbol
{
	CiType* type = NULL;
	
	virtual void Accept(ICiSymbolVisitor* v) { }
	
	CiProperty() {}
	CiProperty(String name, CiType* type) {this->name = name; this->type = type;}
};

struct CiBlock;
struct CiConst;
struct CiVar;
struct CiExpr;
struct CiAssign;
struct CiDelete;
struct CiBreak;
struct CiContinue;
struct CiDoWhile;
struct CiFor;
struct CiIf;
struct CiNativeBlock;
struct CiReturn;
struct CiSwitch;
struct CiThrow;
struct CiWhile;

struct ICiStatementVisitor
{
	virtual void VisitStmt(CiBlock* statement) = 0;
	virtual void VisitStmt(CiConst* statement) = 0;
	virtual void VisitStmt(CiVar* statement) = 0;
	virtual void VisitStmt(CiExpr* statement) = 0;
	virtual void VisitStmt(CiAssign* statement) = 0;
	virtual void VisitStmt(CiDelete* statement) = 0;
	virtual void VisitStmt(CiBreak* statement) = 0;
	virtual void VisitStmt(CiContinue* statement) = 0;
	virtual void VisitStmt(CiDoWhile* statement) = 0;
	virtual void VisitStmt(CiFor* statement) = 0;
	virtual void VisitStmt(CiIf* statement) = 0;
	virtual void VisitStmt(CiNativeBlock* statement) = 0;
	virtual void VisitStmt(CiReturn* statement) = 0;
	virtual void VisitStmt(CiSwitch* statement) = 0;
	virtual void VisitStmt(CiThrow* statement) = 0;
	virtual void VisitStmt(CiWhile* statement) = 0;
};

struct ICiStatement : public Object
{
	ICiStatement() {Object::type = O_CLASS;}
	virtual bool CompletesNormally() = 0;
	virtual void Accept(ICiStatementVisitor* v) = 0;
};

struct CiConst : public CiSymbol, ICiStatement
{
	CiClass* class_ = NULL;
	CiType* type = NULL;
	Object* value = NULL;
	String global_name;
	bool is_7bit = false;
	bool currently_resolving = false;
	
	CiConst() {}
	CiConst(CiType* type, Object* v) {this->type = type; value = v;}
	CiConst(String name, CiType* type, Object* v) {this->name = name; this->type = type; value = v;}
	virtual bool CompletesNormally() { return true; }
	virtual void Accept(ICiSymbolVisitor* v) { v->VisitSymbol(this); }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiVar : public CiSymbol, ICiStatement
{
	CiType* type = NULL;
	CiExpr* initial_value = NULL;
	bool write_initial_value = false; // C89 only
	
	virtual bool CompletesNormally() { return true; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiBinaryResource : public CiSymbol
{
	Vector<byte> content;
	CiArrayStorageType* type = NULL;
};

struct CiParam : public CiVar
{
	CiParam() { }
	CiParam(CiType* type, String name)
	{
		this->type = type;
		this->name = name;
	}
};

struct CiMaybeAssign : public ICiStatement
{
protected:
	CiType* type = NULL;
	
public:
	virtual CiType* Type() {return type;}
	virtual bool CompletesNormally() {Panic("Undefined"); return false;}
	virtual void Accept(ICiStatementVisitor* v) {Panic("Undefined");}
};

struct CiSymbolAccess;
struct CiVarAccess;
struct CiPropertyAccess;
struct CiUnknownMemberAccess;
struct CiIndexAccess;
struct CiMethodCall;
struct CiUnaryExpr;
struct CiCondNotExpr;
struct CiPostfixExpr;
struct CiBinaryExpr;
struct CiBoolBinaryExpr;
struct CiCondExpr;
struct CiBinaryResourceExpr;
struct CiNewExpr;

struct ICiExprVisitor
{
	virtual CiExpr* VisitExpr(CiSymbolAccess* expr) = 0;
	virtual CiExpr* VisitExpr(CiVarAccess* expr) = 0;
	virtual CiExpr* VisitExpr(CiPropertyAccess* expr) = 0;
	virtual CiExpr* VisitExpr(CiUnknownMemberAccess* expr) = 0;
	virtual CiExpr* VisitExpr(CiIndexAccess* expr) = 0;
	virtual CiExpr* VisitExpr(CiMethodCall* expr) = 0;
	virtual CiExpr* VisitExpr(CiUnaryExpr* expr) = 0;
	virtual CiExpr* VisitExpr(CiCondNotExpr* expr) = 0;
	virtual CiExpr* VisitExpr(CiPostfixExpr* expr) = 0;
	virtual CiExpr* VisitExpr(CiBinaryExpr* expr) = 0;
	virtual CiExpr* VisitExpr(CiBoolBinaryExpr* expr) = 0;
	virtual CiExpr* VisitExpr(CiCondExpr* expr) = 0;
	virtual CiExpr* VisitExpr(CiBinaryResourceExpr* expr) = 0;
	virtual CiExpr* VisitExpr(CiNewExpr* expr) = 0;
};

struct CiExpr : public CiMaybeAssign
{
	virtual bool IsConst(Object* value) { return false; }
	virtual bool HasSideEffect() = 0;
	virtual CiExpr* Accept(ICiExprVisitor* v) { return this; }
	virtual String ToString() {return "";}
};

struct CiConstExpr : public CiExpr
{
	Object* value;
	
	
	CiConstExpr(Object* value) {
		if (value == NULL)
			this->value = new Object();
		else
			this->value = value;
	}
	CiConstExpr(int value) {
		this->value = new Object();
		this->value->type = O_INT;
		this->value->i = value;
	}
	CiConstExpr(String value) {
		this->value = new Object();
		this->value->type = O_STRING;
		this->value->s = value;
	}
	~CiConstExpr() {delete value; value = NULL;}
	
	virtual CiType* Type()
	{
		if (value->type == O_BOOL) return CiBoolType::Value();
		if (value->type == O_BYTE) return CiByteType::Value();
		if (value->type == O_INT) return CiIntType::Value();
		if (value->type == O_STRING) return CiStringPtrType::Value();
		if (value->type == O_NULL) return CiType::Null;
		if (value->type == O_ENUM) return (dynamic_cast<CiEnumValue*>(this->value))->type;
		if (value->type == O_CLASS) Panic("UNKNOWN");
		throw NotImplementedException();
	}
	virtual bool IsConst(Object* value) { return value->Equals(this->value); }
	virtual bool HasSideEffect() {return false;}
	virtual String ToString() { return this->value->ToString(); }
};

struct CiLValue : public CiExpr {
	
};

struct CiSymbolAccess : public CiExpr
{
	CiSymbol* symbol;
	
	CiSymbolAccess() {}
	CiSymbolAccess(CiSymbol* sym) {symbol = sym;}
	virtual CiType* Type() {throw NotSupportedException();}
	virtual bool HasSideEffect() {throw NotSupportedException();}
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
	virtual String ToString() { return this->symbol->name; }
};

struct CiConstAccess : public CiExpr
{
	CiConst* const_;
	
	CiConstAccess(CiConst* const_) {this->const_ = const_;}
	virtual CiType* Type() {return this->const_->type;}
	virtual bool HasSideEffect() {return false;}
};

struct CiVarAccess : public CiLValue
{
	CiVar* var;
	
	CiVarAccess(CiVar* var) {this->var = var; type = var->type;}
	virtual CiType* Type() {return this->var->type;}
	virtual bool HasSideEffect() {return false;}
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
};

struct CiUnknownMemberAccess : public CiExpr
{
	CiExpr* parent = NULL;
	String name;
	
	CiUnknownMemberAccess() {}
	CiUnknownMemberAccess(CiExpr* parent, String name) {this->parent = parent; this->name = name;}
	virtual CiType* Type(){ throw NotSupportedException(); }
	virtual bool HasSideEffect(){ throw NotSupportedException(); }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
	virtual String ToString() { return this->parent->ToString() + "." + this->name; }
};

struct CiFieldAccess : public CiLValue
{
	CiExpr* obj = NULL;
	CiField* field = NULL;
	
	CiFieldAccess(CiExpr* obj, CiField* field) {this->obj = obj; this->field = field;}
	virtual CiType* Type() { return this->field->type; }
	virtual bool HasSideEffect() { return this->obj->HasSideEffect(); }
};

struct CiPropertyAccess : public CiExpr
{
	CiExpr* obj = NULL;
	CiProperty* property = NULL;
	
	CiPropertyAccess(CiExpr* parent, CiProperty* prop) {obj = parent; property = prop;}
	virtual CiType* Type() {return this->property->type;}
	virtual bool HasSideEffect() {return this->obj->HasSideEffect();}
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
};

struct CiIndexAccess : public CiExpr
{
	CiExpr* parent = NULL;
	CiExpr* index = NULL;
	
	CiIndexAccess() {}
	CiIndexAccess(CiExpr* parent, CiExpr* index) {this->parent = parent; this->index = index;}
	virtual CiType* Type() { throw NotSupportedException(); }
	virtual bool HasSideEffect() { throw NotSupportedException(); }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
	virtual String ToString() { return this->parent->ToString() + "[" + this->index->ToString() + "]"; }
};

struct CiArrayAccess : public CiLValue
{
	CiExpr* array = NULL;
	CiExpr* index = NULL;
	
	CiArrayAccess(CiExpr* parent, CiExpr* i) {array = parent; index = i;}
	virtual CiType* Type() { return (dynamic_cast<CiArrayType*>(this->array->Type()))->element_type; }
	virtual bool HasSideEffect() {return this->array->HasSideEffect() || this->index->HasSideEffect(); }
};

struct CiUnaryExpr : public CiExpr
{
	CiToken op;
	CiExpr* inner = NULL;
	
	CiUnaryExpr() {}
	CiUnaryExpr(CiToken op, CiExpr* inner) {this->op = op; this->inner = inner;}
	virtual CiType* Type() { return CiIntType::Value(); }
	virtual bool HasSideEffect() { return op == Increment || op == Decrement || inner->HasSideEffect(); }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
};

struct CiCondNotExpr : public CiExpr
{
	CiExpr* inner = NULL;
	
	CiCondNotExpr() {}
	CiCondNotExpr(CiExpr* inner) {this->inner = inner;}
	virtual CiType* Type() { return CiBoolType::Value(); }
	virtual bool HasSideEffect() { return this->inner->HasSideEffect(); }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
};

struct CiPostfixExpr : public CiExpr
{
	CiExpr* inner = NULL;
	CiToken op;
	
	CiPostfixExpr() {}
	CiPostfixExpr(CiExpr* inner, CiToken op) {this->inner = inner; this->op = op;}
	virtual CiType* Type() { return CiIntType::Value(); }
	virtual bool HasSideEffect() { return true; }
	virtual bool CompletesNormally() { return true; }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiBinaryExpr : public CiExpr
{
	CiExpr* left = NULL;
	CiToken op;
	CiExpr* right = NULL;
	
	CiBinaryExpr(CiExpr* left, CiToken op, CiExpr* right) {this->left = left; this->op = op; this->right = right;}
	
	String OpString()
	{
		switch (this->op) {
			case Plus: return "+";
			case Minus: return "-";
			case Asterisk: return "*";
			case Slash: return "/";
			case Mod: return "%";
			case ShiftLeft: return "<<";
			case ShiftRight: return ">>";
			case Less: return "<";
			case LessOrEqual: return "<=";
			case Greater: return ">";
			case GreaterOrEqual: return ">=";
			case Equal: return "==";
			case NotEqual: return "!=";
			case AndToken: return "&";
			case OrToken: return "|";
			case XorToken: return "^";
			case CondAndToken: return "&&";
			case CondOrToken: return "||";
			default: throw ArgumentException(IntStr(this->op));
		}
	}
	virtual CiType* Type() { return CiIntType::Value(); }
	virtual bool HasSideEffect() { return this->left->HasSideEffect() || this->right->HasSideEffect(); }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
	virtual String ToString() { return "(" + this->left->ToString() + " " + this->OpString() + " " + this->right->ToString() + ")"; }
};

struct CiBoolBinaryExpr : public CiBinaryExpr
{
	CiBoolBinaryExpr(CiExpr* left, CiToken op, CiExpr* right) : CiBinaryExpr(left, op, right) {}
	
	virtual CiType* Type() { return CiBoolType::Value(); }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
};

struct CiCondExpr : public CiExpr
{
	CiExpr* cond = NULL;
	CiType* result_type = NULL;
	CiExpr* on_true = NULL;
	CiExpr* on_false = NULL;
	
	CiCondExpr() {}
	CiCondExpr(CiExpr* c, CiType* rt, CiExpr* t, CiExpr* f) {cond = c; result_type = rt; on_true = t; on_false = f;}
	virtual CiType* Type() { return this->result_type; }
	virtual bool HasSideEffect() {return this->cond->HasSideEffect() || this->on_true->HasSideEffect() ||this->on_false->HasSideEffect(); }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
	virtual String ToString() { return "(" + this->cond->ToString() + " ? " + this->on_true->ToString() + " : " + this->on_false->ToString() + ")"; }
};

struct CiBinaryResourceExpr : public CiExpr
{
	CiExpr* name_expr = NULL;
	CiBinaryResource* resource = NULL;
	
	CiBinaryResourceExpr() {}
	CiBinaryResourceExpr(CiExpr* name) {name_expr = name;}
	virtual CiType* Type() { return this->resource->type; }
	virtual bool HasSideEffect() { return false; }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
};

struct CiNewExpr : public CiExpr
{
	CiType* new_type = NULL;
	
	CiNewExpr() {}
	CiNewExpr(CiType* newtype) {new_type = newtype;}
	virtual CiType* Type() { return this->new_type->Ptr(); }
	virtual bool HasSideEffect() { return true; }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
};

struct CiCoercion : public CiExpr
{
	CiType* result_type = NULL;
	CiMaybeAssign* inner;
	
	CiCoercion(CiType* rt, CiMaybeAssign* in) {result_type = rt; inner = in;}
	virtual CiType* Type() { return this->result_type; }
	virtual bool HasSideEffect() { return ((CiExpr*) this->inner)->HasSideEffect(); }
};

struct CiAssign : public CiMaybeAssign
{
	CiExpr* target = NULL;
	CiToken op;
	CiMaybeAssign* source = NULL;
	
	virtual CiType* Type() { return this->target->Type(); }
	virtual bool CompletesNormally() { return true; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiDelete : public ICiStatement
{
	CiExpr* expr = NULL;
	
	CiDelete(CiExpr* expr) {this->expr = expr;}
	virtual bool CompletesNormally() { return true; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiCondCompletionStatement : public ICiStatement
{
	bool completes_normally = false;
	
	virtual bool CompletesNormally() {return completes_normally;}
	virtual void Accept(ICiStatementVisitor* v) = 0;
};

struct CiLoop : public CiCondCompletionStatement
{
	CiExpr* cond = NULL;
	ICiStatement* body = NULL;
};

struct CiBlock : public CiCondCompletionStatement
{
	Vector<ICiStatement*> statements;
	
	CiBlock(const Vector<ICiStatement*>& statements) {this->statements <<= statements;}
	virtual void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiBreak : public ICiStatement
{
	bool CompletesNormally() { return false; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiContinue : public ICiStatement
{
	bool CompletesNormally() { return false; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiDoWhile : public CiLoop
{
	virtual void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiFor : public CiLoop
{
	SymbolTable symbols;
	ICiStatement* init = NULL;
	ICiStatement* advance = NULL;
	
	virtual void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiIf : public CiCondCompletionStatement
{
	CiExpr* cond = NULL;
	ICiStatement* on_true = NULL;
	ICiStatement* on_false = NULL;
	
	virtual void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiNativeBlock : public ICiStatement
{
	String content;
	
	CiNativeBlock(String content) {this->content = content;}
	virtual bool CompletesNormally() { return true; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiReturn : public ICiStatement
{
	CiExpr* value = NULL;
	
	virtual bool CompletesNormally() { return false; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiCase
{
	Vector<Object*> values;
	Vector<ICiStatement*> body;
	bool fallthrough;
	CiExpr* fallthrough_to = NULL;
	
	CiCase(const Vector<Object*>& values) {this->values <<= values;}
};

struct CiSwitch : public CiCondCompletionStatement
{
	CiExpr* value = NULL;
	Vector<CiCase*> cases;
	Vector<ICiStatement*> default_body;
	
	virtual void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiThrow : public ICiStatement
{
	CiExpr* message = NULL;
	
	virtual bool CompletesNormally() { return false; }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiWhile : public CiLoop
{
	virtual void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiDelegate : public CiType
{
	CiType* return_type;
	Vector<CiParam*> params;
	CiWriteStatus write_status; // C only
	
	CiDelegate() {}
	CiDelegate(const CiDelegate& s) {Panic("TODO");}
	CiDelegate(String name, CiType* ret_type, CiParam* param0, CiParam* param1, CiParam* param2, CiParam* param3) {
		this->name = name;
		return_type = ret_type;
		params.Add(param0);
		params.Add(param1);
		params.Add(param2);
		params.Add(param3);
	}
	 //{ Name = name, ReturnType = returnType, Params = paramz };
	virtual CiType* Accept(ICiTypeVisitor& v) { return v.VisitType(this); }
	virtual void Accept(ICiSymbolVisitor* v) { v->VisitSymbol(this); }
};

struct CiMethodCall : public CiExpr
{
	CiExpr* obj = NULL;
	CiMethod* method = NULL;
	Vector<CiExpr*> arguments;
	
	CiMethodCall() {}
	CiMethodCall(CiMethod* m, CiExpr* o, CiExpr* arg) {obj = o; method = m; arguments.Add(arg);}
	CiDelegate* Signature();
	virtual CiType* Type() { return this->Signature()->return_type; }
	virtual bool HasSideEffect() { return true; }
	virtual bool CompletesNormally() { return true; }
	virtual CiExpr* Accept(ICiExprVisitor* v) { return v->VisitExpr(this); }
	void Accept(ICiStatementVisitor* v) { v->VisitStmt(this); }
};

struct CiMethod : public CiSymbol
{
	CiClass* class_ = NULL;
	CiCallType call_type;
	CiDelegate* signature = NULL;
	CiParam* this_ = NULL;
	ICiStatement* body = NULL;
	bool throws = false;
	Object* error_return_value = NULL;
	PtrIndex<CiMethod*> called_by;
	PtrIndex<CiMethod*> calls;
	bool is_mutator = false;
	
	CiMethod(CiType* return_type, String name, CiParam* param0 = NULL, CiParam* param1 = NULL, CiParam* param2 = NULL, CiParam* param3 = NULL)
	{
		this->name = name;
		this->call_type = NormalCallType;
		this->signature = new CiDelegate(name, return_type, param0, param1, param2, param3);
	}
	CiMethod(CiType* return_type, String name, bool is_mutat) {
		this->name = name;
		this->call_type = NormalCallType;
		this->signature = new CiDelegate(name, return_type, NULL, NULL, NULL, NULL);
		is_mutator = is_mutat;
	}
	CiMethod(CiType* return_type, String name, CiClass* cls, CiParam* this_) {
		this->name = name;
		this->call_type = NormalCallType;
		this->signature = new CiDelegate(name, return_type, NULL, NULL, NULL, NULL);
		this->class_ = cls;
		this->this_ = this_;
	}
	CiMethod(CiType* return_type, String name, CiClass* cls, CiCallType ct) {
		this->name = name;
		this->call_type = ct;
		this->signature = new CiDelegate(name, return_type, NULL, NULL, NULL, NULL);
		this->class_ = cls;
	}
	virtual void Accept(ICiSymbolVisitor* v) { v->VisitSymbol(this); }
};

struct CiClass : public CiSymbol
{
	bool is_abstract = false;
	CiClass* base_class = NULL;
	SymbolTable* members = NULL;
	CiMethod* constructor = NULL;
	Vector<CiConst*> const_arrays;
	Vector<CiBinaryResource*> binary_resources;
	bool is_resolved = false;
	String source_filename;
	CiWriteStatus write_status; // C, JS only
	bool has_fields = false; // C only
	bool constructs = false; // C only
	bool is_allocated = false; // C only
	
	CiClass() {}
	virtual void Accept(ICiSymbolVisitor* v) { v->VisitSymbol(this); }
};

struct CiUnknownClass : public CiClass {
	
	CiUnknownClass() {}
	CiUnknownClass(String name) {this->name = name;}
};

struct CiProgram
{
	SymbolTable* globals = NULL;
	
	CiProgram(SymbolTable* gl) {globals = gl;}
};


struct CiClassType : public CiType
{
	CiClass* class_ = NULL;
	
	CiClassType() {}
	CiClassType(const CiClassType& s) {Panic("TODO");}
	virtual CiSymbol* LookupMember(String name) {
		return this->class_->members->Lookup(name);
	}
	virtual CiType* Accept(ICiTypeVisitor& v) { return v.VisitType(this); }
};

struct CiClassPtrType : public CiClassType, ICiPtrType
{
	PtrWritability writability = Unknown;
	PtrIndex<ICiPtrType*> sources;
	
	CiClassPtrType() {}
	CiClassPtrType(CiClass* cls) {this->class_ = cls;}
	CiClassPtrType(String name, CiClass* cls) {this->name = name; this->class_ = cls;}
	virtual bool Equals(CiType& obj)
	{
		CiClassPtrType* that = dynamic_cast<CiClassPtrType*>(&obj);
		return that != NULL && this->class_ == that->class_;
	}
};

struct CiClassStorageType : public CiClassType
{
	CiClassStorageType() {}
	CiClassStorageType(String name, CiClass* cls) {this->name = name; class_ = cls;}
	virtual bool Equals(CiType& obj)
	{
		CiClassStorageType* that = dynamic_cast<CiClassStorageType*>(&obj);
		return that != NULL && this->class_ == that->class_;
	}
	virtual CiType* Ptr() { return new CiClassPtrType(this->class_); }
	virtual CiClass* StorageClass() { return this->class_; }
};

}
