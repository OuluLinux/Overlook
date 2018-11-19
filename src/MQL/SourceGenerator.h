#ifndef _MQL_SourceGenerator_h_
#define _MQL_SourceGenerator_h_

namespace Ci
{

struct SourceGenerator : public ICiStatementVisitor
{
	String output_file;
	TextWriter writer;
	int indent = 0;
	bool at_line_start = true;


	void CreateFileWriter(String filename)
	{
		writer.OpenFile(filename);
	}

	void StartLine()
	{
		if (this->at_line_start) {
			for (int i = 0; i < this->indent; i++)
				writer.Write('\t');
			this->at_line_start = false;
		}
	}

	void Write(char c)
	{
		StartLine();
		writer.Write(c);
	}

	void Write(String s)
	{
		StartLine();
		writer.Write(s);
	}

	void Write(int i)
	{
		StartLine();
		writer.Write(i);
	}

	void WriteLowercase(String s)
	{
		writer.Write(ToLower(s.ToWString()).ToString());
	}

	static String ToCamelCase(String s)
	{
		String s2;
		s2.Cat(ToLower(s[0]));
		s2.Cat(s.Mid(1));
		return s2;
	}

	void WriteCamelCase(String s)
	{
		StartLine();
		writer.Write(ToLower(s[0]));
		writer.Write(s.Mid(1));
	}

	void WriteUppercaseWithUnderscores(String s)
	{
		StartLine();
		bool first = true;
		for(int i = 0; i < s.GetCount(); i++) {
			char c = s[i];
			
			if (IsUpper(c) && !first) {
				writer.Write('_');
				writer.Write(c);
			}
			else
				writer.Write(ToUpper(c));
			first = false;
		}
	}

	void WriteLowercaseWithUnderscores(String s)
	{
		StartLine();
		bool first = true;
		for(int i = 0; i < s.GetCount(); i++) {
			char c = s[i];
			
			if (IsUpper(c)) {
				if (!first)
					writer.Write('_');
				writer.Write(ToLower(c));
			}
			else
				writer.Write(c);
			first = false;
		}
	}

	void WriteLine()
	{
		writer.WriteLine();
		this->at_line_start = true;
	}

	void WriteLine(String s)
	{
		StartLine();
		writer.WriteLine(s);
		this->at_line_start = true;
	}

	/*void WriteLine(String& format, const Vector<Object*>& args)
	{
		StartLine();
		writer.WriteLine(format, args);
		this->at_line_start = true;
	}*/

	
	void WriteDoc(String text)
	{
		for(int i = 0; i < text.GetCount(); i++) {
			char c = text[i];
			
			switch (c) {
			case '&': Write("&amp;"); break;
			case '<': Write("&lt;"); break;
			case '>': Write("&gt;"); break;
			case '\n': WriteLine(); Write(" * "); break;
			default: Write(c); break;
			}
		}
	}

	void Write(CiDocPara* para)
	{
		for(int i = 0; i < para->children.GetCount(); i++) {
			CiDocInline* di = para->children[i];
			CiDocText* text = dynamic_cast<CiDocText*>(di);
			if (text != NULL) {
				WriteDoc(text->text);
				continue;
			}
			CiDocCode* code = dynamic_cast<CiDocCode*>(di);
			if (code != NULL) {
				Write("<code>");
				WriteDoc(code->text);
				Write("</code>");
				continue;
			}
			throw new ArgumentException("Invalid CiDocInline");
		}
	}

	void Write(CiDocBlock* block)
	{
		CiDocList* list = dynamic_cast<CiDocList*>(block);
		if (list != NULL) {
			WriteLine();
			WriteLine(" * <ul>");
			for(int i = 0; i < list->items.GetCount(); i++) {
				CiDocPara* item = list->items[i];
				Write(" * <li>");
				Write(item);
				WriteLine("</li>");
			}
			WriteLine(" * </ul>");
			Write(" * ");
			return;
		}
		Write(dynamic_cast<CiDocPara*>(block));
	}

