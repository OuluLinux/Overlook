
namespace Ci
{

struct CiParser : public CiLexer
{
	SymbolTable* symbols = NULL;
	Vector<CiConst*> const_arrays;
	CiClass* current_class = NULL;
	CiMethod* current_method = NULL;

	CiParser()
	{
		SymbolTable* globals = new SymbolTable();
		globals->Add(CiBoolType::Value());
		globals->Add(CiByteType::Value());
		globals->Add(CiIntType::Value());
		globals->Add(CiStringPtrType::Value());
		globals->Add(new CiConst("true", CiBoolType::Value(), new Object(true)));
		globals->Add(new CiConst("false", CiBoolType::Value(), new Object(false)));
		globals->Add(new CiConst("null", CiType::Null, NULL));
		this->symbols = new SymbolTable(globals);// { parent = globals };
	}

	String ParseId()
	{
		String id = this->current_string;
		Expect(Id);
		return id;
	}

	CiCodeDoc* ParseDoc()
	{
		if (See(DocComment)) {
			CiDocParser* parser = new CiDocParser(this);
			return parser->ParseCodeDoc();
		}
		return NULL;
	}

	CiEnum* ParseEnum()
	{
		CiEnum* enu = new CiEnum();
		Expect(Enum);
		enu->name = ParseId();
		Expect(LeftBrace);
		Vector<CiEnumValue*> values;
		do {
			CiEnumValue* value = new CiEnumValue();
			value->documentation = ParseDoc();
			value->name = ParseId();
			value->type = enu;
			values.Add(value);
		} while (Eat(Comma));
		Expect(RightBrace);
		enu->values <<= values;
		return enu;
	}

	CiType* LookupType(String name)
	{
		CiSymbol* symbol = this->symbols->Lookup(name);
		CiType* t = dynamic_cast<CiType*>(symbol);
		if (t)
			return t;
		CiClass* c = dynamic_cast<CiClass*>(symbol);
		if (c)
			return new CiClassPtrType(name, c);
		if (symbol == NULL) {
			CiType* unknown = new CiUnknownType();
			unknown->name = name;
			return unknown;
		}
		throw ParseException(name + " is not a type");
	}

	CiType* ParseArrayType(CiType* baseType)
	{
		if (Eat(LeftBracket)) {
			if (Eat(RightBracket))
				return new CiArrayPtrType(ParseArrayType(baseType));
			CiExpr* len = ParseExpr();
			Expect(RightBracket);
			return new CiArrayStorageType(len, ParseArrayType(baseType));
		}
		return baseType;
	}

	CiType* ParseType()
	{
		String baseName = ParseId();
		CiType* baseType;
		if (Eat(LeftParenthesis)) {
			if (baseName == "string") {
				baseType = new CiStringStorageType(ParseExpr());
				Expect(RightParenthesis);
			}
			else {
				Expect(RightParenthesis);
				baseType = new CiClassStorageType(baseName, new CiUnknownClass(baseName));
			}
		}
		else
			baseType = LookupType(baseName);
		return ParseArrayType(baseType);
	}

	Object* ParseConstInitializer(CiType* type)
	{
		CiArrayType* t = dynamic_cast<CiArrayType*>(type);
		if (t) {
			Expect(LeftBrace);
			CiType* elementType = t->element_type;
			Vector<Object*> list;
			if (!See(RightBrace)) {
				do
					list.Add(ParseConstInitializer(elementType));
				while (Eat(Comma));
			}
			Expect(RightBrace);
			return new Object(list);
		}
		return ParseExpr();
	}

	CiConst* ParseConst()
	{
		Expect(Const);
		CiConst* konst = new CiConst();
		konst->type = ParseType();
		konst->name = ParseId();
		Expect(Assign);
		konst->value = ParseConstInitializer(konst->type);
		Expect(Semicolon);
		if (this->symbols->parent != NULL && dynamic_cast<CiArrayType*>(konst->type)) {
			this->const_arrays.Add(konst);
			konst->global_name = "CiConstArray_" + IntStr(this->const_arrays.GetCount());
		}
		return konst;
	}

	CiBinaryResourceExpr* ParseBinaryResource()
	{
		Expect(LeftParenthesis);
		CiExpr* nameExpr = ParseExpr();
		Expect(RightParenthesis);
		return new CiBinaryResourceExpr(nameExpr);
	}

