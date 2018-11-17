
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
	PtrIndex<CiMethod*> current_pure_methods;
	PtrMap<CiVar*, CiExpr*> current_pure_arguments;

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
		type->length = ResolveConstExpr(type->length_expr, CiIntType::Value())->GetInt();
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
			type->length = ResolveConstExpr(type->length_expr, CiIntType::Value())->GetInt();
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
			int i = current_pure_arguments.Find(v);
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
		int i = current_pure_arguments.Find(expr->var);
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
		CiUnknownMemberAccess* uma = dynamic_cast<CiUnknownMemberAccess*>(expr->obj);
		if (sa) {
			// Foo(...)
			CiSymbolAccess* sa = dynamic_cast<CiSymbolAccess*>(expr->obj);
			CiMethod* method = dynamic_cast<CiMethod*>(Lookup(sa));
			if (method) {
				expr->method = method;
				if (method->call_type == StaticCallType)
					expr->obj = NULL;
				else {
					if (this->current_method->call_type == StaticCallType)
						throw ResolveException("Cannot call instance method from a static method");
					expr->obj = Coerce(new CiVarAccess(this->current_method->this_), new CiClassPtrType(method->class_));
					CheckCopyPtr(method->this_->type, expr->obj);
				}
				return;
			}
		}
		else if (uma) {
			// ???.Foo(...)
			CiSymbolAccess* sa = dynamic_cast<CiSymbolAccess*>(uma->parent);
			if (sa) {
				CiClass* klass = dynamic_cast<CiClass*>(Lookup(sa));
				if (klass) {
					// Class.Foo(...)
					CiMethod* method = dynamic_cast<CiMethod*>(klass->members->Lookup(uma->name));
					if (method) {
						if (method->call_type != StaticCallType)
							throw ResolveException(method->name + " is a non-static method");
						expr->method = method;
						expr->obj = NULL;
						return;
					}
				}
			}
			CiExpr* obj = Resolve(uma->parent);
			{
				CiMethod* method = dynamic_cast<CiMethod*>(obj->type->LookupMember(uma->name));
				if (method) {
					// obj.Foo(...)
					if (method->call_type == StaticCallType)
						throw ResolveException(method->name + " is a static method");
					if (method->this_ != NULL) {
						// user-defined method
						CheckCopyPtr(method->this_->type, obj);
						obj = Coerce(obj, new CiClassPtrType(method->class_ ));
					}
					expr->method = method;
					expr->obj = obj;
					return;
				}
			}
		}
		expr->obj = Resolve(expr->obj);
		if (!dynamic_cast<CiDelegate*>(expr->obj->type))
			throw ResolveException("Invalid call");
		if (expr->obj->HasSideEffect())
			throw ResolveException("Side effects not allowed in delegate call");
	}

	void CoerceArguments(CiMethodCall* expr)
	{
		expr->Signature()->Accept(this);
		auto& paramz = expr->Signature()->params;
		if (expr->arguments.GetCount() != paramz.GetCount())
			throw ResolveException("Invalid number of arguments for " + expr->Signature()->name + ", expected " + IntStr(paramz.GetCount()) + ", got " + IntStr(expr->arguments.GetCount()));
		for (int i = 0; i < paramz.GetCount(); i++) {
			CiExpr* arg = Resolve(expr->arguments[i]);
			CheckCopyPtr(paramz[i]->type, arg);
			expr->arguments[i] = Coerce(arg, paramz[i]->type);
		}
	}

	CiExpr* VisitExpr(CiMethodCall* expr)
	{
		ResolveObj(expr);
		CoerceArguments(expr);
		if (expr->method != NULL && expr->method != this->current_method) {
			CiReturn* ret = dynamic_cast<CiReturn*>(expr->method->body);
			
			bool all_const_expr = true;
			for(int i = 0; i < expr->arguments.GetCount(); i++)
				if (dynamic_cast<CiConstExpr*>(expr->arguments[i]) == NULL)
					all_const_expr = false;
				
			if (ret != NULL
			 && expr->method->call_type == StaticCallType
			 && all_const_expr
			 && current_pure_methods.HasPtr(expr->method) == false) {
			    current_pure_methods.Add(expr->method);
				auto& paramz = expr->Signature()->params;
				for (int i = 0; i < paramz.GetCount(); i++)
					current_pure_arguments.Add(paramz[i], expr->arguments[i]);
				CiConstExpr* constFold = dynamic_cast<CiConstExpr*>(Resolve(ret->value));
				for(int i = 0; i < paramz.GetCount(); i++) {
					current_pure_arguments.Remove(paramz[i]);
				}
				current_pure_methods.Remove(expr->method);
				if (constFold != NULL)
					return constFold;
			}
			if (expr->method == CiLibrary::CharAtMethod
			 && dynamic_cast<CiConstExpr*>(expr->arguments[0])) {
				CiConstExpr* stringExpr = dynamic_cast<CiConstExpr*>(Resolve(expr->obj));
				if (stringExpr != NULL) {
					String s = stringExpr->value->ToString();
					int i = GetConstInt(expr->arguments[0]);
					if (i >= 0 && i < s.GetCount())
						return new CiConstExpr((int) s[i]);
				}
			}
			if (expr->method->is_mutator)
				MarkWritable(expr->obj);
			expr->method->called_by.Add(this->current_method);
			this->current_method->calls.Add(expr->method);
		}
		return expr;
	}

	CiExpr* VisitExpr(CiUnaryExpr* expr)
	{
		CiExpr* resolved;
		if (expr->op == Increment || expr->op == Decrement)
			resolved = ResolveLValue(expr->inner);
		else
			resolved = Resolve(expr->inner);
		CiExpr* inner = Coerce(resolved, CiIntType::Value());
		if (expr->op == Minus && dynamic_cast<CiConstExpr*>(inner))
			return new CiConstExpr(-GetConstInt(inner));
		expr->inner = inner;
		return expr;
	}

	CiExpr* VisitExpr(CiCondNotExpr* expr)
	{
		expr->inner = Coerce(Resolve(expr->inner), CiBoolType::Value());
		return expr;
	}

	CiExpr* VisitExpr(CiPostfixExpr* expr)
	{
		expr->inner = Coerce(ResolveLValue(expr->inner), CiIntType::Value());
		return expr;
	}

	CiExpr* VisitExpr(CiBinaryExpr* expr)
	{
		CiExpr* left = Resolve(expr->left);
		CiExpr* right = Resolve(expr->right);
		if (expr->op == Plus && (dynamic_cast<CiStringType*>(left->type) || dynamic_cast<CiStringType*>(right->type))) {
			if (!(dynamic_cast<CiConstExpr*>(left) && dynamic_cast<CiConstExpr*>(right)))
				throw ResolveException("String concatenation allowed only for constants. Consider using +=");
			String a = GetConstString(left);
			String b = GetConstString(right);
			return new CiConstExpr(a + b);
		}
		left = Coerce(left, CiIntType::Value());
		right = Coerce(right, CiIntType::Value());
		if (dynamic_cast<CiConstExpr*>(right)) {
			int b = GetConstInt(right);
			if (dynamic_cast<CiConstExpr*>(left)) {
				int a = GetConstInt(left);
				switch (expr->op) {
					case Asterisk: a *= b; break;
					case Slash: a /= b; break;
					case Mod: a %= b; break;
					case AndToken: a &= b; break;
					case ShiftLeft: a <<= b; break;
					case ShiftRight: a >>= b; break;
					case Plus: a += b; break;
					case Minus: a -= b; break;
					case OrToken: a |= b; break;
					case XorToken: a ^= b; break;
				}
				return new CiConstExpr(a);
			}
			if (expr->op == AndToken && (b & ~0xff) == 0) {
				CiCoercion* c = dynamic_cast<CiCoercion*>(left);
				if (c != NULL && c->inner->type == CiByteType::Value())
					left = dynamic_cast<CiExpr*>(c->inner);
			}
		}
		// expr.Left = left;
		// expr->right = right;
		// return expr;
		return new CiBinaryExpr(left, expr->op, right);
	}

	static CiType* FindCommonType(CiExpr* expr1, CiExpr* expr2)
	{
		CiType* type1 = expr1->type;
		CiType* type2 = expr2->type;
		if (type1->Equals(type2))
			return type1;
		if ((type1 == CiIntType::Value() && type2 == CiByteType::Value())
			|| (type1 == CiByteType::Value() && type2 == CiIntType::Value()))
			return CiIntType::Value();
		CiType* type = type1->Ptr();
		if (type != NULL)
			return type; // stg, ptr || stg, NULL
		type = type2->Ptr();
		if (type != NULL)
			return type; // ptr, stg || NULL, stg
		if (type1 != CiType::Null)
			return type1; // ptr, NULL
		if (type2 != CiType::Null)
			return type2; // NULL, ptr
		throw ResolveException("Incompatible types");
	}

	CiExpr* VisitExpr(CiBoolBinaryExpr* expr)
	{
		CiExpr* left = Resolve(expr->left);
		CiExpr* right = Resolve(expr->right);
		CiType* type = NULL;
		switch (expr->op) {
			case CondAndToken:
			case CondOrToken:
				type = CiBoolType::Value();
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
		CiConstExpr* cleft = dynamic_cast<CiConstExpr*>(left);
		if (cleft != NULL) {
			CiConstExpr* cright = NULL;
			switch (expr->op) {
			case CondAndToken:
				return cleft->value->GetInt() ? right : new CiConstExpr(false);
			case CondOrToken:
				return cleft->value->GetInt() ? new CiConstExpr(true) : right;
			case Equal:
			case NotEqual:
				cright = dynamic_cast<CiConstExpr*>(right);
				if (cright) {
					bool eq = cleft->value->Equals(*cright->value);
					return new CiConstExpr(expr->op == Equal ? eq : !eq);
				}
				break;
			default:
				if (dynamic_cast<CiConstExpr*>(right)) {
					int a = GetConstInt(cleft);
					int b = GetConstInt(right);
					bool result = false;
					switch (expr->op) {
						case Less: result = a < b; break;
						case LessOrEqual: result = a <= b; break;
						case Greater: result = a > b; break;
						case GreaterOrEqual: result = a >= b; break;
						default:
							expr->left = left;
							expr->right = right;
							return expr;
					}
					return new CiConstExpr(result);
				}
				break;
			}
		}
		expr->left = left;
		expr->right = right;
		return expr;
	}

	CiExpr* VisitExpr(CiCondExpr* expr)
	{
		CiExpr* cond = Coerce(Resolve(expr->cond), CiBoolType::Value());
		CiExpr* expr1 = Resolve(expr->on_true);
		CiExpr* expr2 = Resolve(expr->on_false);
		CiType* type = FindCommonType(expr1, expr2);
		expr1 = Coerce(expr1, type);
		expr2 = Coerce(expr2, type);
		CiConstExpr* konst = dynamic_cast<CiConstExpr*>(cond);
		if (konst != NULL)
			return konst->value->GetInt() ? expr1 : expr2;
		// expr.Cond = cond;
		// expr.OnTrue = expr1;
		// expr.OnFalse = expr2;
		// return expr;
		return new CiCondExpr(
			cond,
			type,
			expr1,
			expr2);
	}

	CiExpr* VisitExpr(CiBinaryResourceExpr* expr)
	{
		String name = ResolveConstExpr(expr->name_expr, CiStringPtrType::Value())->ToString();
		int i = this->binary_resources.Find(name);
		if (i < 0) {
			CiBinaryResource* resource = new CiBinaryResource();
			resource->name = name;
			
			String c = LoadFile(FindFile(name));
			resource->content.SetCount(c.GetCount());
			memcpy(resource->content.Begin(), c.Begin(), c.GetCount());
			
			resource->type = new CiArrayStorageType(CiByteType::Value(), resource->content.GetCount());
			this->binary_resources.Add(name, resource);
		}
		expr->resource = this->binary_resources.Get(name);
		return expr;
	}

	void CheckCreatable(CiType* type)
	{
		CiClass* storageClass = type->StorageClass();
		if (storageClass != NULL && storageClass->is_abstract)
			throw ResolveException("Cannot create instances of an abstract class " + storageClass->name);
	}

	CiExpr* VisitExpr(CiNewExpr* expr)
	{
		CiType* type = expr->new_type;
		CiClassStorageType* classStorageType = dynamic_cast<CiClassStorageType*>(type);
		if (classStorageType != NULL) {
			classStorageType->class_ = ResolveClass(classStorageType->class_);
			classStorageType->class_->is_allocated = true;
		}
		else {
			CiArrayStorageType* arrayStorageType = dynamic_cast<CiArrayStorageType*>(type);
			arrayStorageType->element_type = Resolve(arrayStorageType->element_type);
			arrayStorageType->length_expr = Coerce(Resolve(arrayStorageType->length_expr), CiIntType::Value());
		}
		CheckCreatable(type);
		return expr;
	}

	CiExpr* Resolve(CiExpr* expr)
	{
		return expr->Accept(this);
	}

	void VisitSymbol(CiField* field)
	{
		field->type = Resolve(field->type);
		CheckCreatable(field->type);
	}

	bool Resolve(Vector<ICiStatement*>& statements)
	{
		bool reachable = true;
		for(int i = 0; i < statements.GetCount(); i++) {
			ICiStatement* child = statements[i];
			if (!reachable)
				throw ResolveException("Unreachable statement");
			child->Accept(*this);
			reachable = child->CompletesNormally();
		}
		return reachable;
	}

	void VisitStmt(CiBlock* statement) {
		statement->completes_normally = Resolve(statement->statements);
	}

	void VisitStmt(CiConst* statement) {
		
	}

	void VisitStmt(CiVar* statement)
	{
		CiType* type = Resolve(statement->type);
		statement->type = type;
		CheckCreatable(type);
		if (statement->initial_value != NULL) {
			CiExpr* initialValue = Resolve(statement->initial_value);
			CheckCopyPtr(type, initialValue);
			CiArrayStorageType* ast = dynamic_cast<CiArrayStorageType*>(type);
			if (ast) {
				type = ast->element_type;
				CiConstExpr* ce = dynamic_cast<CiConstExpr*>(Coerce(initialValue, type));
				if (ce == NULL)
					throw ResolveException("Array initializer is not constant");
				statement->initial_value = ce;
				if (type == CiBoolType::Value()) {
					if (ce->value->GetInt() != 0)
						throw ResolveException("Bool arrays can only be initialized with false");
				}
				else if (type == CiByteType::Value()) {
					if (ce->value->GetInt() != 0)
						throw ResolveException("Byte arrays can only be initialized with zero");
				}
				else if (type == CiIntType::Value()) {
					if (ce->value->GetInt() != 0)
						throw ResolveException("Int arrays can only be initialized with zero");
				}
				else
					throw ResolveException("Invalid array initializer");
			}
			else
				statement->initial_value = Coerce(initialValue, type);
		}
	}

	void VisitStmt(CiExpr* statement)
	{
		Resolve(statement);
	}

	void VisitStmt(CiAssign* statement)
	{
		statement->target = ResolveLValue(statement->target);
		CiVarAccess* va = dynamic_cast<CiVarAccess*>(statement->target);
		if (va  && (va->var == this->current_method->this_))
			throw ResolveException("Cannot assign to this");
		CiMaybeAssign* source = statement->source;
		CiAssign* a = dynamic_cast<CiAssign*>(source);
		if (a)
			Resolve(a);
		else
			source = Resolve(dynamic_cast<CiExpr*>(source));
		CiType* type = statement->target->type;
		CheckCopyPtr(type, source);
		statement->source = Coerce(source, type);
		if (statement->op != Assign && type != CiIntType::Value() && type != CiByteType::Value()) {
			if (statement->op == AddAssign && dynamic_cast<CiStringStorageType*>(type) && dynamic_cast<CiStringType*>(statement->source->type))
				{} // OK
			else
				throw ResolveException("Invalid compound assignment");
		}
	}

	void VisitStmt(CiDelete* statement)
	{
		statement->expr = Resolve(statement->expr);
		ICiPtrType* type = dynamic_cast<ICiPtrType*>(statement->expr->type);
		if (type)
			throw ResolveException("'delete' takes a class or array pointer");
		if (statement->expr->HasSideEffect())
			throw ResolveException("Side effects not allowed in 'delete'");
		this->writable_ptr_types.Add(type);
	}

	void VisitStmt(CiBreak* statement)
	{
		if (this->current_loop_or_switch == NULL)
			throw ResolveException("break outside loop and switch");
		this->current_loop_or_switch->completes_normally = true;
	}

	void VisitStmt(CiContinue* statement)
	{
		if (this->current_loop == NULL)
			throw ResolveException("continue outside loop");
	}

	void ResolveLoop(CiLoop* statement)
	{
		statement->completes_normally = false;
		if (statement->cond != NULL) {
			statement->cond = Coerce(Resolve(statement->cond), CiBoolType::Value());
			statement->completes_normally = !statement->cond->IsConst(new Object(false));
		}
		CiLoop* oldLoop = this->current_loop;
		CiCondCompletionStatement* oldLoopOrSwitch = this->current_loop_or_switch;
		this->current_loop_or_switch = this->current_loop = statement;
		Resolve(statement->body);
		this->current_loop = oldLoop;
		this->current_loop_or_switch = oldLoopOrSwitch;
	}

	void VisitStmt(CiDoWhile* statement)
	{
		ResolveLoop(statement);
	}

	void VisitStmt(CiFor* statement)
	{
		if (statement->init != NULL)
			Resolve(statement->init);
		if (statement->advance != NULL)
			Resolve(statement->advance);
		ResolveLoop(statement);
	}

	void VisitStmt(CiIf* statement)
	{
		statement->cond = Coerce(Resolve(statement->cond), CiBoolType::Value());
		Resolve(statement->on_true);
		if (statement->on_false != NULL) {
			Resolve(statement->on_false);
			statement->completes_normally = statement->on_true->CompletesNormally() || statement->on_false->CompletesNormally();
		}
		else
			statement->completes_normally = true;
	}

	void VisitStmt(CiNativeBlock* statement) {
		
	}

	void VisitStmt(CiReturn* statement)
	{
		CiType* type = this->current_method->signature->return_type;
		if (type != CiType::Void)
			statement->value = Coerce(Resolve(statement->value), type);
	}

	void VisitStmt(CiSwitch* statement)
	{
		statement->value = Resolve(statement->value);
		CiType* type = statement->value->type;
		CiCondCompletionStatement* oldLoopOrSwitch = this->current_loop_or_switch;
		this->current_loop_or_switch = statement;

		PtrIndex<Object*> values;
		CiCase* fallthroughFrom = NULL;
		for(int i = 0; i < statement->cases.GetCount(); i++) {
			CiCase* kase = statement->cases[i];
			for (int i = 0; i < kase->values.GetCount(); i++) {
				kase->values[i] = ResolveConstExpr(dynamic_cast<CiExpr*>(kase->values[i]), type);
				if (!values.Add(kase->values[i]))
					throw ResolveException("Duplicate case value");
			}
			if (fallthroughFrom != NULL) {
				if (fallthroughFrom->fallthrough_to == NULL)
					throw ResolveException("goto default followed by case");
				if (!ResolveConstExpr(fallthroughFrom->fallthrough_to, type)->Equals(*kase->values[0]))
					throw ResolveException("goto case doesn't match the next case");
			}
			bool reachable = Resolve(kase->body);
			if (kase->fallthrough) {
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

		if (statement->default_body.GetCount()) {
			if (fallthroughFrom != NULL && fallthroughFrom->fallthrough_to != NULL)
				throw ResolveException("goto case followed by default");
			bool reachable = Resolve(statement->default_body);
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
		statement->message = Coerce(Resolve(statement->message), CiStringPtrType::Value());
		this->throwing_methods.Add(this->current_method);
	}

	void VisitStmt(CiWhile* statement)
	{
		ResolveLoop(statement);
	}

	void Resolve(ICiStatement* statement)
	{
		statement->Accept(*this);
	}

	void VisitSymbol(CiMethod* method)
	{
		this->current_method = method;
		Resolve(method->signature);
		if (method->call_type != AbstractCallType) {
			Resolve(method->body);
			if (method->signature->return_type != CiType::Void && method->body->CompletesNormally())
				throw ResolveException("Method can complete without a return value");
		}
		this->current_method = NULL;
	}

	void ResolveBase(CiClass* klass)
	{
		if (klass->base_class != NULL) {
			klass->base_class = ResolveClass(klass->base_class);
			klass->members->parent = klass->base_class->members;
		}
	}

	void VisitSymbol(CiClass* klass)
	{
		this->current_class = klass;
		this->symbols = klass->members;
		if (klass->constructor != NULL)
			klass->constructor->Accept(this);
		for(int i = 0; i < klass->members->GetCount(); i++) {
			klass->members->Get(i)->Accept(this);
		}
		klass->binary_resources.Clear();
		for(int i = 0; i < this->binary_resources.GetCount(); i++)
			klass->binary_resources.Add(this->binary_resources[i]);
		this->binary_resources.Clear();
		this->symbols = this->symbols->parent;
		this->current_class = NULL;
	}

	static void MarkWritable(ICiPtrType* type)
	{
		if (type->writability == ReadWrite)
			return;
		if (type->writability == ReadOnly)
			throw ResolveException("Attempt to write a read-only array");
		type->writability = ReadWrite;
		for(int i = 0; i < type->sources.GetCount(); i++)
			MarkWritable(type->sources[i]);
	}

	static Object* GetErrorValue(CiType* type)
	{
		if (type == CiType::Void)
			return new Object(false);
		if (type == CiIntType::Value())
			return new Object(-1);
		if (type == CiStringPtrType::Value() || dynamic_cast<CiClassPtrType*>(type) || dynamic_cast<CiArrayPtrType*>(type))
			return new Object();
		throw ResolveException("throw in a method of unsupported return type");
	}

	static void MarkThrows(CiMethod* method)
	{
		if (method->throws)
			return;
		method->throws = true;
		method->error_return_value = GetErrorValue(method->signature->return_type);
		for(int i = 0; i < method->called_by.GetCount(); i++)
			MarkThrows(method->called_by[i]);
	}

	static void MarkDead(CiMethod* method)
	{
		if (method->visibility == Private && method->call_type != OverrideCallType && method->called_by.GetCount() == 0) {
			method->visibility = Dead;
			for(int i = 0; i < method->calls.GetCount(); i++) {
				CiMethod* called = method->calls[i];
				called->called_by.Remove(method);
				MarkDead(called);
			}
		}
	}

	static void MarkDead(CiClass* klass)
	{
		for(int i = 0; i < klass->members->GetCount(); i++) {
			CiSymbol* member = klass->members->Get(i);
			CiMethod* m = dynamic_cast<CiMethod*>(member);
			if (m)
				MarkDead(m);
		}
	}

	static void MarkInternal(CiMethod* method)
	{
		bool any_caller_class_diff = false;
		for(int i = 0; i < method->called_by.GetCount(); i++) {
			CiMethod* caller = method->called_by[i];
			if (caller->class_ != method->class_)
				any_caller_class_diff = true;
		}
		
		if (method->visibility == Private && any_caller_class_diff)
			method->visibility = InternalVisib;
	}

	static void MarkInternal(CiClass* klass)
	{
		for(int i = 0; i < klass->members->GetCount(); i++) {
			CiSymbol* member = klass->members->Get(i);
			CiMethod* m = dynamic_cast<CiMethod*>(member);
			if (m)
				MarkInternal(m);
		}
	}

	void Resolve(CiProgram* program)
	{
		this->symbols = program->globals;
		for(int i = 0; i < program->globals->GetCount(); i++) {
			CiSymbol* symbol = program->globals->Get(i);
			CiClass* c = dynamic_cast<CiClass*>(symbol);
			if (c)
				ResolveBase(c);
		}
		
		for(int i = 0; i < program->globals->GetCount(); i++) {
			CiSymbol* symbol = program->globals->Get(i);
			symbol->Accept(this);
		}
		
		for(int i = 0; i < this->writable_ptr_types.GetCount(); i++)
			MarkWritable(this->writable_ptr_types[i]);
		
		for(int i = 0; i < this->throwing_methods.GetCount(); i++)
			MarkThrows(this->throwing_methods[i]);
		
		for(int i = 0; i < program->globals->GetCount(); i++) {
			CiSymbol* symbol = program->globals->Get(i);
			CiClass* c = dynamic_cast<CiClass*>(symbol);
			if (c)
				MarkDead(c);
		}
		
		for(int i = 0; i < program->globals->GetCount(); i++) {
			CiSymbol* symbol = program->globals->Get(i);
			CiClass* c = dynamic_cast<CiClass*>(symbol);
			if (c)
				MarkInternal(c);
		}
	}
};

}

