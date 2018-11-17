

namespace Ci
{

struct CiMacro : CiSymbol
{
	Index<String> params;
	bool is_statement;
	String body;
	
	virtual void Accept(ICiSymbolVisitor* v) { }
};

struct MacroExpansion
{
	String friendly_name;
	VectorMap<String, String> args;
	StringReader* parent_reader = NULL;
	
	MacroExpansion(String fname, const VectorMap<String, String>& args, StringReader* sr) {
		friendly_name = fname;
		this->args <<= args;
		parent_reader = sr;
	}
	
	String LookupArg(String name)
	{
		String value;
		int i = args.Find(name);
		if (i >= 0)
			return args[i];
		return "";
	}
};


}
