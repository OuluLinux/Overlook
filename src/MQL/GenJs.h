
namespace Ci
{

class GenJs : SourceGenerator
{
	CiClass current_class;
	bool UsesSubstringMethod;
	bool UsesCopyArrayMethod;
	bool UsesBytesToStringMethod;
	bool UsesClearArrayMethod;

	protected override void Write(CiCodeDoc doc)
	{
		if (doc == NULL)
			return;
		// TODO
	}

	void Write(CiEnum enu)
	{
		WriteLine();
		Write(enu.Documentation);
		Write("var ");
		Write(enu.Name);
		Write(" = ");
		OpenBlock();
		for (int i = 0; i < enu.Values.Length; i++) {
			if (i > 0)
				WriteLine(",");
			CiEnumValue value = enu.Values[i];
			Write(value.Documentation);
			WriteUppercaseWithUnderscores(value.Name);
			Write(" : ");
			Write(i);
		}
		WriteLine();
		CloseBlock();
	}

	protected override void WriteNew(CiType type)
	{
		CiClassStorageType classType = type as CiClassStorageType;
		if (classType != NULL) {
			Write("new ");
			Write(classType.Class.Name);
			Write("()");
		}
		else {
			CiArrayStorageType arrayType = (CiArrayStorageType) type;
			Write("new Array(");
			if (arrayType.LengthExpr != NULL)
				Write(arrayType.LengthExpr);
			else
				Write(arrayType.Length);
			Write(')');
		}
	}

	bool WriteInit(CiType type)
	{
		if (type is CiClassStorageType || type is CiArrayStorageType) {
			Write(" = ");
			WriteNew(type);
			return true;
		}
		return false;
	}

	void Write(CiField field)
	{
		Write(field.Documentation);
		Write("this->");
		WriteCamelCase(field.Name);
		CiType type = field.Type;
		if (type == CiBoolType.Value)
			Write(" = false");
		else if (type == CiByteType::Value() || type == CiIntType::Value())
			Write(" = 0");
		else if (type is CiEnum) {
			Write(" = ");
			WriteConst(((CiEnum) type).Values[0]);
		}
		else if (!WriteInit(type))
			Write(" = NULL");
		WriteLine(";");
	}

	protected override void WriteConst(object value)
	{
		if (value is CiEnumValue) {
			CiEnumValue ev = (CiEnumValue) value;
			Write(ev.Type.Name);
			Write('.');
			WriteUppercaseWithUnderscores(ev.Name);
		}
		else if (value is Array) {
			Write("[ ");
			WriteContent((Array) value);
			Write(" ]");
		}
		else
			base.WriteConst(value);
	}

	protected override void WriteName(CiConst konst)
	{
		Write(this->current_class.Name);
		Write('.');
		WriteUppercaseWithUnderscores(konst.GlobalName ?? konst.Name);
	}

	protected override CiPriority GetPriority(CiExpr expr)
	{
		if (expr is CiPropertyAccess) {
			CiProperty prop = ((CiPropertyAccess) expr).Property;
			if (prop == CiLibrary::SByteProperty)
				return CiPriority.Additive;
			if (prop == CiLibrary::LowByteProperty)
				return CiPriority.And;
		}
		else if (expr is CiBinaryExpr) {
			if (((CiBinaryExpr) expr).Op == Slash)
				return CiPriority.Postfix;
		}
		return base.GetPriority(expr);
	}

	protected override void Write(CiFieldAccess expr)
	{
		WriteChild(expr, expr.Obj);
		Write('.');
		WriteCamelCase(expr.Field.Name);
	}

	protected override void Write(CiPropertyAccess expr)
	{
		if (expr->property == CiLibrary::SByteProperty) {
			Write('(');
			WriteChild(CiPriority.Xor, expr.Obj);
			Write(" ^ 128) - 128");
		}
		else if (expr->property == CiLibrary::LowByteProperty) {
			WriteChild(expr, expr.Obj);
			Write(" & 0xff");
		}
		else if (expr->property == CiLibrary::StringLengthProperty) {
			WriteChild(expr, expr.Obj);
			Write(".length");
		}
		else
			throw ArgumentException(expr->property.Name);
	}