	CiExpr* ParsePrimaryExpr()
	{
		if (See(Increment) || See(Decrement) || See(Minus) || See(Not)) {
			CiToken op = this->current_token;
			NextToken();
			CiExpr* inner = ParsePrimaryExpr();
			return new CiUnaryExpr(op, inner);
		}
		if (Eat(CondNot)) {
			CiExpr* inner = ParsePrimaryExpr();
			return new CiCondNotExpr(inner);
		}
		CiExpr* result;
		if (See(IntConstant)) {
			result = new CiConstExpr(this->current_int);
			NextToken();
		}
		else if (See(StringConstant)) {
			result = new CiConstExpr(this->current_string);
			NextToken();
		}
		else if (Eat(LeftParenthesis)) {
			result = ParseExpr();
			Expect(RightParenthesis);
		}
		else if (See(Id)) {
			String name = ParseId();
			if (name == "BinaryResource")
				result = ParseBinaryResource();
			else {
				CiSymbol* symbol = this->symbols->Lookup(name);
				CiMacro* m = dynamic_cast<CiMacro*>(symbol);
				if (m) {
					Expand(m);
					Expect(LeftParenthesis);
					result = ParseExpr();
					Expect(RightParenthesis);
				}
				else {
					if (symbol == NULL)
						symbol = new CiUnknownSymbol(name);
					result = new CiSymbolAccess(symbol);
				}
			}
		}
		else if (Eat(New)) {
			CiType* newType = ParseType();
			if (!(dynamic_cast<CiClassStorageType*>(newType) || dynamic_cast<CiArrayStorageType*>(newType)))
				throw ParseException("'new' syntax error");
			result = new CiNewExpr(newType);
		}
		else
			throw ParseException("Invalid expression");
		for (;;) {
			if (Eat(Dot))
				result = new CiUnknownMemberAccess(result, ParseId());
			else if (Eat(LeftParenthesis)) {
				CiMethodCall* call = new CiMethodCall();
				call->obj = result;
				Vector<CiExpr*> args;
				if (!See(RightParenthesis)) {
					do
						args.Add(ParseExpr());
					while (Eat(Comma));
				}
				Expect(RightParenthesis);
				call->arguments <<= args;
				result = call;
			}
			else if (Eat(LeftBracket)) {
				CiExpr* index = ParseExpr();
				Expect(RightBracket);
				result = new CiIndexAccess(result, index);
			}
			else if (See(Increment) || See(Decrement)) {
				CiToken op = this->current_token;
				NextToken();
				return new CiPostfixExpr(result, op);
			}
			else
				return result;
		}
	}

	CiExpr* ParseMulExpr()
	{
		CiExpr* left = ParsePrimaryExpr();
		while (See(Asterisk) || See(Slash) || See(Mod)) {
			CiToken op = this->current_token;
			NextToken();
			left = new CiBinaryExpr(left, op, ParsePrimaryExpr());
		}
		return left;
	}

	CiExpr* ParseAddExpr()
	{
		CiExpr* left = ParseMulExpr();
		while (See(Plus) || See(Minus)) {
			CiToken op = this->current_token;
			NextToken();
			left = new CiBinaryExpr(left, op, ParseMulExpr());
		}
		return left;
	}

	CiExpr* ParseShiftExpr()
	{
		CiExpr* left = ParseAddExpr();
		while (See(ShiftLeft) || See(ShiftRight)) {
			CiToken op = this->current_token;
			NextToken();
			left = new CiBinaryExpr(left, op, ParseAddExpr());
		}
		return left;
	}

	CiExpr* ParseRelExpr()
	{
		CiExpr* left = ParseShiftExpr();
		while (See(Less) || See(LessOrEqual) || See(Greater) || See(GreaterOrEqual)) {
			CiToken op = this->current_token;
			NextToken();
			left = new CiBoolBinaryExpr(left, op, ParseShiftExpr());
		}
		return left;
	}

	CiExpr* ParseEqualityExpr()
	{
		CiExpr* left = ParseRelExpr();
		while (See(Equal) || See(NotEqual)) {
			CiToken op = this->current_token;
			NextToken();
			left = new CiBoolBinaryExpr(left, op, ParseRelExpr());
		}
		return left;
	}

	CiExpr* ParseAndExpr()
	{
		CiExpr* left = ParseEqualityExpr();
		while (Eat(AndToken))
			left = new CiBinaryExpr(left, AndToken, ParseEqualityExpr());
		return left;
	}

	CiExpr* ParseXorExpr()
	{
		CiExpr* left = ParseAndExpr();
		while (Eat(XorToken))
			left = new CiBinaryExpr(left, XorToken, ParseAndExpr());
		return left;
	}