	void WriteDontClose(CiCodeDoc* doc)
	{
		WriteLine("/**");
		Write(" * ");
		Write(doc->summary);
		if (doc->details.GetCount() > 0) {
			WriteLine();
			Write(" * ");
			for(int i = 0; i < doc->details.GetCount(); i++) {
				Write(doc->details[i]);
			}
		}
		WriteLine();
	}

	virtual void Write(CiCodeDoc* doc)
	{
		if (doc != NULL) {
			WriteDontClose(doc);
			WriteLine(" */");
		}
	}

	void WriteDoc(CiMethod* method)
	{
		if (method->documentation != NULL) {
			WriteDontClose(method->documentation);
			for(int i = 0; i < method->signature->params.GetCount(); i++) {
				CiParam* param = method->signature->params[i];
				if (param->documentation != NULL) {
					Write(" * @param ");
					Write(param->name);
					Write(' ');
					Write(param->documentation->summary);
					WriteLine();
				}
			}
			WriteLine(" */");
		}
	}


	virtual void WriteBanner()
	{
		WriteLine("// Generated automatically with \"cito\". Do not edit.");
	}

	void CreateFile(String filename)
	{
		CreateFileWriter(filename);
		WriteBanner();
	}

	void CloseFile()
	{
		writer.Close();
	}

	void OpenBlock()
	{
		WriteLine("{");
		this->indent++;
	}

	void CloseBlock()
	{
		this->indent--;
		WriteLine("}");
	}

	void WriteInitializer(CiArrayType* type)
	{
		CiClassStorageType* cst = dynamic_cast<CiClassStorageType*>(type->element_type);
		if (cst) {
			CiArrayStorageType* storageType = dynamic_cast<CiArrayStorageType*>(type);
			if (storageType != NULL) {
				int len = storageType->length;
				if (len > 0) {
					Write("[] { ");
					for (int i = 0; i < len; i++) {
						if (i > 0)
							Write(", ");
						WriteNew(type->element_type);
					}
					Write(" }");
					return;
				}
			}
		}
		for (; type != NULL; type = dynamic_cast<CiArrayType*>(type->element_type)) {
			Write('[');
			CiArrayStorageType* storageType = dynamic_cast<CiArrayStorageType*>(type);
			if (storageType != NULL) {
				if (storageType->length_expr != NULL)
					Write(storageType->length_expr);
				else
					Write(storageType->length);
			}
			Write(']');
		}
	}

	void WriteContent(const Vector<Object*>& array)
	{
		for (int i = 0; i < array.GetCount(); i++) {
			if (i > 0) {
				if (i % 16 == 0) {
					WriteLine(",");
					Write('\t');
				}
				else
					Write(", ");
			}
			WriteConst(array[i]);
		}
	}

	void WriteContent(const Vector<byte>& array)
	{
		for (int i = 0; i < array.GetCount(); i++) {
			if (i > 0) {
				if (i % 16 == 0) {
					WriteLine(",");
					Write('\t');
				}
				else
					Write(", ");
			}
			IntStr(array[i]);
		}
	}

	virtual void WriteConst(Object* value)
	{
		if (value->type == O_BOOL)
			Write(value->b ? "true" : "false");
		else if (value->type == O_BYTE)
			Write(IntStr(value->byt));
		else if (value->type == O_INT)
			Write(IntStr(value->i));
		else if (value->type == O_STRING) {
			Write('"');
			for(int i = 0; i < value->s.GetCount(); i++) {
				char c = value->s[i];
				switch (c) {
					case '\t': Write("\\t"); break;
					case '\r': Write("\\r"); break;
					case '\n': Write("\\n"); break;
					case '\\': Write("\\\\"); break;
					case '\"': Write("\\\""); break;
					default: Write(c); break;
				}
			}
			Write('"');
		}
		else if (value->type == O_ENUM) {
			CiEnumValue* ev = dynamic_cast<CiEnumValue*>(value);
			Write(ev->type->name);
			Write('.');
			Write(ev->name);
		}
		else if (value->objs.GetCount()) {
			Write("{ ");
			WriteContent(value->objs);
			Write(" }");
		}
		else if (value == NULL || value->type == O_NULL)
			Write("null");
		else
			throw new ArgumentException(value->ToString());
	}

