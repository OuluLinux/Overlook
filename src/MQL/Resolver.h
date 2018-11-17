
namespace Ci
{

struct ResolveException : public Exc
{
	ResolveException(String message) : Exc(message) {}
	//public ResolveException(String format, const Vector<Object*>& args) : this(string.Format(format, args)) {}
};

struct CiResolver : public ICiSymbolVisitor, ICiTypeVisitor, ICiExprVisitor, ICiStatementVisitor
{
	Vector<String> search_dirs;
	SortedArrayMap<String, CiBinaryResource*> binary_resources;
	SymbolTable* symbols;
	Vector<ICiPtrType*> writable_ptr_types;
	Vector<CiMethod*> throwing_methods;
	CiClass* current_class = NULL;
	CiMethod* current_method = NULL;
	CiLoop* current_loop = NULL;
	CiCondCompletionStatement* current_loop_or_switch = NULL;
	Vector<CiMethod*> current_pure_methods;
	ArrayMap<CiVar, CiExpr*> current_pure_arguments;

	CiResolver()
	{
		this->writable_ptr_types.Add(CiArrayPtrType::WritableByteArray);
	}

	String FindFile(String name)
	{
		for(int i = 0; i < search_dirs.GetCount(); i++) {
			String full = AppendFileName(search_dirs[i], name);
			if (FileExists(full))
				return full;
		}
		if (FileExists(name))
			return name;
		throw ResolveException("File " + name + " not found");
	}

	CiType* VisitType(CiUnknownType* type)
	{
		CiSymbol* symbol = this->symbols->Lookup(type->name);
		CiType* t = dynamic_cast<CiType*>(symbol);
		if (t)
			return t;
		CiClass* c = dynamic_cast<CiClass*>(symbol);
		if (c)
			return new CiClassPtrType(type->name, c);
		throw ResolveException(type->name + " is not a type");
	}

	CiType* VisitType(CiStringStorageType* type)
	{
		type->length = (int) ResolveConstExpr(type->length_expr, CiIntType::Value());
		return type;
	}

	CiClass* ResolveClass(CiClass* klass)
	{
		CiUnknownClass* k = dynamic_cast<CiUnknownClass*>(klass);
		if (k) {
			String name = klass->name;
			klass = dynamic_cast<CiClass*>(this->symbols->Lookup(name));
			if (klass == NULL)
				throw ResolveException(name + " is not a class");
		}
		return klass;
	}

	CiType* VisitType(CiClassType* type)
	{
		type->class_ = ResolveClass(type->class_);
		return type;
	}

	CiType* VisitType(CiArrayType* type)
	{
		type->element_type = Resolve(type->element_type);
		return type;
	}

	CiType* VisitType(CiArrayStorageType* type)
	{
		type->element_type = Resolve(type->element_type);
		if (type->length_expr != NULL) {
			type->length = (int) ResolveConstExpr(type->length_expr, CiIntType::Value());
			type->length_expr = NULL;
		}
		return type;
	}

	CiType* Resolve(CiType* type)
	{
		return type->Accept(this);
	}

	CiCondExpr* Coerce(CiCondExpr* expr, CiType* expected)
	{
		return new CiCondExpr(
			expr->cond,
			expected,
			Coerce(expr->on_true, expected),
			Coerce(expr->on_false, expected)
		);
	}

	static bool Extends(CiType* type, CiClass* baseClass)
	{
		CiClassType* t = dynamic_cast<CiClassType*>(type);
		if (!t)
			return false;
		CiClass* klass = t->class_;
		while (klass != baseClass) {
			// TODO: resolve, make sure no loops
			klass = klass->base_class;
			if (klass == NULL)
				return false;
		}
		return true;
	}

	CiMaybeAssign* Coerce(CiMaybeAssign* expr, CiType* expected)
	{
		CiType* got = expr->type;
		if (expected->Equals(got))
			return expr;
		if (expected == CiIntType::Value() && got == CiByteType::Value()) {
			CiConstExpr* konst = dynamic_cast<CiConstExpr*>(expr);
			if (konst != NULL)
				return new CiConstExpr(konst->value); // TODO check (object) (int) (byte) konst->value cast
			CiCondExpr* cond = dynamic_cast<CiCondExpr*>(expr);
			if (cond != NULL && (dynamic_cast<CiConstExpr*>(cond->on_true) || dynamic_cast<CiConstExpr*>(cond->on_false))) {
				// avoid ((foo ? 1 : 0) & 0xff) in Java
				return Coerce(cond, expected);
			}
			CiArrayAccess* aa = dynamic_cast<CiArrayAccess*>(expr);
			if (aa) {
				CiConstAccess* ca = dynamic_cast<CiConstAccess*>(aa->array);
				if (ca != NULL && ca->const_->is_7bit)
					return expr;
			}
			return new CiCoercion(expected, expr);
		}
		if (expected == CiByteType::Value() && got == CiIntType::Value()) {
			CiConstExpr* konst = dynamic_cast<CiConstExpr*>(expr);
			if (konst != NULL)
				return new CiConstExpr(konst->value); // TODO check (object) (int) (byte) konst->value cast
			return new CiCoercion(expected, expr);
		}
		if (expected == CiStringPtrType::Value() && (got == CiType::Null || dynamic_cast<CiStringType*>(got)))
			return expr;
		if (dynamic_cast<CiStringStorageType*>(expected) && dynamic_cast<CiStringType*>(got))
			return expr;
		CiClassType* ct = dynamic_cast<CiClassType*>(expected);
		if (ct) {
			if (got == CiType::Null)
				return expr;
			if (Extends(got, (ct->class_))) {
				CiCondExpr* ce = dynamic_cast<CiCondExpr*>(expr);
				if (ce) {
					// C doesn't like &(cond ? foo : bar)
					return Coerce(ce, expected);
				}
				return new CiCoercion(expected, expr);
			}
		}
		CiArrayPtrType* apt = dynamic_cast<CiArrayPtrType*>(expected);
		if (apt) {
			if (got == CiType::Null)
				return expr;
			CiArrayType* gotArray = dynamic_cast<CiArrayType*>(got);
			if (got != NULL && (apt->element_type->Equals(gotArray->element_type)))
				return new CiCoercion(expected, expr);
		}
		throw ResolveException("Expected " + expected->name + ", got " + got->name);
	}