	CiExpr* ParseOrExpr()
	{
		CiExpr* left = ParseXorExpr();
		while (Eat(OrToken))
			left = new CiBinaryExpr(left, OrToken, ParseXorExpr());
		return left;
	}

	CiExpr* ParseCondAndExpr()
	{
		CiExpr* left = ParseOrExpr();
		while (Eat(CondAndToken))
			left = new CiBoolBinaryExpr(left, CondAndToken, ParseOrExpr());
		return left;
	}

	CiExpr* ParseCondOrExpr()
	{
		CiExpr* left = ParseCondAndExpr();
		while (Eat(CondOrToken))
			left = new CiBoolBinaryExpr(left, CondOrToken, ParseCondAndExpr());
		return left;
	}

	CiExpr* ParseExpr()
	{
		CiExpr* left = ParseCondOrExpr();
		if (Eat(QuestionMark)) {
			CiCondExpr* result = new CiCondExpr();
			result->cond = left;
			result->on_true = ParseExpr();
			Expect(Colon);
			result->on_false = ParseExpr();
			return result;
		}
		return left;
	}

	CiMaybeAssign* ParseMaybeAssign()
	{
		CiExpr* left = ParseExpr();
		CiToken op = this->current_token;
		if (op == Assign || op == AddAssign || op == SubAssign || op == MulAssign || op == DivAssign || op == ModAssign
		 || op == AndAssign || op == OrAssign || op == XorAssign || op == ShiftLeftAssign || op == ShiftRightAssign) {
			NextToken();
			CiAssign* result = new CiAssign();
			result->target = left;
			result->op = op;
			result->source = ParseMaybeAssign();
			return result;
		}
		return left;
	}

	ICiStatement* ParseExprWithSideEffect()
	{
		ICiStatement* result = dynamic_cast<ICiStatement*>(ParseMaybeAssign());
		if (result == NULL)
			throw ParseException("Useless expression");
		return result;
	}

	CiExpr* ParseCond()
	{
		Expect(LeftParenthesis);
		CiExpr* cond = ParseExpr();
		Expect(RightParenthesis);
		return cond;
	}

	void OpenScope()
	{
		this->symbols = new SymbolTable(this->symbols);
	}

	void CloseScope()
	{
		this->symbols = this->symbols->parent;
	}

	CiVar* ParseVar()
	{
		CiVar* def = new CiVar();
		def->type = ParseType();
		def->name = ParseId();
		if (Eat(Assign))
			def->initial_value = ParseExpr();
		Expect(Semicolon);
		this->symbols->Add(def);
		return def;
	}

	ICiStatement* ParseVarOrExpr()
	{
		String name = this->current_string;
		CiSymbol* symbol = this->symbols->Lookup(name);
		CiMacro* m = dynamic_cast<CiMacro*>(symbol);
		if (m) {
			NextToken();
			Expand(m);
			return ParseStatement();
		}
		// try var
		SharedString* sb = new SharedString();
		copy_to = sb;
		try {
			CiVar* var = ParseVar();
			copy_to = NULL;
			return var;
		}
		catch (ParseException) {
		}
		copy_to = NULL;

		// try expr
		this->current_string = name;
		this->current_token = Id;
		BeginExpand("ambiguous code", *sb, VectorMap<String,String>());
		SetReader(new StringReader(*sb));
		ICiStatement* result = ParseExprWithSideEffect();
		Expect(Semicolon);
		return result;
	}

	CiNativeBlock* ParseNativeBlock()
	{
		SharedString* sb = new SharedString();
		this->copy_to = sb;
		
		Expect(LeftBrace);
		int level = 1;
		for (;;) {
			if (See(EndOfFile))
				throw ParseException("Native block not terminated");
			if (See(LeftBrace))
				level++;
			else if (See(RightBrace))
				if (--level == 0)
					break;
			NextToken();
		}
		
		this->copy_to = NULL;
		NextToken();
		ASSERT((*sb)[sb->GetCount() - 1] == '}');
		return new CiNativeBlock(sb->Left(sb->GetCount() - 1));
	}

	CiReturn* ParseReturn()
	{
		CiReturn* result = new CiReturn();
		if (this->current_method->signature->return_type != CiType::Void)
			result->value = ParseExpr();
		Expect(Semicolon);
		return result;
	}