	virtual CiPriority GetPriority(CiExpr* expr)
	{
		CiConstExpr* ce = dynamic_cast<CiConstExpr*>(expr);
		CiConstAccess* ca = dynamic_cast<CiConstAccess*>(expr);
		CiVarAccess* va = dynamic_cast<CiVarAccess*>(expr);
		CiFieldAccess* fa = dynamic_cast<CiFieldAccess*>(expr);
		CiPropertyAccess* pa = dynamic_cast<CiPropertyAccess*>(expr);
		CiArrayAccess* aa = dynamic_cast<CiArrayAccess*>(expr);
		CiMethodCall* mc = dynamic_cast<CiMethodCall*>(expr);
		CiBinaryResourceExpr* bre = dynamic_cast<CiBinaryResourceExpr*>(expr);
		CiNewExpr* ne = dynamic_cast<CiNewExpr*>(expr);
		CiUnaryExpr* ue = dynamic_cast<CiUnaryExpr*>(expr);
		CiCondNotExpr* cne = dynamic_cast<CiCondNotExpr*>(expr);
		CiPostfixExpr* pfe = dynamic_cast<CiPostfixExpr*>(expr);
		CiCoercion* c = dynamic_cast<CiCoercion*>(expr);
		CiBinaryExpr* be = dynamic_cast<CiBinaryExpr*>(expr);
		
		if (ce
		 || ca
		 || va
		 || fa
		 || pa
		 || aa
		 || mc
		 || bre
		 || ne) // ?
			return Postfix;
		if (ue
		 || cne
		 || pfe) // ?
			return Prefix;
		if (c)
			return GetPriority(dynamic_cast<CiExpr*>(c->inner));
		if (be) {
			switch (be->op) {
			case Asterisk:
			case Slash:
			case Mod:
				return Multiplicative;
			case Plus:
			case Minus:
				return Additive;
			case ShiftLeft:
			case ShiftRight:
				return Shift;
			case Less:
			case LessOrEqual:
			case Greater:
			case GreaterOrEqual:
				return Ordering;
			case Equal:
			case NotEqual:
				return Equality;
			case AndToken:
				return AndPrior;
			case XorToken:
				return XorPrior;
			case OrToken:
				return OrPrior;
			case CondAndToken:
				return CondAndPrior;
			case CondOrToken:
				return CondOrPrior;
			default:
				throw new ArgumentException("Priority " + IntStr(be->op));
			}
		}
		if (dynamic_cast<CiCondExpr*>(expr))
			return CondExpr;
		throw new ArgumentException(expr->Type()->name);
	}

	void WriteChild(CiPriority parentPriority, CiExpr* child)
	{
		if (GetPriority(child) < parentPriority) {
			Write('(');
			Write(child);
			Write(')');
		}
		else
			Write(child);
	}

	void WriteChild(CiExpr* parent, CiExpr* child)
	{
		WriteChild(GetPriority(parent), child);
	}

	void WriteNonAssocChild(CiPriority parentPriority, CiExpr* child)
	{
		if (GetPriority(child) <= parentPriority) {
			Write('(');
			Write(child);
			Write(')');
		}
		else
			Write(child);
	}

	void WriteNonAssocChild(CiExpr* parent, CiExpr* child)
	{
		WriteNonAssocChild(GetPriority(parent), child);
	}

	void WriteSum(CiExpr* left, CiExpr* right)
	{
		Write(new CiBinaryExpr(left, Plus, right));
	}

	virtual void WriteName(CiConst* konst)
	{
		Write(!konst->global_name.IsEmpty() ? konst->global_name : konst->name);
	}

	virtual void Write(CiVarAccess* expr)
	{
		Write(expr->var->name);
	}

	virtual void Write(CiFieldAccess* expr)
	{
		WriteChild(expr, expr->obj);
		Write('.');
		Write(expr->field->name);
	}

	virtual void Write(CiPropertyAccess* expr) = 0;

	virtual void Write(CiArrayAccess* expr)
	{
		WriteChild(expr, expr->array);
		Write('[');
		Write(expr->index);
		Write(']');
	}