	protected override void WriteName(CiBinaryResource resource)
	{
		Write(this->current_class.Name);
		Write(".CI_BINARY_RESOURCE_");
		foreach (char c in resource.Name)
			Write(CiLexer.IsLetter(c) ? char.ToUpperInvariant(c) : '_');
	}

	protected override void WriteName(CiMethod method)
	{
		WriteCamelCase(method.Name);
	}

	protected override void Write(CiMethodCall expr)
	{
		if (expr.Method == CiLibrary::MulDivMethod) {
			Write("Math.floor(");
			WriteMulDiv(CiPriority.Multiplicative, expr);
		}
		else if (expr.Method == CiLibrary::CharAtMethod) {
			Write(expr.Obj);
			Write(".charCodeAt(");
			Write(expr.Arguments[0]);
			Write(')');
		}
		else if (expr.Method == CiLibrary::SubstringMethod) {
			if (expr.Arguments[0].HasSideEffect) {
				Write("Ci.substring(");
				Write(expr.Obj);
				Write(", ");
				Write(expr.Arguments[0]);
				Write(", ");
				Write(expr.Arguments[1]);
				Write(')');
				this->UsesSubstringMethod = true;
			}
			else {
				Write(expr.Obj);
				Write(".substring(");
				Write(expr.Arguments[0]);
				Write(", ");
				Write(new CiBinaryExpr(expr.Arguments[0], Plus, expr.Arguments[1] });
				Write(')');
			}
		}
		else if (expr.Method == CiLibrary::Arraycopy_toMethod) {
			Write("Ci.copyArray(");
			Write(expr.Obj);
			Write(", ");
			Write(expr.Arguments[0]);
			Write(", ");
			Write(expr.Arguments[1]);
			Write(", ");
			Write(expr.Arguments[2]);
			Write(", ");
			Write(expr.Arguments[3]);
			Write(')');
			this->UsesCopyArrayMethod = true;
		}
		else if (expr.Method == CiLibrary::ArrayToStringMethod) {
			Write("Ci.bytesToString(");
			Write(expr.Obj);
			Write(", ");
			Write(expr.Arguments[0]);
			Write(", ");
			Write(expr.Arguments[1]);
			Write(')');
			this->UsesBytesToStringMethod = true;
		}
		else if (expr.Method == CiLibrary::ArrayStorageClearMethod) {
			Write("Ci.clearArray(");
			Write(expr.Obj);
			Write(", 0)");
			this->UsesClearArrayMethod = true;
		}
		else
			base.Write(expr);
	}

	protected override void Write(CiBinaryExpr expr)
	{
		if (expr.Op == Slash) {
			Write("Math.floor(");
			WriteChild(CiPriority.Multiplicative, expr.Left);
			Write(" / ");
			WriteNonAssocChild(CiPriority.Multiplicative, expr.Right);
			Write(')');
		}
		else
			base.Write(expr);
	}

	protected virtual void WriteInitArrayStorageVar(CiVar stmt)
	{
		WriteLine(";");
		Write("Ci.clearArray(");
		Write(stmt.Name);
		Write(", ");
		Write(stmt.InitialValue);
		Write(')');
		this->UsesClearArrayMethod = true;
	}

	virtual void Visit(CiVar stmt)
	{
		Write("var ");
		Write(stmt.Name);
		WriteInit(stmt.Type);
		if (stmt.InitialValue != NULL) {
			if (stmt.Type is CiArrayStorageType)
				WriteInitArrayStorageVar(stmt);
			else {
				Write(" = ");
				Write(stmt.InitialValue);
			}
		}
	}

	virtual void Visit(CiThrow stmt)
	{
		Write("throw ");
		Write(stmt.Message);
		WriteLine(";");
	}

	void Write(CiMethod method)
	{
		if (method.CallType == CiCallType.Abstract)
			return;
		WriteLine();
		Write(method.Class.Name);
		Write('.');
		if (method.CallType != CiCallType.Static)
			Write("prototype.");
		WriteCamelCase(method.Name);
		Write(" = function(");
		bool first = true;
		foreach (CiParam param in method.Signature.Params) {
			if (first)
				first = false;
			else
				Write(", ");
			Write(param.Name);
		}
		Write(") ");
		if (method.Body is CiBlock)
			Write(method.Body);
		else {
			OpenBlock();
			Write(method.Body);
			CloseBlock();
		}
	}