	CiSwitch* ParseSwitch()
	{
		Expect(LeftParenthesis);
		CiSwitch* result = new CiSwitch();
		result->value = ParseExpr();
		Expect(RightParenthesis);
		Expect(LeftBrace);

		Vector<CiCase*> cases;
		while (Eat(Case)) {
			Vector<Object*> values;
			do {
				values.Add(ParseExpr());
				Expect(Colon);
			} while (Eat(Case));
			if (See(Default))
				throw ParseException("Please remove case before default");
			CiCase* kase = new CiCase(values);

			Vector<ICiStatement*> statements;
			do
				statements.Add(ParseStatement());
			while (!See(Case) && !See(Default) && !See(Goto) && !See(RightBrace));
			kase->body <<= statements;

			if (Eat(Goto)) {
				if (Eat(Case))
					kase->fallthrough_to = ParseExpr();
				else if (Eat(Default))
					kase->fallthrough_to = NULL;
				else
					throw ParseException("Expected goto case or goto default");
				Expect(Semicolon);
				kase->fallthrough = true;
			}
			cases.Add(kase);
		}
		if (cases.GetCount() == 0)
			throw ParseException("Switch with no cases");
		result->cases <<= cases;

		if (Eat(Default)) {
			Expect(Colon);
			Vector<ICiStatement*> statements;
			do
				statements.Add(ParseStatement());
			while (!See(RightBrace));
			result->default_body <<= statements;
		}

		Expect(RightBrace);
		return result;
	}

	ICiStatement* ParseStatement()
	{
		while (Eat(Macro))
			this->symbols->Add(ParseMacro());
		if (See(Id))
			return ParseVarOrExpr();
		if (See(LeftBrace)) {
			OpenScope();
			CiBlock* result = ParseBlock();
			CloseScope();
			return result;
		}
		if (Eat(Break)) {
			Expect(Semicolon);
			return new CiBreak();
		}
		if (See(Const)) {
			CiConst* konst = ParseConst();
			this->symbols->Add(konst);
			return konst;
		}
		if (Eat(Continue)) {
			Expect(Semicolon);
			return new CiContinue();
		}
		if (Eat(Delete)) {
			CiExpr* expr = ParseExpr();
			Expect(Semicolon);
			return new CiDelete(expr);
		}
		if (Eat(Do)) {
			CiDoWhile* result = new CiDoWhile();
			result->body = ParseStatement();
			Expect(While);
			result->cond = ParseCond();
			Expect(Semicolon);
			return result;
		}
		if (Eat(For)) {
			Expect(LeftParenthesis);
			OpenScope();
			CiFor* result = new CiFor();
			if (See(Id))
				result->init = ParseVarOrExpr();
			else
				Expect(Semicolon);
			if (!See(Semicolon))
				result->cond = ParseExpr();
			Expect(Semicolon);
			if (!See(RightParenthesis))
				result->advance = ParseExprWithSideEffect();
			Expect(RightParenthesis);
			result->body = ParseStatement();
			CloseScope();
			return result;
		}
		if (Eat(If)) {
			CiIf* result = new CiIf();
			result->cond = ParseCond();
			result->on_true = ParseStatement();
			if (Eat(Else))
				result->on_false = ParseStatement();
			return result;
		}
		if (Eat(Native))
			return ParseNativeBlock();
		if (Eat(Return))
			return ParseReturn();
		if (Eat(Switch))
			return ParseSwitch();
		if (Eat(Throw)) {
			CiThrow* result = new CiThrow();
			result->message = ParseExpr();
			Expect(Semicolon);
			return result;
		}
		if (Eat(While)) {
			CiWhile* result = new CiWhile();
			result->cond = ParseCond();
			result->body = ParseStatement();
			return result;
		}
		throw ParseException("Invalid statement");
	}

	CiBlock* ParseBlock()
	{
		Expect(LeftBrace);
		Vector<ICiStatement*> statements;
		while (!Eat(RightBrace))
			statements.Add(ParseStatement());
		return new CiBlock(statements);
	}

	CiParam* CreateThis()
	{
		CiParam* thiz = new CiParam();
		thiz->type = new CiClassPtrType(this->current_class->name, this->current_class);
		thiz->name = "this";
		this->symbols->Add(thiz);
		return thiz;
	}

	CiType* ParseReturnType()
	{
		if (Eat(Void))
			return CiType::Void;
		return ParseType();
	}

	Vector<CiParam*> ParseParams()
	{
		Expect(LeftParenthesis);
		Vector<CiParam*> paramz;
		if (!See(RightParenthesis)) {
			do {
				CiParam* param = new CiParam();
				param->documentation = ParseDoc();
				param->type = ParseType();
				param->name = ParseId();
				this->symbols->Add(param);
				paramz.Add(param);
			} while (Eat(Comma));
		}
		Expect(RightParenthesis);
		return paramz;
	}