	virtual void WriteName(CiMethod* method)
	{
		Write(method->name);
	}

	virtual void WriteDelegateCall(CiExpr* expr)
	{
		Write(expr);
	}

	void WriteMulDiv(CiPriority firstPriority, CiMethodCall* expr)
	{
		WriteChild(firstPriority, expr->obj);
		Write(" * ");
		WriteChild(Multiplicative, expr->arguments[0]);
		Write(" / ");
		WriteNonAssocChild(Multiplicative, expr->arguments[1]);
		Write(')');
	}

	void WriteArguments(CiMethodCall* expr)
	{
		Write('(');
		bool first = true;
		for(int i = 0; i < expr->arguments.GetCount(); i++) {
			CiExpr* arg = expr->arguments[i];
			if (first)
				first = false;
			else
				Write(", ");
			Write(arg);
		}
		Write(')');
	}

	virtual void Write(CiMethodCall* expr)
	{
		if (expr->method != NULL) {
			if (expr->obj != NULL)
				Write(expr->obj);
			else
				Write(expr->method->class_->name);
			Write('.');
			WriteName(expr->method);
		}
		else
			WriteDelegateCall(expr->obj);
		WriteArguments(expr);
	}

	void Write(CiUnaryExpr* expr)
	{
		switch (expr->op) {
			case Increment: Write("++"); break;
			case Decrement: Write("--"); break;
			case Minus: Write('-'); break;
			case Not: Write('~'); break;
			default: throw new ArgumentException("Op " + IntStr(expr->op));
		}
		WriteChild(expr, expr->inner);
	}

	void Write(CiCondNotExpr* expr)
	{
		Write('!');
		WriteChild(expr, expr->inner);
	}

	void Write(CiPostfixExpr* expr)
	{
		WriteChild(expr, expr->inner);
		switch (expr->op) {
			case Increment: Write("++"); break;
			case Decrement: Write("--"); break;
			default: throw new ArgumentException("op " + IntStr(expr->op));
		}
	}

	void WriteOp(CiBinaryExpr* expr)
	{
		Write(' ');
		Write(expr->OpString());
		Write(' ');
	}

	virtual void Write(CiBinaryExpr* expr)
	{
		WriteChild(expr, expr->left);
		switch (expr->op) {
			case Plus:
			case Asterisk:
			case Less:
			case LessOrEqual:
			case Greater:
			case GreaterOrEqual:
			case Equal:
			case NotEqual:
			case AndToken:
			case OrToken:
			case XorToken:
			case CondAndToken:
			case CondOrToken:
				WriteOp(expr);
				WriteChild(expr, expr->right);
				break;
			case Minus:
			case Slash:
			case Mod:
			case ShiftLeft:
			case ShiftRight:
				WriteOp(expr);
				WriteNonAssocChild(expr, expr->right);
				break;
			default:
				throw new ArgumentException("op " + IntStr(expr->op));
		}
	}

	virtual void Write(CiCondExpr* expr)
	{
		WriteNonAssocChild(expr, expr->cond);
		Write(" ? ");
		WriteChild(expr, expr->on_true);
		Write(" : ");
		WriteChild(expr, expr->on_false);
	}

	virtual void WriteName(CiBinaryResource* resource)
	{
		Write("CiBinaryResource_");
		for(int i = 0; i < resource->name.GetCount(); i++) {
			char c = resource->name[i];
			Write(IsLetter(c) ? c : '_');
		}
	}

	virtual void Write(CiBinaryResourceExpr* expr)
	{
		WriteName(expr->resource);
	}

	virtual void WriteNew(CiType* type) = 0;

	void WriteInline(CiMaybeAssign* expr)
	{
		CiExpr* e = dynamic_cast<CiExpr*>(expr);
		if (e)
			Write(e);
		else
			VisitStmt(dynamic_cast<CiAssign*>(expr));
	}

	virtual void Write(CiCoercion* expr)
	{
		WriteInline(expr->inner);
	}