	CiExpr* Coerce(CiExpr* expr, CiType* expected)
	{
		return Coerce(expr, expected);
	}

	Object* ResolveConstExpr(CiExpr* expr, CiType* type)
	{
		expr = Coerce(Resolve(expr), type);
		CiConstExpr* ce = dynamic_cast<CiConstExpr*>(expr);
		if (ce == NULL)
			throw ResolveException(expr->ToString() + " is not constant");
		return ce->value;
	}

	Object* ResolveConstInitializer(CiType* type, Object* value)
	{
		CiArrayType* at = dynamic_cast<CiArrayType*>(type);
		if (at) {
			if (value->objs.IsEmpty())
				return value;
			CiType* elementType = at->element_type;
			CiArrayStorageType* ast = dynamic_cast<CiArrayStorageType*>(type);
			if (ast) {
				int expected = ast->length;
				if (value->objs.GetCount() != expected)
					throw ResolveException("Expected " + IntStr(expected) + " array elements, got " + IntStr(value->objs.GetCount()));
			}
			else {
				type = new CiArrayStorageType(elementType, value->objs.GetCount());
			}
			Object* dest = new Object();
			dest->objs <<= value->objs;
			for (int i = 0; i < value->objs.GetCount(); i++)
				dest->objs.Add(ResolveConstInitializer(elementType, value->objs[i]));
			return dest;
		}
		CiExpr* e = dynamic_cast<CiExpr*>(value);
		if (e)
			return ResolveConstExpr(e, type);
		return value;
	}

	void VisitSymbol(CiEnum* enu) {
		
	}

	static bool Is7Bit(const Vector<byte>& bytes)
	{
		for(int i = 0; i < bytes.GetCount(); i++) {
			byte b = bytes[i];
			if ((b & ~0x7f) != 0)
				return false;
		}
		return true;
	}

	void VisitSymbol(CiConst* konst)
	{
		if (konst->currently_resolving)
			throw ResolveException("Circular dependency for " + konst->name);
		konst->currently_resolving = true;
		konst->type = Resolve(konst->type);
		konst->value = ResolveConstInitializer(konst->type, konst->value);
		if (konst->value->bytes.GetCount())
			konst->is_7bit = Is7Bit(konst->value->bytes);
		konst->currently_resolving = false;
	}

	static String GetConstString(CiExpr* expr)
	{
		Object* o = dynamic_cast<CiConstExpr*>(expr)->value;
		if (o->type == O_STRING || o->type == O_INT || o->type == O_BYTE)
			return o->ToString();
		throw ResolveException("Cannot convert " + expr->type->name + " to string");
	}

	static int GetConstInt(CiExpr* expr)
	{
		return dynamic_cast<CiConstExpr*>(expr)->value->GetInt();
	}

	void MarkWritable(CiExpr* target)
	{
		for (;;) {
			CiFieldAccess* fa = dynamic_cast<CiFieldAccess*>(target);
			CiArrayAccess* aa = dynamic_cast<CiArrayAccess*>(target);
			if (fa)
				target = fa->obj;
			else if (aa)
				target = aa->array;
			else
				break;
			CiCoercion* c = dynamic_cast<CiCoercion*>(target);
			while (c) {
				target = dynamic_cast<CiExpr*>(c->inner);
				c = dynamic_cast<CiCoercion*>(target);
			}
			ICiPtrType* pt = dynamic_cast<ICiPtrType*>(target->type);
			if (pt != NULL) {
				this->writable_ptr_types.Add(pt);
				break;
			}
		}
	}

