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
		if (this.at_line_start) {
			for (int i = 0; i < this.Indent; i++)
				this.Writer.Write('\t');
			this.at_line_start = false;
		}
	}

	void Write(char c)
	{
		StartLine();
		this.Writer.Write(c);
	}

	void Write(String s)
	{
		StartLine();
		this.Writer.Write(s);
	}

	void Write(int i)
	{
		StartLine();
		this.Writer.Write(i);
	}

	void WriteLowercase(String s)
	{
		foreach (char c in s)
			this.Writer.Write(char.ToLowerInvariant(c));
	}

	static String ToCamelCase(String s)
	{
		return char.ToLowerInvariant(s[0]) + s.Substring(1);
	}

	void WriteCamelCase(String s)
	{
		StartLine();
		this.Writer.Write(char.ToLowerInvariant(s[0]));
		this.Writer.Write(s.Substring(1));
	}

	void WriteUppercaseWithUnderscores(String s)
	{
		StartLine();
		bool first = true;
		foreach (char c in s) {
			if (char.IsUpper(c) && !first) {
				this.Writer.Write('_');
				this.Writer.Write(c);
			}
			else
				this.Writer.Write(char.ToUpperInvariant(c));
			first = false;
		}
	}

	void WriteLowercaseWithUnderscores(String s)
	{
		StartLine();
		bool first = true;
		foreach (char c in s) {
			if (char.IsUpper(c)) {
				if (!first)
					this.Writer.Write('_');
				this.Writer.Write(char.ToLowerInvariant(c));
			}
			else
				this.Writer.Write(c);
			first = false;
		}
	}

	void WriteLine()
	{
		this.Writer.WriteLine();
		this.at_line_start = true;
	}

	void WriteLine(String s)
	{
		StartLine();
		this.Writer.WriteLine(s);
		this.at_line_start = true;
	}

	void WriteLine(String format, params object[] args)
	{
		StartLine();
		this.Writer.WriteLine(format, args);
		this.at_line_start = true;
	}

	
	void WriteDoc(string text)
	{
		foreach (char c in text) {
			switch (c) {
			case '&': Write("&amp;"); break;
			case '<': Write("&lt;"); break;
			case '>': Write("&gt;"); break;
			case '\n': WriteLine(); Write(" * "); break;
			default: Write(c); break;
			}
		}
	}

	void Write(CiDocPara para)
	{
		foreach (CiDocInline inline in para.Children) {
			CiDocText text = inline as CiDocText;
			if (text != null) {
				WriteDoc(text.Text);
				continue;
			}
			CiDocCode code = inline as CiDocCode;
			if (code != null) {
				Write("<code>");
				WriteDoc(code.Text);
				Write("</code>");
				continue;
			}
			throw new ArgumentException(inline.GetType().Name);
		}
	}

	void Write(CiDocBlock block)
	{
		CiDocList list = block as CiDocList;
		if (list != null) {
			WriteLine();
			WriteLine(" * <ul>");
			foreach (CiDocPara item in list.Items) {
				Write(" * <li>");
				Write(item);
				WriteLine("</li>");
			}
			WriteLine(" * </ul>");
			Write(" * ");
			return;
		}
		Write((CiDocPara) block);
	}

	void WriteDontClose(CiCodeDoc doc)
	{
		WriteLine("/**");
		Write(" * ");
		Write(doc.Summary);
		if (doc.Details.Length > 0) {
			WriteLine();
			Write(" * ");
			foreach (CiDocBlock block in doc.Details)
				Write(block);
		}
		WriteLine();
	}

	protected virtual void Write(CiCodeDoc doc)
	{
		if (doc != null) {
			WriteDontClose(doc);
			WriteLine(" */");
		}
	}

	void WriteDoc(CiMethod method)
	{
		if (method.Documentation != null) {
			WriteDontClose(method.Documentation);
			foreach (CiParam param in method.Signature.Params) {
				if (param.Documentation != null) {
					Write(" * @param ");
					Write(param.Name);
					Write(' ');
					Write(param.Documentation.Summary);
					WriteLine();
				}
			}
			WriteLine(" */");
		}
	}


	protected virtual void WriteBanner()
	{
		WriteLine("// Generated automatically with \"cito\". Do not edit.");
	}

	void CreateFile(string filename)
	{
		this.Writer = CreateFileWriter(filename);
		WriteBanner();
	}

	void CloseFile()
	{
		this.Writer.Close();
	}

	void OpenBlock()
	{
		WriteLine("{");
		this.Indent++;
	}

	void CloseBlock()
	{
		this.Indent--;
		WriteLine("}");
	}

	void WriteInitializer(CiArrayType type)
	{
		if (type.ElementType is CiClassStorageType) {
			CiArrayStorageType storageType = type as CiArrayStorageType;
			if (storageType != null) {
				int len = storageType.Length;
				if (len > 0) {
					Write("[] { ");
					for (int i = 0; i < len; i++) {
						if (i > 0)
							Write(", ");
						WriteNew(type.ElementType);
					}
					Write(" }");
					return;
				}
			}
		}
		for (; type != null; type = type.ElementType as CiArrayType) {
			Write('[');
			CiArrayStorageType storageType = type as CiArrayStorageType;
			if (storageType != null) {
				if (storageType.LengthExpr != null)
					Write(storageType.LengthExpr);
				else
					Write(storageType.Length);
			}
			Write(']');
		}
	}

	void WriteContent(Array array)
	{
		for (int i = 0; i < array.Length; i++) {
			if (i > 0) {
				if (i % 16 == 0) {
					WriteLine(",");
					Write('\t');
				}
				else
					Write(", ");
			}
			WriteConst(array.GetValue(i));
		}
	}

	protected virtual void WriteConst(object value)
	{
		if (value is bool)
			Write((bool) value ? "true" : "false");
		else if (value is byte)
			Write((byte) value);
		else if (value is int)
			Write((int) value);
		else if (value is string) {
			Write('"');
			foreach (char c in (string) value) {
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
		else if (value is CiEnumValue) {
			CiEnumValue ev = (CiEnumValue) value;
			Write(ev.Type.Name);
			Write('.');
			Write(ev.Name);
		}
		else if (value is Array) {
			Write("{ ");
			WriteContent((Array) value);
			Write(" }");
		}
		else if (value == null)
			Write("null");
		else
			throw new ArgumentException(value.ToString());
	}

	protected virtual CiPriority GetPriority(CiExpr expr)
	{
		if (expr is CiConstExpr
		 || expr is CiConstAccess
		 || expr is CiVarAccess
		 || expr is CiFieldAccess
		 || expr is CiPropertyAccess
		 || expr is CiArrayAccess
		 || expr is CiMethodCall
		 || expr is CiBinaryResourceExpr
		 || expr is CiNewExpr) // ?
			return CiPriority.Postfix;
		if (expr is CiUnaryExpr
		 || expr is CiCondNotExpr
		 || expr is CiPostfixExpr) // ?
			return CiPriority.Prefix;
		if (expr is CiCoercion)
			return GetPriority((CiExpr) ((CiCoercion) expr).Inner);
		if (expr is CiBinaryExpr) {
			switch (((CiBinaryExpr) expr).Op) {
			case CiToken.Asterisk:
			case CiToken.Slash:
			case CiToken.Mod:
				return CiPriority.Multiplicative;
			case CiToken.Plus:
			case CiToken.Minus:
				return CiPriority.Additive;
			case CiToken.ShiftLeft:
			case CiToken.ShiftRight:
				return CiPriority.Shift;
			case CiToken.Less:
			case CiToken.LessOrEqual:
			case CiToken.Greater:
			case CiToken.GreaterOrEqual:
				return CiPriority.Ordering;
			case CiToken.Equal:
			case CiToken.NotEqual:
				return CiPriority.Equality;
			case CiToken.And:
				return CiPriority.And;
			case CiToken.Xor:
				return CiPriority.Xor;
			case CiToken.Or:
				return CiPriority.Or;
			case CiToken.CondAnd:
				return CiPriority.CondAnd;
			case CiToken.CondOr:
				return CiPriority.CondOr;
			default:
				throw new ArgumentException(((CiBinaryExpr) expr).Op.ToString());
			}
		}
		if (expr is CiCondExpr)
			return CiPriority.CondExpr;
		throw new ArgumentException(expr.GetType().Name);
	}

	void WriteChild(CiPriority parentPriority, CiExpr child)
	{
		if (GetPriority(child) < parentPriority) {
			Write('(');
			Write(child);
			Write(')');
		}
		else
			Write(child);
	}

	void WriteChild(CiExpr parent, CiExpr child)
	{
		WriteChild(GetPriority(parent), child);
	}

	void WriteNonAssocChild(CiPriority parentPriority, CiExpr child)
	{
		if (GetPriority(child) <= parentPriority) {
			Write('(');
			Write(child);
			Write(')');
		}
		else
			Write(child);
	}

	void WriteNonAssocChild(CiExpr parent, CiExpr child)
	{
		WriteNonAssocChild(GetPriority(parent), child);
	}

	void WriteSum(CiExpr left, CiExpr right)
	{
		Write(new CiBinaryExpr { Left = left, Op = CiToken.Plus, Right = right });
	}

	protected virtual void WriteName(CiConst konst)
	{
		Write(konst.GlobalName ?? konst.Name);
	}

	protected virtual void Write(CiVarAccess expr)
	{
		Write(expr.Var.Name);
	}

	protected virtual void Write(CiFieldAccess expr)
	{
		WriteChild(expr, expr.Obj);
		Write('.');
		Write(expr.Field.Name);
	}

	protected abstract void Write(CiPropertyAccess expr);

	protected virtual void Write(CiArrayAccess expr)
	{
		WriteChild(expr, expr.Array);
		Write('[');
		Write(expr.Index);
		Write(']');
	}

	protected virtual void WriteName(CiMethod method)
	{
		Write(method.Name);
	}

	protected virtual void WriteDelegateCall(CiExpr expr)
	{
		Write(expr);
	}

	void WriteMulDiv(CiPriority firstPriority, CiMethodCall expr)
	{
		WriteChild(firstPriority, expr.Obj);
		Write(" * ");
		WriteChild(CiPriority.Multiplicative, expr.Arguments[0]);
		Write(" / ");
		WriteNonAssocChild(CiPriority.Multiplicative, expr.Arguments[1]);
		Write(')');
	}

	void WriteArguments(CiMethodCall expr)
	{
		Write('(');
		bool first = true;
		foreach (CiExpr arg in expr.Arguments)
		{
			if (first)
				first = false;
			else
				Write(", ");
			Write(arg);
		}
		Write(')');
	}

	protected virtual void Write(CiMethodCall expr)
	{
		if (expr.Method != null) {
			if (expr.Obj != null)
				Write(expr.Obj);
			else
				Write(expr.Method.Class.Name);
			Write('.');
			WriteName(expr.Method);
		}
		else
			WriteDelegateCall(expr.Obj);
		WriteArguments(expr);
	}

	void Write(CiUnaryExpr expr)
	{
		switch (expr.Op) {
		case CiToken.Increment: Write("++"); break;
		case CiToken.Decrement: Write("--"); break;
		case CiToken.Minus: Write('-'); break;
		case CiToken.Not: Write('~'); break;
		default: throw new ArgumentException(expr.Op.ToString());
		}
		WriteChild(expr, expr.Inner);
	}

	void Write(CiCondNotExpr expr)
	{
		Write('!');
		WriteChild(expr, expr.Inner);
	}

	void Write(CiPostfixExpr expr)
	{
		WriteChild(expr, expr.Inner);
		switch (expr.Op) {
		case CiToken.Increment: Write("++"); break;
		case CiToken.Decrement: Write("--"); break;
		default: throw new ArgumentException(expr.Op.ToString());
		}
	}

	void WriteOp(CiBinaryExpr expr)
	{
		Write(' ');
		Write(expr.OpString);
		Write(' ');
	}

	protected virtual void Write(CiBinaryExpr expr)
	{
		WriteChild(expr, expr.Left);
		switch (expr.Op) {
		case CiToken.Plus:
		case CiToken.Asterisk:
		case CiToken.Less:
		case CiToken.LessOrEqual:
		case CiToken.Greater:
		case CiToken.GreaterOrEqual:
		case CiToken.Equal:
		case CiToken.NotEqual:
		case CiToken.And:
		case CiToken.Or:
		case CiToken.Xor:
		case CiToken.CondAnd:
		case CiToken.CondOr:
			WriteOp(expr);
			WriteChild(expr, expr.Right);
			break;
		case CiToken.Minus:
		case CiToken.Slash:
		case CiToken.Mod:
		case CiToken.ShiftLeft:
		case CiToken.ShiftRight:
			WriteOp(expr);
			WriteNonAssocChild(expr, expr.Right);
			break;
		default:
			throw new ArgumentException(expr.Op.ToString());
		}
	}

	protected virtual void Write(CiCondExpr expr)
	{
		WriteNonAssocChild(expr, expr.Cond);
		Write(" ? ");
		WriteChild(expr, expr.OnTrue);
		Write(" : ");
		WriteChild(expr, expr.OnFalse);
	}

	protected virtual void WriteName(CiBinaryResource resource)
	{
		Write("CiBinaryResource_");
		foreach (char c in resource.Name)
			Write(CiLexer.IsLetter(c) ? c : '_');
	}

	protected virtual void Write(CiBinaryResourceExpr expr)
	{
		WriteName(expr.Resource);
	}

	protected abstract void WriteNew(CiType type);

	void WriteInline(CiMaybeAssign expr)
	{
		if (expr is CiExpr)
			Write((CiExpr) expr);
		else
			Visit((CiAssign) expr);
	}

	protected virtual void Write(CiCoercion expr)
	{
		WriteInline(expr.Inner);
	}

	void Write(CiExpr expr)
	{
		if (expr is CiConstExpr)
			WriteConst(((CiConstExpr) expr).Value);
		else if (expr is CiConstAccess)
			WriteName(((CiConstAccess) expr).Const);
		else if (expr is CiVarAccess)
			Write((CiVarAccess) expr);
		else if (expr is CiFieldAccess)
			Write((CiFieldAccess) expr);
		else if (expr is CiPropertyAccess)
			Write((CiPropertyAccess) expr);
		else if (expr is CiArrayAccess)
			Write((CiArrayAccess) expr);
		else if (expr is CiMethodCall)
			Write((CiMethodCall) expr);
		else if (expr is CiUnaryExpr)
			Write((CiUnaryExpr) expr);
		else if (expr is CiCondNotExpr)
			Write((CiCondNotExpr) expr);
		else if (expr is CiPostfixExpr)
			Write((CiPostfixExpr) expr);
		else if (expr is CiBinaryExpr)
			Write((CiBinaryExpr) expr);
		else if (expr is CiCondExpr)
			Write((CiCondExpr) expr);
		else if (expr is CiBinaryResourceExpr)
			Write((CiBinaryResourceExpr) expr);
		else if (expr is CiNewExpr)
			WriteNew(((CiNewExpr) expr).NewType);
		else if (expr is CiCoercion)
			Write((CiCoercion) expr);
		else
			throw new ArgumentException(expr.ToString());
	}

	void Write(ICiStatement[] statements, int length)
	{
		for (int i = 0; i < length; i++)
			Write(statements[i]);
	}

	protected virtual void Write(ICiStatement[] statements)
	{
		Write(statements, statements.Length);
	}

	public virtual void Visit(CiBlock block)
	{
		OpenBlock();
		Write(block.Statements);
		CloseBlock();
	}

	protected virtual void WriteChild(ICiStatement stmt)
	{
		if (stmt is CiBlock) {
			Write(' ');
			Write((CiBlock) stmt);
		}
		else {
			WriteLine();
			this.Indent++;
			Write(stmt);
			this.Indent--;
		}
	}

	public virtual void Visit(CiExpr expr)
	{
		Write(expr);
	}

	public abstract void Visit(CiVar stmt);

	public virtual void Visit(CiAssign assign)
	{
		Write(assign.Target);
		switch (assign.Op) {
		case CiToken.Assign: Write(" = "); break;
		case CiToken.AddAssign: Write(" += "); break;
		case CiToken.SubAssign: Write(" -= "); break;
		case CiToken.MulAssign: Write(" *= "); break;
		case CiToken.DivAssign: Write(" /= "); break;
		case CiToken.ModAssign: Write(" %= "); break;
		case CiToken.ShiftLeftAssign: Write(" <<= "); break;
		case CiToken.ShiftRightAssign: Write(" >>= "); break;
		case CiToken.AndAssign: Write(" &= "); break;
		case CiToken.OrAssign: Write(" |= "); break;
		case CiToken.XorAssign: Write(" ^= "); break;
		default: throw new ArgumentException(assign.Op.ToString());
		}
		WriteInline(assign.Source);
	}

	public virtual void Visit(CiDelete stmt)
	{
		// do nothing - assume automatic garbage collector
	}

	public virtual void Visit(CiBreak stmt)
	{
		WriteLine("break;");
	}

	public virtual void Visit(CiConst stmt)
	{
	}

	public virtual void Visit(CiContinue stmt)
	{
		WriteLine("continue;");
	}

	public virtual void Visit(CiDoWhile stmt)
	{
		Write("do");
		WriteChild(stmt.Body);
		Write("while (");
		Write(stmt.Cond);
		WriteLine(");");
	}

	public virtual void Visit(CiFor stmt)
	{
		Write("for (");
		if (stmt.Init != null)
			stmt.Init.Accept(this);
		Write(';');
		if (stmt.Cond != null) {
			Write(' ');
			Write(stmt.Cond);
		}
		Write(';');
		if (stmt.Advance != null) {
			Write(' ');
			stmt.Advance.Accept(this);
		}
		Write(')');
		WriteChild(stmt.Body);
	}

	protected virtual void WriteIfOnTrue(CiIf stmt)
	{
		WriteChild(stmt.OnTrue);
	}

	public virtual void Visit(CiIf stmt)
	{
		Write("if (");
		Write(stmt.Cond);
		Write(')');
		WriteIfOnTrue(stmt);
		if (stmt.OnFalse != null) {
			Write("else");
			if (stmt.OnFalse is CiIf) {
				Write(' ');
				Write(stmt.OnFalse);
			}
			else
				WriteChild(stmt.OnFalse);
		}
	}

	void ICiStatementVisitor.Visit(CiNativeBlock statement)
	{
		Write(statement.Content);
	}

	public virtual void Visit(CiReturn stmt)
	{
		if (stmt.Value == null)
			WriteLine("return;");
		else {
			Write("return ");
			Write(stmt.Value);
			WriteLine(";");
		}
	}

	protected virtual void StartSwitch(CiSwitch stmt)
	{
	}

	protected virtual void StartCase(ICiStatement stmt)
	{
	}

	protected virtual void WriteFallthrough(CiExpr expr)
	{
	}

	protected virtual void EndSwitch(CiSwitch stmt)
	{
	}

	public virtual void Visit(CiSwitch stmt)
	{
		Write("switch (");
		Write(stmt.Value);
		WriteLine(") {");
		StartSwitch(stmt);
		foreach (CiCase kase in stmt.Cases) {
			foreach (object value in kase.Values) {
				Write("case ");
				WriteConst(value);
				WriteLine(":");
			}
			this.Indent++;
			StartCase(kase.Body[0]);
			Write(kase.Body);
			if (kase.Fallthrough)
				WriteFallthrough(kase.FallthroughTo);
			this.Indent--;
		}
		if (stmt.DefaultBody != null) {
			WriteLine("default:");
			this.Indent++;
			StartCase(stmt.DefaultBody[0]);
			Write(stmt.DefaultBody);
			this.Indent--;
		}
		EndSwitch(stmt);
		WriteLine("}");
	}

	public abstract void Visit(CiThrow stmt);

	public virtual void Visit(CiWhile stmt)
	{
		Write("while (");
		Write(stmt.Cond);
		Write(')');
		WriteChild(stmt.Body);
	}

	void Write(ICiStatement stmt)
	{
		stmt.Accept(this);
		if ((stmt is CiMaybeAssign || stmt is CiVar) && !this.at_line_start)
			WriteLine(";");
	}

	void WriteBody(CiMethod method)
	{
		if (method.CallType == CiCallType.Abstract)
			WriteLine(";");
		else {
			WriteLine();
			if (method.Body is CiBlock)
				Write(method.Body);
			else {
				OpenBlock();
				Write(method.Body);
				CloseBlock();
			}
		}
	}

	void OpenClass(bool isAbstract, CiClass klass, string extendsClause)
	{
		if (isAbstract)
			Write("abstract ");
		Write("class ");
		Write(klass.Name);
		if (klass.BaseClass != null) {
			Write(extendsClause);
			Write(klass.BaseClass.Name);
		}
		WriteLine();
		OpenBlock();
	}

	public abstract void Write(CiProgram prog);
}

}

#endif
