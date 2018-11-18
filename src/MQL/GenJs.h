
namespace Ci
{

class GenJs : public SourceGenerator
{
	CiClass* current_class = NULL;
	bool UsesSubstringMethod = false;
	bool UsesCopyArrayMethod = false;
	bool UsesBytesToStringMethod = false;
	bool UsesClearArrayMethod = false;
	
	void Write(ICiStatement* s) {SourceGenerator::Write(s);}
	void Write(CiExpr* e) {SourceGenerator::Write(e);}
	void Write(String s) {SourceGenerator::Write(s);}
	void Write(int i) {SourceGenerator::Write(i);}

	/*virtual void VisitStmt(CiWhile* statement) {}
	virtual void VisitStmt(CiThrow* statement) {}
	virtual void VisitStmt(CiSwitch* statement) {}
	virtual void VisitStmt(CiReturn* statement) {}
	virtual void VisitStmt(CiIf* statement) {}
	virtual void VisitStmt(CiFor* statement) {}
	virtual void VisitStmt(CiDoWhile* statement) {}
	virtual void VisitStmt(CiContinue* statement) {}
	virtual void VisitStmt(CiBreak* statement) {}
	virtual void VisitStmt(CiDelete* statement) {}
	virtual void VisitStmt(CiAssign* statement) {}
	virtual void VisitStmt(CiExpr* statement) {}
	virtual void VisitStmt(CiVar* statement) {}
	virtual void VisitStmt(CiConst* statement) {}
	virtual void VisitStmt(CiBlock* statement) {}*/
	
	void Write(CiCodeDoc* doc)
	{
		if (doc == NULL)
			return;
		// TODO
	}

	void Write(CiEnum* enu)
	{
		WriteLine();
		Write(enu->documentation);
		Write("var ");
		Write(enu->name);
		Write(" = ");
		OpenBlock();
		for (int i = 0; i < enu->values.GetCount(); i++) {
			if (i > 0)
				WriteLine(",");
			CiEnumValue* value = enu->values[i];
			Write(value->documentation);
			WriteUppercaseWithUnderscores(value->name);
			Write(" : ");
			Write(i);
		}
		WriteLine();
		CloseBlock();
	}

	void WriteNew(CiType* type)
	{
		CiClassStorageType* classType = dynamic_cast<CiClassStorageType*>(type);
		if (classType != NULL) {
			Write("new ");
			Write(classType->class_->name);
			Write("()");
		}
		else {
			CiArrayStorageType* arrayType = dynamic_cast<CiArrayStorageType*>(type);
			Write("new Array(");
			if (arrayType->length_expr != NULL)
				SourceGenerator::Write(arrayType->length_expr);
			else
				Write(arrayType->length);
			Write(')');
		}
	}

	bool WriteInit(CiType* type)
	{
		CiClassStorageType* cst = dynamic_cast<CiClassStorageType*>(type);
		CiArrayStorageType* ast = dynamic_cast<CiArrayStorageType*>(type);
		if (cst || ast) {
			Write(" = ");
			WriteNew(type);
			return true;
		}
		return false;
	}

	void Write(CiField* field)
	{
		Write(field->documentation);
		Write("this->");
		WriteCamelCase(field->name);
		CiType* type = field->type;
		CiEnum* e = dynamic_cast<CiEnum*>(type);
		if (type == CiBoolType::Value())
			Write(" = false");
		else if (type == CiByteType::Value() || type == CiIntType::Value())
			Write(" = 0");
		else if (e) {
			Write(" = ");
			WriteConst(e->values[0]);
		}
		else if (!WriteInit(type))
			Write(" = NULL");
		WriteLine(";");
	}

	void WriteConst(Object* value)
	{
		CiEnumValue* ev = dynamic_cast<CiEnumValue*>(value);
		if (ev) {
			Write(ev->type->name);
			Write('.');
			WriteUppercaseWithUnderscores(ev->name);
		}
		else if (value->objs.GetCount()) {
			Write("[ ");
			WriteContent(value->objs);
			Write(" ]");
		}
		else
			SourceGenerator::WriteConst(value);
	}

	void WriteName(CiConst* konst)
	{
		Write(this->current_class->name);
		Write('.');
		WriteUppercaseWithUnderscores(konst->global_name.GetCount() ? konst->global_name : konst->name);
	}

	CiPriority GetPriority(CiExpr* expr)
	{
		CiPropertyAccess* pa = dynamic_cast<CiPropertyAccess*>(expr);
		CiBinaryExpr* be = dynamic_cast<CiBinaryExpr*>(expr);
		if (pa) {
			CiProperty* prop = pa->property;
			if (prop == CiLibrary::SByteProperty)
				return Additive;
			if (prop == CiLibrary::LowByteProperty)
				return AndPrior;
		}
		else if (be) {
			if (be->op == Slash)
				return Postfix;
		}
		return SourceGenerator::GetPriority(expr);
	}