	void Write(CiConst konst)
	{
		WriteName(konst);
		Write(" = ");
		WriteConst(konst.Value);
		WriteLine(";");
	}

	void Write(CiClass klass)
	{
		// topological sorting of class hierarchy
		if (klass.WriteStatus == CiWriteStatus.Done)
			return;
		if (klass.WriteStatus == CiWriteStatus.InProgress)
			throw ResolveException("Circular dependency for class {0}", klass.Name);
		klass.WriteStatus = CiWriteStatus.InProgress;
		if (klass.BaseClass != NULL)
			Write(klass.BaseClass);
		klass.WriteStatus = CiWriteStatus.Done;

		this->current_class = klass;
		WriteLine();
		Write(klass.Documentation);
		Write("function ");
		Write(klass.Name);
		WriteLine("()");
		OpenBlock();
		foreach (CiSymbol member in klass.Members) {
			if (member is CiField)
				Write((CiField) member);
		}
		if (klass.Constructor != NULL)
			Write(((CiBlock) klass.Constructor.Body).Statements);
		CloseBlock();
		if (klass.BaseClass != NULL) {
			Write(klass.Name);
			Write(".prototype = new ");
			Write(klass.BaseClass.Name);
			WriteLine("();");
		}
		foreach (CiSymbol member in klass.Members) {
			if (member is CiMethod)
				Write((CiMethod) member);
			else if (member is CiConst && member.Visibility == CiVisibility.Public)
				Write((CiConst) member);
		}
		foreach (CiConst konst in klass.ConstArrays)
			Write(konst);
		foreach (CiBinaryResource resource in klass.binary_resources) {
			WriteName(resource);
			Write(" = ");
			WriteConst(resource.Content);
			WriteLine(";");
		}
		this->current_class = NULL;
	}

	void WriteBuiltins()
	{
		List<string[]> code = new List<string[]>();
		if (this->UsesSubstringMethod) {
			code.Add(new string[] {
				"substring : function(s, offset, length)",
				"return s.substring(offset, offset + length);"
			});
		}
		if (this->UsesCopyArrayMethod) {
			code.Add(new string[] {
				"copyArray : function(sa, soffset, da, doffset, length)",
				"for (var i = 0; i < length; i++)",
				"\tda[doffset + i] = sa[soffset + i];"
			});
		}
		if (this->UsesBytesToStringMethod) {
			code.Add(new string[] {
				"bytesToString : function(a, offset, length)",
				"var s = \"\";",
				"for (var i = 0; i < length; i++)",
				"\ts += String.fromCharCode(a[offset + i]);",
				"return s;"
			});
		}
		if (this->UsesClearArrayMethod) {
			code.Add(new string[] {
				"clearArray : function(a, value)",
				"for (var i = 0; i < a.length; i++)",
				"\ta[i] = value;"
			});
		}
		if (code.Count > 0) {
			WriteLine("var Ci = {");
			this->Indent++;
			for (int i = 0; ; ) {
				string[] lines = code[i];
				Write(lines[0]);
				WriteLine(" {");
				this->Indent++;
				for (int j = 1; j < lines.Length; j++)
					WriteLine(lines[j]);
				this->Indent--;
				Write('}');
				if (++i >= code.Count)
					break;
				WriteLine(",");
			}
			WriteLine();
			this->Indent--;
			WriteLine("};");
		}
	}

	virtual void Write(CiProgram prog)
	{
		CreateFile(this->OutputFile);
		this->UsesSubstringMethod = false;
		this->UsesCopyArrayMethod = false;
		this->UsesBytesToStringMethod = false;
		this->UsesClearArrayMethod = false;
		foreach (CiSymbol symbol in prog.Globals)
			if (symbol is CiClass)
				((CiClass) symbol).WriteStatus = CiWriteStatus.NotYet;
		foreach (CiSymbol symbol in prog.Globals) {
			if (symbol is CiEnum)
				Write((CiEnum) symbol);
			else if (symbol is CiClass)
				Write((CiClass) symbol);
		}
		WriteBuiltins();
		CloseFile();
	}
}

}