	static void CheckCopyPtr(CiType* target, CiMaybeAssign* source)
	{
		ICiPtrType* tp = dynamic_cast<ICiPtrType*>(target);
		if (tp == NULL)
			return;
		CiCondExpr* cond = dynamic_cast<CiCondExpr*>(source);
		if (cond != NULL) {
			CheckCopyPtr(target, cond->on_true);
			CheckCopyPtr(target, cond->on_false);
			return;
		}
		for (;;) {
			CiCoercion* c = dynamic_cast<CiCoercion*>(source);
			while (c) {
				source = c->inner;
				c = dynamic_cast<CiCoercion*>(source);
			}
			ICiPtrType* sp = dynamic_cast<ICiPtrType*>(source->type);
			if (sp != NULL) {
				tp->sources.Add(sp);
				break;
			}
			CiFieldAccess* fa = dynamic_cast<CiFieldAccess*>(source);
			CiArrayAccess* aa = dynamic_cast<CiArrayAccess*>(source);
			if (fa)
				source = fa->obj;
			else if (aa)
				source = aa->array;
			else
				break;
		}
	}

	CiLValue* ResolveLValue(CiExpr* expr)
	{
		CiLValue* result = dynamic_cast<CiLValue*>(Resolve(expr));
		if (result == NULL)
			throw ResolveException("Expected l-value");
		MarkWritable(result);
		return result;
	}

	CiSymbol* Lookup(CiSymbolAccess* expr)
	{
		CiSymbol* symbol = expr->symbol;
		CiUnknownSymbol* us = dynamic_cast<CiUnknownSymbol*>(symbol);
		if (us)
			symbol = this->symbols->Lookup(us->name);
		return symbol;
	}

	CiExpr* GetValue(CiConst* konst)
	{
		VisitSymbol(konst);
		CiArrayType* at = dynamic_cast<CiArrayType*>(konst->type);
		if (at)
			return new CiConstAccess(konst);
		else
			return new CiConstExpr(konst->value);
	}

	CiFieldAccess* CreateFieldAccess(CiExpr* obj, CiField* field)
	{
		if (field->class_ != this->current_class && field->visibility == Private)
			field->visibility = InternalVisib;
		CiClassPtrType* cpt = dynamic_cast<CiClassPtrType*>(obj->type);
		if (!cpt || (cpt->class_ != field->class_))
			obj = Coerce(obj, new CiClassStorageType("", field->class_));
		return new CiFieldAccess(obj, field);
	}

	CiExpr* VisitExpr(CiSymbolAccess* expr)
	{
		CiSymbol* symbol = Lookup(expr);
		
		CiVar* v = dynamic_cast<CiVar*>(symbol);
		if (v) {
			int i = current_pure_arguments.Find(*v);
			if (i >= 0)
				return current_pure_arguments[i];
			return new CiVarAccess(v);
		}
		
		CiConst* c = dynamic_cast<CiConst*>(symbol);
		if (c)
			return GetValue(c);
		
		CiField* f = dynamic_cast<CiField*>(symbol);
		if (f) {
			if (this->current_method->call_type == StaticCallType)
				throw ResolveException("Cannot access field from a static method");
			symbol->Accept(this);
			return CreateFieldAccess(new CiVarAccess(this->current_method->this_), f);
		}
		
		throw ResolveException("Invalid expression");
	}

	CiExpr* VisitExpr(CiVarAccess* expr)
	{
		CiExpr* arg;
		int i = current_pure_arguments.Find(*expr->var);
		if (i >= 0)
			return current_pure_arguments[i];
		return expr;
	}

	CiExpr* VisitExpr(CiPropertyAccess* expr)
	{
		CiConstExpr* konst = dynamic_cast<CiConstExpr*>(Resolve(expr->obj));
		if (konst) {
			if (expr->property == CiLibrary::LowByteProperty)
				return new CiConstExpr(konst->value->GetUnsignedByte()); // TODO: (byte) (int)  cast
			if (expr->property == CiLibrary::SByteProperty)
				return new CiConstExpr(konst->value->GetChar());
			if (expr->property == CiLibrary::StringLengthProperty)
				return new CiConstExpr(konst->value->s.GetCount());
		}
		return expr;
	}

	CiExpr* VisitExpr(CiUnknownMemberAccess* expr)
	{
		CiSymbolAccess* sa = dynamic_cast<CiSymbolAccess*>(expr->parent);
		if (sa) {
			CiSymbol* symbol = Lookup(sa);
			
			CiEnum* e = dynamic_cast<CiEnum*>(symbol);
			if (e)
				return new CiConstExpr(e->LookupMember(expr->name));
			
			CiClass* c = dynamic_cast<CiClass*>(symbol);
			if (c) {
				symbol = c->members->Lookup(expr->name);
				
				CiConst* co = dynamic_cast<CiConst*>(symbol);
				if (co)
					return GetValue(co);
				
				throw ResolveException("Cannot access " + expr->name);
			}
		}
		CiExpr* parent = Resolve(expr->parent);
		CiSymbol* member = parent->type->LookupMember(expr->name);
		member->Accept(this);
		
		CiField* f = dynamic_cast<CiField*>(member);
		if (f)
			return CreateFieldAccess(parent, f);
		
		CiProperty* prop = dynamic_cast<CiProperty*>(member);
		if (prop != NULL)
			return (new CiPropertyAccess(parent, prop))->Accept(this); //TODO ref new
		
		CiConst* c = dynamic_cast<CiConst*>(member);
		if (c)
			return new CiConstExpr(c->value);
		throw ResolveException(member->ToString());
	}