	void Write(CiFieldAccess* expr)
	{
		WriteChild(expr, expr->obj);
		Write('.');
		WriteCamelCase(expr->field->name);
	}

	void Write(CiPropertyAccess* expr)
	{
		if (expr->property == CiLibrary::SByteProperty) {
			Write('(');
			WriteChild(XorPrior, expr->obj);
			Write(" ^ 128) - 128");
		}
		else if (expr->property == CiLibrary::LowByteProperty) {
			WriteChild(expr, expr->obj);
			Write(" & 0xff");
		}
		else if (expr->property == CiLibrary::StringLengthProperty) {
			WriteChild(expr, expr->obj);
			Write(".length");
		}
		else
			throw ArgumentException(expr->property->name);
	}

	void WriteName(CiBinaryResource* resource)
	{
		Write(this->current_class->name);
		Write(".CI_BINARY_RESOURCE_");
		for(int i = 0; i < resource->name.GetCount(); i++) {
			char c = resource->name[i];
			Write(IsLetter(c) ? ToUpper(c) : '_');
		}
	}

	void WriteName(CiMethod* method)
	{
		WriteCamelCase(method->name);
	}

	void Write(CiMethodCall* expr)
	{
		if (expr->method == CiLibrary::MulDivMethod) {
			Write("Math.floor(");
			WriteMulDiv(Multiplicative, expr);
		}
		else if (expr->method == CiLibrary::CharAtMethod) {
			Write(expr->obj);
			Write(".charCodeAt(");
			Write(expr->arguments[0]);
			Write(')');
		}
		else if (expr->method == CiLibrary::SubstringMethod) {
			if (expr->arguments[0]->HasSideEffect()) {
				Write("Ci.substring(");
				Write(expr->obj);
				Write(", ");
				Write(expr->arguments[0]);
				Write(", ");
				Write(expr->arguments[1]);
				Write(')');
				this->UsesSubstringMethod = true;
			}
			else {
				Write(expr->obj);
				Write(".substring(");
				Write(expr->arguments[0]);
				Write(", ");
				Write(new CiBinaryExpr(expr->arguments[0], Plus, expr->arguments[1]));
				Write(')');
			}
		}
		else if (expr->method == CiLibrary::Arraycopy_toMethod) {
			Write("Ci.copyArray(");
			Write(expr->obj);
			Write(", ");
			Write(expr->arguments[0]);
			Write(", ");
			Write(expr->arguments[1]);
			Write(", ");
			Write(expr->arguments[2]);
			Write(", ");
			Write(expr->arguments[3]);
			Write(')');
			this->UsesCopyArrayMethod = true;
		}
		else if (expr->method == CiLibrary::ArrayToStringMethod) {
			Write("Ci.bytesToString(");
			Write(expr->obj);
			Write(", ");
			Write(expr->arguments[0]);
			Write(", ");
			Write(expr->arguments[1]);
			Write(')');
			this->UsesBytesToStringMethod = true;
		}
		else if (expr->method == CiLibrary::ArrayStorageClearMethod) {
			Write("Ci.clearArray(");
			Write(expr->obj);
			Write(", 0)");
			this->UsesClearArrayMethod = true;
		}
		else
			SourceGenerator::Write(expr);
	}

	void Write(CiBinaryExpr* expr)
	{
		if (expr->op == Slash) {
			Write("Math.floor(");
			WriteChild(Multiplicative, expr->left);
			Write(" / ");
			WriteNonAssocChild(Multiplicative, expr->right);
			Write(')');
		}
		else
			SourceGenerator::Write(expr);
	}

	virtual void WriteInitArrayStorageVar(CiVar* stmt)
	{
		WriteLine(";");
		Write("Ci.clearArray(");
		Write(stmt->name);
		Write(", ");
		Write(stmt->initial_value);
		Write(')');
		this->UsesClearArrayMethod = true;
	}

	virtual void VisitStmt(CiVar* stmt)
	{
		Write("var ");
		Write(stmt->name);
		WriteInit(stmt->type);
		if (stmt->initial_value != NULL) {
			if (dynamic_cast<CiArrayStorageType*>(stmt->type))
				WriteInitArrayStorageVar(stmt);
			else {
				Write(" = ");
				Write(stmt->initial_value);
			}
		}
	}

	virtual void VisitStmt(CiThrow* stmt)
	{
		Write("throw ");
		Write(stmt->message);
		WriteLine(";");
	}

	void Write(CiMethod* method)
	{
		if (method->call_type == AbstractCallType)
			return;
		WriteLine();
		Write(method->class_->name);
		Write('.');
		if (method->call_type != StaticCallType)
			Write("prototype.");
		WriteCamelCase(method->name);
		Write(" = function(");
		bool first = true;
		for(int i = 0; i < method->signature->params.GetCount(); i++) {
			CiParam* param = method->signature->params[i];
			if (first)
				first = false;
			else
				Write(", ");
			Write(param->name);
		}
		Write(") ");
		if (dynamic_cast<CiBlock*>(method->body))
			Write(method->body);
		else {
			OpenBlock();
			Write(method->body);
			CloseBlock();
		}
	}