	void ParseMethod(CiMethod* method)
	{
		this->current_method = method;
		OpenScope();
		if (method->call_type != StaticCallType)
			method->this_ = CreateThis();
		method->signature->params = ParseParams();
		if (method->call_type == AbstractCallType)
			Expect(Semicolon);
		else if (method->signature->return_type != CiType::Void && Eat(Return))
			method->body = ParseReturn();
		else
			method->body = ParseBlock();
		CloseScope();
		this->current_method = NULL;
	}

	CiMethod* ParseConstructor()
	{
		OpenScope();
		CiMethod* method = new CiMethod(
			CiType::Void, "<constructor>",
			this->current_class,
			CreateThis());
		this->current_method = method;
		method->body = ParseBlock();
		CloseScope();
		this->current_method = NULL;
		return method;
	}

	CiClass* ParseClass()
	{
		CiClass* klass = new CiClass();
		klass->source_filename = this->filename;
		if (Eat(AbstractToken))
			klass->is_abstract = true;
		Expect(Class);
		klass->name = ParseId();
		if (Eat(Colon))
			klass->base_class = new CiUnknownClass(ParseId());
		Expect(LeftBrace);
		OpenScope();
		this->current_class = klass;
		klass->members = this->symbols;
		while (!Eat(RightBrace)) {
			CiCodeDoc* doc = ParseDoc();
			CiVisibility visibility = Private;
			if (Eat(PublicToken))
				visibility = PublicVisib;
			else if (Eat(InternalToken))
				visibility = InternalVisib;
			CiSymbol* symbol;
			if (See(Const)) {
				CiConst* konst = ParseConst();
				symbol = konst;
				konst->class_ = klass;
			}
			else if (Eat(Macro)) {
				if (visibility != Private)
					throw ParseException("Macros must be private");
				symbol = ParseMacro();
			}
			else {
				CiCallType callType;
				if (Eat(StaticToken))
					callType = StaticCallType;
				else if (Eat(AbstractToken)) {
					if (!klass->is_abstract)
						throw ParseException("Abstract methods only allowed in abstract classes");
					callType = AbstractCallType;
					if (visibility == Private)
						visibility = InternalVisib;
				}
				else if (Eat(VirtualToken)) {
					callType = VirtualCallType;
					if (visibility == Private)
						visibility = InternalVisib;
				}
				else if (Eat(OverrideToken)) {
					callType = OverrideCallType;
					if (visibility == Private)
						visibility = InternalVisib;
				}
				else
					callType = NormalCallType;
				CiType* type = ParseReturnType();
				if (dynamic_cast<CiClassStorageType*>(type) && See(LeftBrace)) {
					if (type->name != klass->name)
						throw ParseException(type->name + "() looks like a constructor, but it is in a different class " + klass->name);
					if (callType != NormalCallType)
						throw ParseException("Constructor cannot be static, abstract, virtual or override");
					if (klass->constructor != NULL)
						throw ParseException("Duplicate constructor");
					klass->constructor = ParseConstructor();
					continue;
				}
				String name = ParseId();
				if (See(LeftParenthesis)) {
					CiMethod* method = new CiMethod(type, name,
						klass,
						callType);
					ParseMethod(method);
					symbol = method;
				}
				else {
					if (visibility != Private)
						throw ParseException("Fields must be private");
					if (callType != NormalCallType)
						throw ParseException("Fields cannot be static, abstract, virtual or override");
					if (type == CiType::Void)
						throw ParseException("Field is void");
					Expect(Semicolon);
					symbol = new CiField(klass, type, name);
				}
			}
			symbol->documentation = doc;
			symbol->visibility = visibility;
			klass->members->Add(symbol);
		}
		this->current_class = NULL;
		CloseScope();
		klass->const_arrays <<= this->const_arrays;
		this->const_arrays.Clear();
		return klass;
	}

	CiDelegate* ParseDelegate()
	{
		CiDelegate* del = new CiDelegate();
		Expect(Delegate);
		del->return_type = ParseReturnType();
		del->name = ParseId();
		OpenScope();
		del->params = ParseParams();
		CloseScope();
		Expect(Semicolon);
		return del;
	}