	CiExpr* VisitExpr(CiIndexAccess* expr)
	{
		CiExpr* parent = Resolve(expr->parent);
		CiExpr* index = Coerce(Resolve(expr->index), CiIntType::Value());
		CiArrayType* at = dynamic_cast<CiArrayType*>(parent->type);
		if (at)
			return new CiArrayAccess(parent, index);
		CiStringType* st = dynamic_cast<CiStringType*>(parent->type);
		if (st) {
			CiConstExpr* pce = dynamic_cast<CiConstExpr*>(parent);
			CiConstExpr* ice = dynamic_cast<CiConstExpr*>(index);
			if (pce && ice) {
				String s = pce->value->ToString();
				int i = GetConstInt(index);
				if (i >= 0 && i < s.GetCount())
					return new CiConstExpr(s[i]);
			}
			return new CiMethodCall(CiLibrary::CharAtMethod, parent, index);
		}
		throw ResolveException("Indexed object is neither array or string");
	}

	void VisitSymbol(CiDelegate* del)
	{
		del->return_type = Resolve(del->return_type);
		for(int i = 0; i < del->params.GetCount(); i++) {
			CiParam* param = del->params[i];
			param->type = Resolve(param->type);
		}
	}

	CiType* VisitType(CiDelegate* del)
	{
		VisitSymbol(del);
		return del;
	}

	void ResolveObj(CiMethodCall* expr)
	{
		if (expr->method != NULL)
			return; // already resolved
		CiSymbolAccess* sa = dynamic_cast<CiSymbolAccess*>(expr->obj);
		if (sa) {
			// Foo(...)
			CiMethod method = Lookup((CiSymbolAccess) expr.Obj) as CiMethod;
			if (method != NULL) {
				expr.Method = method;
				if (method.CallType == CiCallType.Static)
					expr.Obj = NULL;
				else {
					if (this->current_method.CallType == CiCallType.Static)
						throw ResolveException("Cannot call instance method from a static method");
					expr.Obj = Coerce(new CiVarAccess { Var = this->current_method.This }, new CiClassPtrType { Class = method.Class });
					CheckCopyPtr(method.This.Type, expr.Obj);
				}
				return;
			}
		}
		else if (expr.Obj is CiUnknownMemberAccess) {
			// ???.Foo(...)
			CiUnknownMemberAccess uma = (CiUnknownMemberAccess) expr.Obj;
			if (uma.Parent is CiSymbolAccess) {
				CiClass klass = Lookup((CiSymbolAccess) uma.Parent) as CiClass;
				if (klass != NULL) {
					// Class.Foo(...)
					CiMethod method = klass.Members.Lookup(uma.Name) as CiMethod;
					if (method != NULL) {
						if (method.CallType != CiCallType.Static)
							throw ResolveException("{0} is a non-static method", method.Name);
						expr.Method = method;
						expr.Obj = NULL;
						return;
					}
				}
			}
			CiExpr obj = Resolve(uma.Parent);
			{
				CiMethod method = obj.Type.LookupMember(uma.Name) as CiMethod;
				if (method != NULL) {
					// obj.Foo(...)
					if (method.CallType == CiCallType.Static)
						throw ResolveException("{0} is a static method", method.Name);
					if (method.This != NULL) {
						// user-defined method
						CheckCopyPtr(method.This.Type, obj);
						obj = Coerce(obj, new CiClassPtrType { Class = method.Class });
					}
					expr.Method = method;
					expr.Obj = obj;
					return;
				}
			}
		}
		expr.Obj = Resolve(expr.Obj);
		if (!(expr.Obj.Type is CiDelegate))
			throw ResolveException("Invalid call");
		if (expr.Obj.HasSideEffect)
			throw ResolveException("Side effects not allowed in delegate call");
	}

	void CoerceArguments(CiMethodCall* expr)
	{
		expr.Signature.Accept(this);
		CiParam[] paramz = expr.Signature.Params;
		if (expr.Arguments.Length != paramz.Length)
			throw ResolveException("Invalid number of arguments for {0}, expected {1}, got {2}", expr.Signature.Name, paramz.Length, expr.Arguments.Length);
		for (int i = 0; i < paramz.Length; i++) {
			CiExpr arg = Resolve(expr.Arguments[i]);
			CheckCopyPtr(paramz[i].Type, arg);
			expr.Arguments[i] = Coerce(arg, paramz[i].Type);
		}
	}