	void Write(CiExpr* expr)
	{
		CiConstExpr* ce = dynamic_cast<CiConstExpr*>(expr);
		CiConstAccess* ca = dynamic_cast<CiConstAccess*>(expr);
		CiVarAccess* va = dynamic_cast<CiVarAccess*>(expr);
		CiFieldAccess* fa = dynamic_cast<CiFieldAccess*>(expr);
		CiPropertyAccess* pa = dynamic_cast<CiPropertyAccess*>(expr);
		CiArrayAccess* aa = dynamic_cast<CiArrayAccess*>(expr);
		CiMethodCall* mc = dynamic_cast<CiMethodCall*>(expr);
		CiUnaryExpr* ue = dynamic_cast<CiUnaryExpr*>(expr);
		CiCondNotExpr* cne = dynamic_cast<CiCondNotExpr*>(expr);
		CiPostfixExpr* pfe = dynamic_cast<CiPostfixExpr*>(expr);
		CiBinaryExpr* be = dynamic_cast<CiBinaryExpr*>(expr);
		CiCondExpr* cde = dynamic_cast<CiCondExpr*>(expr);
		CiBinaryResourceExpr* bre = dynamic_cast<CiBinaryResourceExpr*>(expr);
		CiNewExpr* ne = dynamic_cast<CiNewExpr*>(expr);
		CiCoercion* c = dynamic_cast<CiCoercion*>(expr);
		if (ce)
			WriteConst(ce->value);
		else if (ca)
			WriteName(ca->const_);
		else if (va)
			Write(va);
		else if (fa)
			Write(fa);
		else if (pa)
			Write(pa);
		else if (aa)
			Write(aa);
		else if (mc)
			Write(mc);
		else if (ue)
			Write(ue);
		else if (cne)
			Write(cne);
		else if (pfe)
			Write(pfe);
		else if (be)
			Write(be);
		else if (cde)
			Write(cde);
		else if (bre)
			Write(bre);
		else if (ne)
			WriteNew(ne->new_type);
		else if (c)
			Write(c);
		else
			throw new ArgumentException(expr->ToString());
	}

	void Write(const Vector<ICiStatement*>& statements, int length)
	{
		for (int i = 0; i < length; i++)
			Write(statements[i]);
	}

	virtual void Write(const Vector<ICiStatement*>& statements)
	{
		Write(statements, statements.GetCount());
	}

	virtual void VisitStmt(CiBlock* block)
	{
		OpenBlock();
		Write(block->statements);
		CloseBlock();
	}

	virtual void WriteChild(ICiStatement* stmt)
	{
		CiBlock* b = dynamic_cast<CiBlock*>(stmt);
		if (b) {
			Write(' ');
			Write(b);
		}
		else {
			WriteLine();
			this->indent++;
			Write(stmt);
			this->indent--;
		}
	}

	virtual void VisitStmt(CiExpr* expr)
	{
		Write(expr);
	}

	virtual void VisitStmt(CiVar* stmt) = 0;

	virtual void VisitStmt(CiAssign* assign)
	{
		Write(assign->target);
		switch (assign->op) {
			case Assign: Write(" = "); break;
			case AddAssign: Write(" += "); break;
			case SubAssign: Write(" -= "); break;
			case MulAssign: Write(" *= "); break;
			case DivAssign: Write(" /= "); break;
			case ModAssign: Write(" %= "); break;
			case ShiftLeftAssign: Write(" <<= "); break;
			case ShiftRightAssign: Write(" >>= "); break;
			case AndAssign: Write(" &= "); break;
			case OrAssign: Write(" |= "); break;
			case XorAssign: Write(" ^= "); break;
			default: throw new ArgumentException("op " + IntStr(assign->op));
		}
		WriteInline(assign->source);
	}

	virtual void VisitStmt(CiDelete* stmt)
	{
		// do nothing - assume automatic garbage collector
	}

	virtual void VisitStmt(CiBreak* stmt)
	{
		WriteLine("break;");
	}

	virtual void VisitStmt(CiConst* stmt)
	{
	}

	virtual void VisitStmt(CiContinue* stmt)
	{
		WriteLine("continue;");
	}