	void Parse(String filename, StringReader* reader)
	{
		Open(filename, reader);
		while (!See(EndOfFile)) {
			CiCodeDoc* doc = ParseDoc();
			bool pub = Eat(PublicToken);
			CiSymbol* symbol;
			if (See(Enum))
				symbol = ParseEnum();
			else if (See(Class) || See(AbstractToken))
				symbol = ParseClass();
			else if (See(Delegate))
				symbol = ParseDelegate();
			else
				throw ParseException("Expected class, enum or delegate");
			symbol->documentation = doc;
			symbol->visibility = pub ? PublicVisib : InternalVisib;
			this->symbols->Add(symbol);
		}
	}

	CiProgram* Program()
	{
		return new CiProgram(symbols);
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	// Macro parsing
	
	
	void ParseBody(CiToken left, CiToken right)
	{
		int level = 1;
		for (;;) {
			NextToken();
			if (See(EndOfFile))
				throw ParseException("Macro definition not terminated");
			if (See(left))
				level++;
			else if (See(right))
				if (--level == 0)
					break;
		}
	}

	CiMacro* ParseMacro()
	{
		CiMacro* macro = new CiMacro();
		macro->name = ParseId();
		Expect(LeftParenthesis);
		Index<String> paramz;
		if (See(Id)) {
			do {
				String name = ParseId();
				if (paramz.Find(name) >= 0)
					throw ParseException("Duplicate macro parameter " + name);
				paramz.Add(name);
			} while (Eat(Comma));
		}
		Expect(RightParenthesis);
		macro->params <<= paramz;
		SharedString* sb = new SharedString();
		this->copy_to = sb;
		
		if (See(LeftParenthesis)) {
			sb->Cat('(');
			ParseBody(LeftParenthesis, RightParenthesis);
		}
		else if (See(LeftBrace)) {
			ParseBody(LeftBrace, RightBrace);
			ASSERT((*sb)[sb->GetCount() - 1] == '}');
			sb->String::operator=(sb->Left(sb->GetCount() - 1));
			macro->is_statement = true;
		}
		else
			throw ParseException("Macro definition must be wrapped in parentheses or braces");
		
		this->copy_to = NULL;
		
		macro->body = *sb;
		NextToken();
		return macro;
	}

	void ParseArg()
	{
		int level = 0;
		for (;;) {
			if (See(EndOfFile))
				throw ParseException("Macro argument not terminated");
			if (See(LeftParenthesis))
				level++;
			else if (See(RightParenthesis)) {
				if (--level < 0)
					break;
			}
			else if (level == 0 && See(Comma))
				break;
			NextToken();
		}
	}

	Stack<MacroExpansion*> macro_stack;

	void PrintMacroStack()
	{
		for(int i = 0; i < macro_stack.GetCount(); i++) {
			RLOG("   in " + macro_stack[i]->friendly_name);
		}
	}

	void BeginExpand(String friendlyName, String content, const VectorMap<String, String>& args)
	{
		this->macro_stack.Push(new MacroExpansion(
			friendlyName,
			args,
			SetReader(new StringReader(content))));
	}

	void Expand(CiMacro* macro)
	{
		VectorMap<String, String> args;
		SharedString* sb = new SharedString();
		this->copy_to = sb;
		
		Expect(LeftParenthesis);
		bool first = true;
		for(int i = 0; i < macro->params.GetCount(); i++) {
			String name = macro->params[i];
			
			if (first)
				first = false;
			else
				Expect(Comma);
			ParseArg();
			char c = (*sb)[sb->GetCount() - 1];
			ASSERT(c == ',' || c == ')');
			sb->String::operator=(sb->Left(sb->GetCount() - 1));
			args.Add(name, TrimBoth(*sb));
			sb->String::operator=("");
		}
		
		this->copy_to = NULL;
		
		Check(RightParenthesis);
		if (macro->is_statement) {
			NextToken();
			Check(Semicolon);
		}
		BeginExpand("macro " + macro->name, macro->body, args);
		NextToken();
	}

	bool IsExpandingMacro()
	{
		return this->macro_stack.GetCount() > 0;
	}

	bool ExpandMacroArg(String name)
	{
		if (this->macro_stack.GetCount() > 0) {
			String value = this->macro_stack.Peek()->LookupArg(name);
			if (value != "") {
				BeginExpand("macro argument " + name, value, VectorMap<String,String>());
				return true;
			}
		}
		return false;
	}

	bool OnStreamEnd()
	{
		if (this->macro_stack.GetCount() > 0) {
			MacroExpansion* top = this->macro_stack.Pop();
			SetReader(top->parent_reader);
			return true;
		}
		return false;
	}
};

}