	CiExpr* VisitExpr(CiMethodCall* expr)
	{
		ResolveObj(expr);
		CoerceArguments(expr);
		if (expr.Method != NULL && expr.Method != this->current_method) {
			CiReturn ret = expr.Method.Body as CiReturn;
			if (ret != NULL
			 && expr.Method.CallType == CiCallType.Static
			 && expr.Arguments.All(arg => arg is CiConstExpr)
			 && current_pure_methods.Add(expr.Method)) {
				CiParam[] paramz = expr.Signature.Params;
				for (int i = 0; i < paramz.Length; i++)
					current_pure_arguments.Add(paramz[i], expr.Arguments[i]);
				CiConstExpr constFold = Resolve(ret.Value) as CiConstExpr;
				foreach (CiParam param in paramz)
					current_pure_arguments.Remove(param);
				current_pure_methods.Remove(expr.Method);
				if (constFold != NULL)
					return constFold;
			}
			if (expr.Method == CiLibrary::CharAtMethod
			 && expr.Arguments[0] is CiConstExpr) {
				CiConstExpr stringExpr = Resolve(expr.Obj) as CiConstExpr;
				if (stringExpr != NULL) {
					string s = (string) stringExpr.Value;
					int i = GetConstInt(expr.Arguments[0]);
					if (i < s.Length)
						return new CiConstExpr((int) s[i]);
				}
			}
			if (expr.Method.IsMutator)
				MarkWritable(expr.Obj);
			expr.Method.CalledBy.Add(this->current_method);
			this->current_method.Calls.Add(expr.Method);
		}
		return expr;
	}

	CiExpr* VisitExpr(CiUnaryExpr* expr)
	{
		CiExpr resolved;
		if (expr.Op == Increment || expr.Op == Decrement)
			resolved = ResolveLValue(expr.Inner);
		else
			resolved = Resolve(expr.Inner);
		CiExpr inner = Coerce(resolved, CiIntType::Value());
		if (expr.Op == Minus && inner is CiConstExpr)
			return new CiConstExpr(-GetConstInt(inner));
		expr.Inner = inner;
		return expr;
	}

	CiExpr* VisitExpr(CiCondNotExpr* expr)
	{
		expr.Inner = Coerce(Resolve(expr.Inner), CiBoolType.Value);
		return expr;
	}

	CiExpr* VisitExpr(CiPostfixExpr* expr)
	{
		expr.Inner = Coerce(ResolveLValue(expr.Inner), CiIntType::Value());
		return expr;
	}

	CiExpr* VisitExpr(CiBinaryExpr* expr)
	{
		CiExpr left = Resolve(expr.Left);
		CiExpr right = Resolve(expr.Right);
		if (expr.Op == Plus && (left.Type is CiStringType || right.Type is CiStringType)) {
			if (!(left is CiConstExpr && right is CiConstExpr))
				throw ResolveException("String concatenation allowed only for constants. Consider using +=");
			string a = GetConstString(left);
			string b = GetConstString(right);
			return new CiConstExpr(a + b);
		}
		left = Coerce(left, CiIntType::Value());
		right = Coerce(right, CiIntType::Value());
		if (right is CiConstExpr) {
			int b = GetConstInt(right);
			if (left is CiConstExpr) {
				int a = GetConstInt(left);
				switch (expr.Op) {
				case Asterisk: a *= b; break;
				case Slash: a /= b; break;
				case Mod: a %= b; break;
				case And: a &= b; break;
				case ShiftLeft: a <<= b; break;
				case ShiftRight: a >>= b; break;
				case Plus: a += b; break;
				case Minus: a -= b; break;
				case Or: a |= b; break;
				case Xor: a ^= b; break;
				}
				return new CiConstExpr(a);
			}
			if (expr.Op == And && (b & ~0xff) == 0) {
				CiCoercion c = left as CiCoercion;
				if (c != NULL && c.Inner.Type == CiByteType::Value())
					left = (CiExpr) c.Inner;
			}
		}
		// expr.Left = left;
		// expr.Right = right;
		// return expr;
		return new CiBinaryExpr(left, expr->op, right);
	}

	static CiType* FindCommonType(CiExpr* expr1, CiExpr* expr2)
	{
		CiType type1 = expr1.Type;
		CiType type2 = expr2.Type;
		if (type1.Equals(type2))
			return type1;
		if ((type1 == CiIntType::Value() && type2 == CiByteType::Value())
			|| (type1 == CiByteType::Value() && type2 == CiIntType::Value()))
			return CiIntType::Value();
		CiType type = type1.Ptr;
		if (type != NULL)
			return type; // stg, ptr || stg, NULL
		type = type2.Ptr;
		if (type != NULL)
			return type; // ptr, stg || NULL, stg
		if (type1 != CiType.Null)
			return type1; // ptr, NULL
		if (type2 != CiType.Null)
			return type2; // NULL, ptr
		throw ResolveException("Incompatible types");
	}