	virtual void VisitStmt(CiDoWhile* stmt)
	{
		Write("do");
		WriteChild(stmt->body);
		Write("while (");
		Write(stmt->cond);
		WriteLine(");");
	}

	virtual void VisitStmt(CiFor* stmt)
	{
		Write("for (");
		if (stmt->init != NULL)
			stmt->init->Accept(this);
		Write(';');
		if (stmt->cond != NULL) {
			Write(' ');
			Write(stmt->cond);
		}
		Write(';');
		if (stmt->advance != NULL) {
			Write(' ');
			stmt->advance->Accept(this);
		}
		Write(')');
		WriteChild(stmt->body);
	}

	virtual void WriteIfOnTrue(CiIf* stmt)
	{
		WriteChild(stmt->on_true);
	}

	virtual void VisitStmt(CiIf* stmt)
	{
		Write("if (");
		Write(stmt->cond);
		Write(')');
		WriteIfOnTrue(stmt);
		if (stmt->on_false != NULL) {
			Write("else");
			CiIf* i = dynamic_cast<CiIf*>(stmt->on_false);
			if (i) {
				Write(' ');
				Write(i);
			}
			else
				WriteChild(stmt->on_false);
		}
	}

	virtual void VisitStmt(CiNativeBlock* statement)
	{
		Write(statement->content);
	}

	virtual void VisitStmt(CiReturn* stmt)
	{
		if (stmt->value == NULL)
			WriteLine("return;");
		else {
			Write("return ");
			Write(stmt->value);
			WriteLine(";");
		}
	}

	virtual void StartSwitch(CiSwitch* stmt)
	{
	}

	virtual void StartCase(ICiStatement* stmt)
	{
	}

	virtual void WriteFallthrough(CiExpr* expr)
	{
	}

	virtual void EndSwitch(CiSwitch* stmt)
	{
	}

	virtual void VisitStmt(CiSwitch* stmt)
	{
		Write("switch (");
		Write(stmt->value);
		WriteLine(") {");
		StartSwitch(stmt);
		for(int i = 0; i < stmt->cases.GetCount(); i++) {
			CiCase* kase = stmt->cases[i];
			for(int j = 0; j < kase->values.GetCount(); j++) {
				Object* value = kase->values[j];
				Write("case ");
				WriteConst(value);
				WriteLine(":");
			}
			this->indent++;
			StartCase(kase->body[0]);
			Write(kase->body);
			if (kase->fallthrough_to)
				WriteFallthrough(kase->fallthrough_to);
			this->indent--;
		}
		if (stmt->default_body.GetCount()) {
			WriteLine("default:");
			this->indent++;
			StartCase(stmt->default_body[0]);
			Write(stmt->default_body);
			this->indent--;
		}
		EndSwitch(stmt);
		WriteLine("}");
	}

	virtual void VisitStmt(CiThrow* stmt) = 0;

	virtual void VisitStmt(CiWhile* stmt)
	{
		Write("while (");
		Write(stmt->cond);
		Write(')');
		WriteChild(stmt->body);
	}

	void Write(ICiStatement* stmt)
	{
		stmt->Accept(this);
		CiMaybeAssign* ma = dynamic_cast<CiMaybeAssign*>(stmt);
		CiVar* v = dynamic_cast<CiVar*>(stmt);
		if ((ma || v) && !this->at_line_start)
			WriteLine(";");
	}

	void WriteBody(CiMethod* method)
	{
		if (method->call_type == AbstractCallType)
			WriteLine(";");
		else {
			WriteLine();
			CiBlock* b = dynamic_cast<CiBlock*>(method->body);
			if (b)
				Write(method->body);
			else {
				OpenBlock();
				Write(method->body);
				CloseBlock();
			}
		}
	}

	void OpenClass(bool isAbstract, CiClass* klass, String extendsClause)
	{
		if (isAbstract)
			Write("abstract ");
		Write("class ");
		Write(klass->name);
		if (klass->base_class != NULL) {
			Write(extendsClause);
			Write(klass->base_class->name);
		}
		WriteLine();
		OpenBlock();
	}

	virtual void Write(CiProgram* prog) = 0;
};

}

#endif