	void Write(CiConst* konst)
	{
		WriteName(konst);
		Write(" = ");
		WriteConst(konst->value);
		WriteLine(";");
	}

	void Write(CiClass* klass)
	{
		// topological sorting of class hierarchy
		if (klass->write_status == Done)
			return;
		if (klass->write_status == InProgress)
			throw ResolveException("Circular dependency for class " + klass->name);
		klass->write_status = InProgress;
		if (klass->base_class != NULL)
			Write(klass->base_class);
		klass->write_status = Done;

		this->current_class = klass;
		WriteLine();
		Write(klass->documentation);
		Write("function ");
		Write(klass->name);
		WriteLine("()");
		OpenBlock();
		for(int i = 0; i < klass->members->GetCount(); i++) {
			CiSymbol* member = klass->members->Get(i);
			CiField* f = dynamic_cast<CiField*>(member);
			if (f)
				Write(f);
		}
		if (klass->constructor != NULL)
			SourceGenerator::Write(dynamic_cast<CiBlock*>(klass->constructor->body)->statements);
		CloseBlock();
		if (klass->base_class != NULL) {
			Write(klass->name);
			Write(".prototype = new ");
			Write(klass->base_class->name);
			WriteLine("();");
		}
		for(int i = 0; i < klass->members->GetCount(); i++) {
			CiSymbol* member = klass->members->Get(i);
			CiMethod* m = dynamic_cast<CiMethod*>(member);
			CiConst* c = dynamic_cast<CiConst*>(member);
			if (m)
				Write(m);
			else if (c && member->visibility == PublicVisib)
				Write(c);
		}
		
		for(int i = 0; i < klass->const_arrays.GetCount(); i++)
			Write(klass->const_arrays[i]);
		
		for(int i = 0; i < klass->binary_resources.GetCount(); i++) {
			CiBinaryResource* resource = klass->binary_resources[i];
			
			WriteName(resource);
			Write(" = ");
			Write("{");
			SourceGenerator::WriteContent(resource->content);
			Write("}");
			WriteLine(";");
		}
		this->current_class = NULL;
	}

	void WriteBuiltins()
	{
		Vector<Vector<String> > code;
		if (this->UsesSubstringMethod) {
			auto& c = code.Add();
			c.Add("substring : function(s, offset, length)");
			c.Add("return s.substring(offset, offset + length);");
		}
		if (this->UsesCopyArrayMethod) {
			auto& c = code.Add();
			c.Add("copyArray : function(sa, soffset, da, doffset, length)");
			c.Add("for (var i = 0; i < length; i++)");
			c.Add("\tda[doffset + i] = sa[soffset + i];");
		}
		if (this->UsesBytesToStringMethod) {
			auto& c = code.Add();
			c.Add("bytesToString : function(a, offset, length)");
			c.Add("var s = \"\";");
			c.Add("for (var i = 0; i < length; i++)");
			c.Add("\ts += String.fromCharCode(a[offset + i]);");
			c.Add("return s;");
		}
		if (this->UsesClearArrayMethod) {
			auto& c = code.Add();
			c.Add("clearArray : function(a, value)");
			c.Add("for (var i = 0; i < a.length; i++)");
			c.Add("\ta[i] = value;");
		}
		if (code.GetCount() > 0) {
			WriteLine("var Ci = {");
			this->indent++;
			for (int i = 0; ; ) {
				Vector<String>& lines = code[i];
				Write(lines[0]);
				WriteLine(" {");
				this->indent++;
				for (int j = 1; j < lines.GetCount(); j++)
					WriteLine(lines[j]);
				this->indent--;
				Write('}');
				if (++i >= code.GetCount())
					break;
				WriteLine(",");
			}
			WriteLine();
			this->indent--;
			WriteLine("};");
		}
	}

	virtual void Write(CiProgram* prog)
	{
		CreateFile(this->output_file);
		this->UsesSubstringMethod = false;
		this->UsesCopyArrayMethod = false;
		this->UsesBytesToStringMethod = false;
		this->UsesClearArrayMethod = false;
		for(int i = 0; i < prog->globals->GetCount(); i++) {
			CiSymbol* s = prog->globals->Get(i);
			CiClass* c = dynamic_cast<CiClass*>(s);
			if (c)
				c->write_status = NotYet;
		}
		for(int i = 0; i < prog->globals->GetCount(); i++) {
			CiSymbol* s = prog->globals->Get(i);
			CiEnum* e = dynamic_cast<CiEnum*>(s);
			CiClass* c = dynamic_cast<CiClass*>(s);
			if (e)
				Write(e);
			else if (c)
				Write(c);
		}
		WriteBuiltins();
		CloseFile();
	}
};

}