	CiExpr* VisitExpr(CiBoolBinaryExpr* expr)
	{
		CiExpr left = Resolve(expr.Left);
		CiExpr right = Resolve(expr.Right);
		CiType type;
		switch (expr.Op) {
		case CondAnd:
		case CondOr:
			type = CiBoolType.Value;
			break;
		case Equal:
		case NotEqual:
			type = FindCommonType(left, right);
			break;
		default:
			type = CiIntType::Value();
			break;
		}
		left = Coerce(left, type);
		right = Coerce(right, type);
		CiConstExpr cleft = left as CiConstExpr;
		if (cleft != NULL) {
			switch (expr.Op) {
			case CondAnd:
				return (bool) cleft.Value ? right : new CiConstExpr(false);
			case CondOr:
				return (bool) cleft.Value ? new CiConstExpr(true) : right;
			case Equal:
			case NotEqual:
				CiConstExpr cright = right as CiConstExpr;
				if (cright != NULL) {
					bool eq = object.Equals(cleft.Value, cright.Value);
					return new CiConstExpr(expr.Op == Equal ? eq : !eq);
				}
				break;
			default:
				if (right is CiConstExpr) {
					int a = GetConstInt(cleft);
					int b = GetConstInt(right);
					bool result;
					switch (expr.Op) {
					case Less: result = a < b; break;
					case LessOrEqual: result = a <= b; break;
					case Greater: result = a > b; break;
					case GreaterOrEqual: result = a >= b; break;
					default:
						expr.Left = left;
						expr.Right = right;
						return expr;
					}
					return new CiConstExpr(result);
				}
				break;
			}
		}
		expr.Left = left;
		expr.Right = right;
		return expr;
	}

	CiExpr* VisitExpr(CiCondExpr* expr)
	{
		CiExpr cond = Coerce(Resolve(expr.Cond), CiBoolType.Value);
		CiExpr expr1 = Resolve(expr.OnTrue);
		CiExpr expr2 = Resolve(expr.OnFalse);
		CiType type = FindCommonType(expr1, expr2);
		expr1 = Coerce(expr1, type);
		expr2 = Coerce(expr2, type);
		CiConstExpr konst = cond as CiConstExpr;
		if (konst != NULL)
			return (bool) konst.Value ? expr1 : expr2;
		// expr.Cond = cond;
		// expr.OnTrue = expr1;
		// expr.OnFalse = expr2;
		// return expr;
		return new CiCondExpr {
			Cond = cond,
			ResultType = type,
			OnTrue = expr1,
			OnFalse = expr2
		};
	}

	CiExpr* VisitExpr(CiBinaryResourceExpr* expr)
	{
		string name = (string) ResolveConstExpr(expr->nameExpr, CiStringPtrType.Value);
		CiBinaryResource resource;
		if (!this->binary_resources.TryGetValue(name, out resource)) {
			resource = new CiBinaryResource();
			resource.Name = name;
			resource.Content = File.ReadAllBytes(FindFile(name));
			resource.Type = new CiArrayStorageType { element_type = CiByteType::Value(), Length = resource.Content.Length };
			this->binary_resources.Add(name, resource);
		}
		expr.Resource = resource;
		return expr;
	}

	void CheckCreatable(CiType* type)
	{
		CiClass storageClass = type.StorageClass;
		if (storageClass != NULL && storageClass.IsAbstract)
			throw ResolveException("Cannot create instances of an abstract class {0}", storageClass.Name);
	}

	CiExpr* VisitExpr(CiNewExpr* expr)
	{
		CiType type = expr.NewType;
		CiClassStorageType classStorageType = type as CiClassStorageType;
		if (classStorageType != NULL) {
			classStorageType.Class = ResolveClass(classStorageType.Class);
			classStorageType.Class.IsAllocated = true;
		}
		else {
			CiArrayStorageType arrayStorageType = (CiArrayStorageType) type;
			arrayStorageType.element_type = Resolve(arrayStorageType.element_type);
			arrayStorageType.LengthExpr = Coerce(Resolve(arrayStorageType.LengthExpr), CiIntType::Value());
		}
		CheckCreatable(type);
		return expr;
	}

	CiExpr* Resolve(CiExpr* expr)
	{
		return expr.Accept(this);
	}

	void VisitSymbol(CiField* field)
	{
		field.Type = Resolve(field.Type);
		CheckCreatable(field.Type);
	}

	bool Resolve(Vector<ICiStatement*>& statements)
	{
		bool reachable = true;
		foreach (ICiStatement child in statements) {
			if (!reachable)
				throw ResolveException("Unreachable statement");
			child.Accept(this);
			reachable = child.CompletesNormally;
		}
		return reachable;
	}

	void VisitStmt(CiBlock* statement) {
		statement.CompletesNormally = Resolve(statement.Statements);
	}

	void VisitStmt(CiConst* statement) {
		
	}

	void VisitStmt(CiVar* statement)
	{
		CiType type = Resolve(statement.Type);
		statement.Type = type;
		CheckCreatable(type);
		if (statement.InitialValue != NULL) {
			CiExpr initialValue = Resolve(statement.InitialValue);
			CheckCopyPtr(type, initialValue);
			if (type is CiArrayStorageType) {
				type = ((CiArrayStorageType) type).element_type;
				CiConstExpr ce = Coerce(initialValue, type) as CiConstExpr;
				if (ce == NULL)
					throw ResolveException("Array initializer is not constant");
				statement.InitialValue = ce;
				if (type == CiBoolType.Value) {
					if (!false.Equals(ce.Value))
						throw ResolveException("Bool arrays can only be initialized with false");
				}
				else if (type == CiByteType::Value()) {
					if (!((byte) 0).Equals(ce.Value))
						throw ResolveException("Byte arrays can only be initialized with zero");
				}
				else if (type == CiIntType::Value()) {
					if (!0.Equals(ce.Value))
						throw ResolveException("Int arrays can only be initialized with zero");
				}
				else
					throw ResolveException("Invalid array initializer");
			}
			else
				statement.InitialValue = Coerce(initialValue, type);
		}
	}

	void VisitStmt(CiExpr* statement)
	{
		Resolve((CiExpr) statement);
	}

	void VisitStmt(CiAssign* statement)
	{
		statement.Target = ResolveLValue(statement.Target);
		if (statement.Target is CiVarAccess && ((CiVarAccess) statement.Target).Var == this->current_method.This)
			throw ResolveException("Cannot assign to this");
		CiMaybeAssign source = statement.Source;
		if (source is CiAssign)
			Resolve((ICiStatement) source);
		else
			source = Resolve((CiExpr) source);
		CiType type = statement.Target.Type;
		CheckCopyPtr(type, source);
		statement.Source = Coerce(source, type);
		if (statement.Op != Assign && type != CiIntType::Value() && type != CiByteType::Value()) {
			if (statement.Op == AddAssign && type is CiStringStorageType && statement.Source.Type is CiStringType)
				{} // OK
			else
				throw ResolveException("Invalid compound assignment");
		}
	}

	void VisitStmt(CiDelete* statement)
	{
		statement.Expr = Resolve(statement.Expr);
		ICiPtrType type = statement.Expr.Type as ICiPtrType;
		if (type == NULL)
			throw ResolveException("'delete' takes a class or array pointer");
		if (statement.Expr.HasSideEffect)
			throw ResolveException("Side effects not allowed in 'delete'");
		this->writable_ptr_types.Add(type);
	}

	void VisitStmt(CiBreak* statement)
	{
		if (this->current_loop_or_switch == NULL)
			throw ResolveException("break outside loop and switch");
		this->current_loop_or_switch.CompletesNormally = true;
	}

	void VisitStmt(CiContinue* statement)
	{
		if (this->current_loop == NULL)
			throw ResolveException("continue outside loop");
	}

	void ResolveLoop(CiLoop* statement)
	{
		statement.CompletesNormally = false;
		if (statement.Cond != NULL) {
			statement.Cond = Coerce(Resolve(statement.Cond), CiBoolType.Value);
			statement.CompletesNormally = !statement.Cond.IsConst(false);
		}
		CiLoop oldLoop = this->current_loop;
		CiCondCompletionStatement oldLoopOrSwitch = this->current_loop_or_switch;
		this->current_loop_or_switch = this->current_loop = statement;
		Resolve(statement.Body);
		this->current_loop = oldLoop;
		this->current_loop_or_switch = oldLoopOrSwitch;
	}

	void VisitStmt(CiDoWhile* statement)
	{
		ResolveLoop(statement);
	}

	void VisitStmt(CiFor* statement)
	{
		if (statement.Init != NULL)
			Resolve(statement.Init);
		if (statement.Advance != NULL)
			Resolve(statement.Advance);
		ResolveLoop(statement);
	}

	void VisitStmt(CiIf* statement)
	{
		statement.Cond = Coerce(Resolve(statement.Cond), CiBoolType.Value);
		Resolve(statement.OnTrue);
		if (statement.OnFalse != NULL) {
			Resolve(statement.OnFalse);
			statement.CompletesNormally = statement.OnTrue.CompletesNormally || statement.OnFalse.CompletesNormally;
		}
		else
			statement.CompletesNormally = true;
	}

	void VisitStmt(CiNativeBlock* statement) {
		
	}

	void VisitStmt(CiReturn* statement)
	{
		CiType type = this->current_method.Signature.ReturnType;
		if (type != CiType.Void)
			statement.Value = Coerce(Resolve(statement.Value), type);
	}

	void VisitStmt(CiSwitch* statement)
	{
		statement.Value = Resolve(statement.Value);
		CiType type = statement.Value.Type;
		CiCondCompletionStatement oldLoopOrSwitch = this->current_loop_or_switch;
		this->current_loop_or_switch = statement;

		HashSet<object> values = new HashSet<object>();
		CiCase fallthroughFrom = NULL;
		foreach (CiCase kase in statement.Cases) {
			for (int i = 0; i < kase.Values.Length; i++) {
				kase.Values[i] = ResolveConstExpr((CiExpr) kase.Values[i], type);
				if (!values.Add(kase.Values[i]))
					throw ResolveException("Duplicate case value");
			}
			if (fallthroughFrom != NULL) {
				if (fallthroughFrom.FallthroughTo == NULL)
					throw ResolveException("goto default followed by case");
				if (!ResolveConstExpr(fallthroughFrom.FallthroughTo, type).Equals(kase.Values[0]))
					throw ResolveException("goto case doesn't match the next case");
			}
			bool reachable = Resolve(kase.Body);
			if (kase.Fallthrough) {
				if (!reachable)
					throw ResolveException("goto is not reachable");
				fallthroughFrom = kase;
			}
			else {
				if (reachable)
					throw ResolveException("case must end with break, return, throw or goto");
				fallthroughFrom = NULL;
			}
		}

		if (statement.DefaultBody != NULL) {
			if (fallthroughFrom != NULL && fallthroughFrom.FallthroughTo != NULL)
				throw ResolveException("goto case followed by default");
			bool reachable = Resolve(statement.DefaultBody);
			if (reachable)
				throw ResolveException("default must end with break, return, throw or goto");
		}
		else {
			if (fallthroughFrom != NULL)
				throw ResolveException("goto cannot be the last statement in switch");
		}

		this->current_loop_or_switch = oldLoopOrSwitch;
	}

	void VisitStmt(CiThrow* statement)
	{
		statement.Message = Coerce(Resolve(statement.Message), CiStringPtrType.Value);
		this->throwing_methods.Add(this->current_method);
	}

	void VisitStmt(CiWhile* statement)
	{
		ResolveLoop(statement);
	}

	void Resolve(ICiStatement* statement)
	{
		statement.Accept(this);
	}

	void VisitSymbol(CiMethod* method)
	{
		this->current_method = method;
		Resolve(method.Signature);
		if (method.CallType != CiCallType.Abstract) {
			Resolve(method.Body);
			if (method.Signature.ReturnType != CiType.Void && method.Body.CompletesNormally)
				throw ResolveException("Method can complete without a return value");
		}
		this->current_method = NULL;
	}

	void ResolveBase(CiClass* klass)
	{
		if (klass.BaseClass != NULL) {
			klass.BaseClass = ResolveClass(klass.BaseClass);
			klass.Members.Parent = klass.BaseClass.Members;
		}
	}

	void VisitSymbol(CiClass* klass)
	{
		this->current_class = klass;
		this->Symbols = klass.Members;
		if (klass.Constructor != NULL)
			klass.Constructor.Accept(this);
		foreach (CiSymbol member in klass.Members)
			member.Accept(this);
		klass.binary_resources = this->binary_resources.Values.ToArray();
		this->binary_resources.Clear();
		this->Symbols = this->Symbols.Parent;
		this->current_class = NULL;
	}

	static void MarkWritable(ICiPtrType* type)
	{
		if (type.Writability == PtrWritability.ReadWrite)
			return;
		if (type.Writability == PtrWritability.ReadOnly)
			throw ResolveException("Attempt to write a read-only array");
		type.Writability = PtrWritability.ReadWrite;
		foreach (ICiPtrType source in type.Sources)
			MarkWritable(source);
	}

	static Object* GetErrorValue(CiType* type)
	{
		if (type == CiType.Void)
			return false;
		if (type == CiIntType::Value())
			return -1;
		if (type == CiStringPtrType.Value || type is CiClassPtrType || type is CiArrayPtrType)
			return NULL;
		throw ResolveException("throw in a method of unsupported return type");
	}

	static void MarkThrows(CiMethod* method)
	{
		if (method.Throws)
			return;
		method.Throws = true;
		method.ErrorReturnValue = GetErrorValue(method.Signature.ReturnType);
		foreach (CiMethod calledBy in method.CalledBy)
			MarkThrows(calledBy);
	}

	static void MarkDead(CiMethod* method)
	{
		if (method.Visibility == CiVisibility.Private && method.CallType != CiCallType.Override && method.CalledBy.Count == 0) {
			method.Visibility = CiVisibility.Dead;
			foreach (CiMethod called in method.Calls) {
				called.CalledBy.Remove(method);
				MarkDead(called);
			}
		}
	}

	static void MarkDead(CiClass* klass)
	{
		foreach (CiSymbol member in klass.Members) {
			if (member is CiMethod)
				MarkDead((CiMethod) member);
		}
	}

	static void MarkInternal(CiMethod* method)
	{
		if (method.Visibility == CiVisibility.Private && method.CalledBy.Any(caller => caller.Class != method.Class))
			method.Visibility = CiVisibility.Internal;
	}

	static void MarkInternal(CiClass* klass)
	{
		foreach (CiSymbol member in klass.Members) {
			if (member is CiMethod)
				MarkInternal((CiMethod) member);
		}
	}

	void Resolve(CiProgram* program)
	{
		this->Symbols = program.Globals;
		foreach (CiSymbol symbol in program.Globals) {
			if (symbol is CiClass)
				ResolveBase((CiClass) symbol);
		}
		foreach (CiSymbol symbol in program.Globals)
			symbol.Accept(this);
		foreach (ICiPtrType type in this->writable_ptr_types)
			MarkWritable(type);
		foreach (CiMethod method in this->throwing_methods)
			MarkThrows(method);
		foreach (CiSymbol symbol in program.Globals) {
			if (symbol is CiClass)
				MarkDead((CiClass) symbol);
		}
		foreach (CiSymbol symbol in program.Globals) {
			if (symbol is CiClass)
				MarkInternal((CiClass) symbol);
		}
	}
};

}

