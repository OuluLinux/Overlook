#ifndef _Ci_CiLexer_h_
#define _Ci_CiLexer_h_


namespace Ci {





struct CiLexer {
	StringReader* reader = NULL;
	String filename;
	int input_line_no;
	CiToken current_token;
	String current_string;
	int current_int;
	SharedString* copy_to = NULL;
	Index<String> pre_symbols;
	bool at_line_start = true;
	bool line_mode = false;
	
public:
	CiLexer() {
		pre_symbols.Add("true");
	}
	
	void Open(String filename, StringReader* reader) {
		this->filename = filename;
		this->reader = reader;
		input_line_no = 1;
		NextToken();
	}
	
	virtual bool IsExpandingMacro() {
		return false;
	}
	
	virtual bool ExpandMacroArg(String name) {
		return false;
	}
	
	virtual bool OnStreamEnd() {
		return false;
	}
	
	StringReader* SetReader(StringReader* reader)
	{
		StringReader* old = this->reader;
		this->reader = reader;
		return old;
	}
	
	StringReader id_reader;
	
	int PeekChar() {
		if (!this->id_reader.IsEmpty())
			return this->id_reader.Peek();
			
		return this->reader->Peek();
	}
	
	static bool IsLetter(int c) {
		if (c >= 'a' && c <= 'z')
			return true;
			
		if (c >= 'A' && c <= 'Z')
			return true;
			
		if (c >= '0' && c <= '9')
			return true;
			
		return c == '_';
	}
	
	int ReadChar() {
		int c;
		
		if (!this->id_reader.IsEmpty()) {
			c = this->id_reader.Read();
			
			if (this->id_reader.Peek() < 0)
				this->id_reader.Clear();
		}
		
		else {
			c = this->reader->Read();
			
			if (IsLetter(c)) {
				String sb;
				
				for (;;) {
					sb.Cat((char) c);
					c = this->reader->Peek();
					
					if (!IsLetter(c))
						break;
						
					this->reader->Read();
				}
				
				if (c == '#' && this->IsExpandingMacro()) {
					this->reader->Read();
					
					if (this->reader->Read() != '#')
						throw ParseException("Invalid character");
				}
				
				String s = sb.ToString();
				
				if (!ExpandMacroArg(s))
					this->id_reader.Set(s);
					
				return ReadChar();
			}
			
			if (c == '\n' && !this->IsExpandingMacro()) {
				this->input_line_no++;
				this->at_line_start = true;
			}
		}
		
		if (c >= 0) {
			if (this->copy_to)
				this->copy_to->Cat(c);
				
			switch (c) {
			
			case '\t':
			
			case '\r':
			
			case ' ':
			
			case '\n':
				break;
				
			default:
				this->at_line_start = false;
				break;
			}
			
			while (this->reader->Peek() < 0 && OnStreamEnd())
				;
		}
		
		return c;
	}
	
	bool EatChar(int c) {
		if (PeekChar() == c) {
			ReadChar();
			return true;
		}
		
		return false;
	}
	
	int ReadDigit(bool hex) {
		int c = PeekChar();
		
		if (c >= '0' && c <= '9')
			return ReadChar() - '0';
			
		if (hex) {
			if (c >= 'a' && c <= 'f')
				return ReadChar() - 'a' + 10;
				
			if (c >= 'A' && c <= 'F')
				return ReadChar() - 'A' + 10;
		}
		
		return -1;
	}
	
	char ReadCharLiteral() {
		int c = ReadChar();
		
		if (c < 32)
			throw ParseException("Invalid character in literal");
			
		if (c != '\\')
			return (char) c;
			
		switch (ReadChar()) {
		
		case 't':
			return '\t';
			
		case 'r':
			return '\r';
			
		case 'n':
			return '\n';
			
		case '\\':
			return '\\';
			
		case '\'':
			return '\'';
			
		case '"':
			return '"';
			
		default:
			throw ParseException("Unknown escape sequence");
		}
	}
	
	String ReadId(int c) {
		String sb;
		
		for (;;) {
			sb.Cat(c);
			
			if (!IsLetter(PeekChar()))
				break;
				
			c = ReadChar();
		}
		
		return sb;
	}
	
	CiToken ReadPreToken() {
		for (;;) {
			bool atLineStart = this->at_line_start;
			int c = ReadChar();
			
			switch (c) {
			
			case - 1:
				return EndOfFile;
				
			case '\t':
			
			case '\r':
			
			case ' ':
				continue;
				
			case '\n':
			
				if (this->line_mode)
					return EndOfLine;
					
				continue;
				
			case '#':
				c = ReadChar();
				
				if (c == '#')
					return PasteTokens;
					
				if (atLineStart && IsLetter(c)) {
					String s = ReadId(c);
					
					if (s == "if")
						return PreIf;
						
					if (s ==  "elif")
						return PreElIf;
						
					if (s == "else")
						return PreElse;
						
					if (s == "endif")
						return PreEndIf;
						
					throw ParseException("Unknown preprocessor directive #" + s);
				}
				
				throw ParseException("Invalid character");
				
			case ';':
				return Semicolon;
				
			case '.':
				return Dot;
				
			case ',':
				return Comma;
				
			case '(':
				return LeftParenthesis;
				
			case ')':
				return RightParenthesis;
				
			case '[':
				return LeftBracket;
				
			case ']':
				return RightBracket;
				
			case '{':
				return LeftBrace;
				
			case '}':
				return RightBrace;
				
			case '+':
			
				if (EatChar('+'))
					return Increment;
					
				if (EatChar('='))
					return AddAssign;
					
				return Plus;
				
			case '-':
				if (EatChar('-'))
					return Decrement;
					
				if (EatChar('='))
					return SubAssign;
					
				return Minus;
				
			case '*':
				if (EatChar('='))
					return MulAssign;
					
				return Asterisk;
				
			case '/':
				if (EatChar('/')) {
					c = ReadChar();
					
					if (c == '/') {
						while (EatChar(' '))
							;
							
						return DocComment;
					}
					
					while (c != '\n' && c >= 0)
						c = ReadChar();
						
					if (c == '\n' && this->line_mode)
						return EndOfLine;
						
					continue;
				}
				
				if (EatChar('='))
					return DivAssign;
					
				return Slash;
				
			case '%':
				if (EatChar('='))
					return ModAssign;
					
				return Mod;
				
			case '&':
				if (EatChar('&'))
					return CondAndToken;
					
				if (EatChar('='))
					return AndAssign;
					
				return AndToken;
				
			case '|':
				if (EatChar('|'))
					return CondOrToken;
					
				if (EatChar('='))
					return OrAssign;
					
				return OrToken;
				
			case '^':
				if (EatChar('='))
					return XorAssign;
					
				return XorToken;
				
			case '=':
				if (EatChar('='))
					return Equal;
					
				return Assign;
				
			case '!':
				if (EatChar('='))
					return NotEqual;
					
				return CondNot;
				
			case '<':
				if (EatChar('<')) {
					if (EatChar('='))
						return ShiftLeftAssign;
						
					return ShiftLeft;
				}
				
				if (EatChar('='))
					return LessOrEqual;
					
				return Less;
				
			case '>':
				if (EatChar('>')) {
					if (EatChar('='))
						return ShiftRightAssign;
						
					return ShiftRight;
				}
				
				if (EatChar('='))
					return GreaterOrEqual;
					
				return Greater;
				
			case '~':
				return Not;
				
			case '?':
				return QuestionMark;
				
			case ':':
				return Colon;
				
			case '\'':
				this->current_int = ReadCharLiteral();
				
				if (ReadChar() != '\'')
					throw ParseException("Unterminated character literal");
					
				return IntConstant;
				
			case '"': {
					String sb;
					
					while (PeekChar() != '"')
						sb.Cat(ReadCharLiteral());
						
					ReadChar();
					
					this->current_string = sb.ToString();
					
					return StringConstant;
				}
				
			case '0':
			
				if (EatChar('x')) {
					int i = ReadDigit(true);
					
					if (i < 0)
						throw ParseException("Invalid hex number");
						
					for (;;) {
						int d = ReadDigit(true);
						
						if (d < 0) {
							this->current_int = i;
							return IntConstant;
						}
						
						if (i > 0x7ffffff)
							throw ParseException("Hex number too big");
							
						i = (i << 4) + d;
					}
				}
				
			case '1':
			
			case '2':
			
			case '3':
			
			case '4':
			
			case '5':
			
			case '6':
			
			case '7':
			
			case '8':
			
			case '9': {
					int i = c - '0';
					
					for (;;) {
						int d = ReadDigit(false);
						
						if (d < 0) {
							this->current_int = i;
							return IntConstant;
						}
						
						if (i == 0)
							throw ParseException("Octal numbers not supported");
							
						if (i > 214748364)
							throw ParseException("Integer too big");
							
						i = 10 * i + d;
						
						if (i < 0)
							throw ParseException("Integer too big");
					}
				}
				
			case 'A':
			
			case 'B':
			
			case 'C':
			
			case 'D':
			
			case 'E':
			
			case 'F':
			
			case 'G':
			
			case 'H':
			
			case 'I':
			
			case 'J':
			
			case 'K':
			
			case 'L':
			
			case 'M':
			
			case 'N':
			
			case 'O':
			
			case 'P':
			
			case 'Q':
			
			case 'R':
			
			case 'S':
			
			case 'T':
			
			case 'U':
			
			case 'V':
			
			case 'W':
			
			case 'X':
			
			case 'Y':
			
			case 'Z':
			
			case '_':
			
			case 'a':
			
			case 'b':
			
			case 'c':
			
			case 'd':
			
			case 'e':
			
			case 'f':
			
			case 'g':
			
			case 'h':
			
			case 'i':
			
			case 'j':
			
			case 'k':
			
			case 'l':
			
			case 'm':
			
			case 'n':
			
			case 'o':
			
			case 'p':
			
			case 'q':
			
			case 'r':
			
			case 's':
			
			case 't':
			
			case 'u':
			
			case 'v':
			
			case 'w':
			
			case 'x':
			
			case 'y':
			
			case 'z': {
					String s = ReadId(c);
					
					
					if (s == "abstract")
						return AbstractToken;
						
					if (s == "break")
						return Break;
						
					if (s == "case")
						return Case;
						
					if (s == "class")
						return Class;
						
					if (s == "const")
						return Const;
						
					if (s == "continue")
						return Continue;
						
					if (s == "default")
						return Default;
						
					if (s == "delegate")
						return Delegate;
						
					if (s == "delete")
						return Delete;
						
					if (s == "do")
						return Do;
						
					if (s == "else")
						return Else;
						
					if (s == "enum")
						return Enum;
						
					if (s == "for")
						return For;
						
					if (s == "goto")
						return Goto;
						
					if (s == "if")
						return If;
						
					if (s == "internal")
						return InternalToken;
						
					if (s == "macro")
						return Macro;
						
					if (s == "native")
						return Native;
						
					if (s == "new")
						return New;
						
					if (s == "override")
						return OverrideToken;
						
					if (s == "public")
						return PublicToken;
						
					if (s == "return")
						return Return;
						
					if (s == "static")
						return StaticToken;
						
					if (s == "switch")
						return Switch;
						
					if (s == "throw")
						return Throw;
						
					if (s == "virtual")
						return VirtualToken;
						
					if (s == "void")
						return Void;
						
					if (s == "while")
						return While;
						
					this->current_string = s;
					return Id;
				}
				
			default:
				break;
			}
			
			throw ParseException("Invalid character");
		}
	}
	
	void NextPreToken() {
		this->current_token = ReadPreToken();
	}
	
	bool EatPre(CiToken token) {
		if (See(token)) {
			NextPreToken();
			return true;
		}
		
		return false;
	}
	
	bool ParsePrePrimary() {
		if (EatPre(CondNot))
			return !ParsePrePrimary();
			
		if (EatPre(LeftParenthesis)) {
			bool result = ParsePreOr();
			Check(RightParenthesis);
			NextPreToken();
			return result;
		}
		
		if (See(Id)) {
			bool result = this->pre_symbols.Find(this->current_string) != -1;
			NextPreToken();
			return result;
		}
		
		throw ParseException("Invalid preprocessor expression");
	}
	
	bool ParsePreAnd() {
		bool result = ParsePrePrimary();
		
		while (EatPre(CondAndToken))
			result &= ParsePrePrimary();
			
		return result;
	}
	
	bool ParsePreOr() {
		bool result = ParsePreAnd();
		
		while (EatPre(CondOrToken))
			result |= ParsePreAnd();
			
		return result;
	}
	
	bool ParsePreExpr() {
		this->line_mode = true;
		NextPreToken();
		bool result = ParsePreOr();
		Check(EndOfLine);
		this->line_mode = false;
		return result;
	}
	
	void ExpectEndOfLine(String directive) {
		this->line_mode = true;
		CiToken token = ReadPreToken();
		
		if (token != EndOfLine && token != EndOfFile)
			throw ParseException("Unexpected characters after " + directive);
			
		this->line_mode = false;
	}
	
	typedef enum {
		IfOrElIf,
		Else_
	} PreDirectiveClass;
	
	Stack<PreDirectiveClass> pre_stack;
	
	void PopPreStack(String directive) {
		try {
			PreDirectiveClass pdc = this->pre_stack.Pop();
			
			if (directive != "#endif" && pdc == Else_)
				throw ParseException(directive + " after #else");
		}
		
		catch (InvalidOperationException e) {
			throw ParseException(directive + " with no matching #if");
		}
	}
	
	void SkipUntilPreMet() {
		for (;;) {
			// we are in a conditional that wasn't met yet
			switch (ReadPreToken()) {
			
			case EndOfFile:
				throw ParseException("Expected #endif, got end of file");
				
			case PreIf:
				ParsePreExpr();
				SkipUntilPreEndIf(false);
				break;
				
			case PreElIf:
			
				if (ParsePreExpr()) {
					this->pre_stack.Push(IfOrElIf);
					return;
				}
				
				break;
				
			case PreElse:
				ExpectEndOfLine("#else");
				this->pre_stack.Push(Else_);
				return;
				
			case PreEndIf:
				ExpectEndOfLine("#endif");
				return;
			}
		}
	}
	
	void SkipUntilPreEndIf(bool wasElse) {
		for (;;) {
			// we are in a conditional that was met before
			switch (ReadPreToken()) {
			
			case EndOfFile:
				throw ParseException("Expected #endif, got end of file");
				
			case PreIf:
				ParsePreExpr();
				SkipUntilPreEndIf(false);
				break;
				
			case PreElIf:
			
				if (wasElse)
					throw ParseException("#elif after #else");
					
				ParsePreExpr();
				
				break;
				
			case PreElse:
				if (wasElse)
					throw ParseException("#else after #else");
					
				ExpectEndOfLine("#else");
				
				wasElse = true;
				
				break;
				
			case PreEndIf:
				ExpectEndOfLine("#endif");
				
				return;
			}
		}
	}
	
	CiToken ReadToken() {
		for (;;) {
			// we are in no conditionals or in all met
			CiToken token = ReadPreToken();
			
			switch (token) {
			
			case EndOfFile:
			
				if (this->pre_stack.GetCount() != 0)
					throw ParseException("Expected #endif, got end of file");
					
				return EndOfFile;
				
			case PreIf:
				if (ParsePreExpr()) {
					this->pre_stack.Push(IfOrElIf);
					break;
				}
				
				else
					SkipUntilPreMet();
					
				break;
				
			case PreElIf:
				PopPreStack("#elif");
				
				ParsePreExpr();
				
				SkipUntilPreEndIf(false);
				
				break;
				
			case PreElse:
				PopPreStack("#else");
				
				ExpectEndOfLine("#else");
				
				SkipUntilPreEndIf(true);
				
				break;
				
			case PreEndIf:
				PopPreStack("#endif");
				
				ExpectEndOfLine("#endif");
				
				break;
				
			default:
				return token;
			}
		}
	}
	
	CiToken NextToken() {
		CiToken token = ReadToken();
		this->current_token = token;
		return token;
	}
	
	bool See(CiToken token) {
		return this->current_token == token;
	}
	
	bool Eat(CiToken token) {
		if (See(token)) {
			NextToken();
			return true;
		}
		
		return false;
	}
	
	void Check(CiToken expected) {
		if (!See(expected))
			throw ParseException("Expected " + TokenStr(expected) + ", got " + TokenStr(this->current_token));
	}
	
	void Expect(CiToken expected) {
		Check(expected);
		NextToken();
	}
	
	void DebugLexer() {
		while (this->current_token != EndOfFile) {
			LOG(TokenStr(this->current_token));
			NextToken();
		}
	}
	
	static String TokenStr(int i) {
		#define CASE(x) case x: return #x;
		switch (i) {
			CASE(EndOfFile)
			CASE(Id)
			CASE(IntConstant)
			CASE(StringConstant)
			CASE(Semicolon)
			CASE(Dot)
			CASE(Comma)
			CASE(LeftParenthesis)
			CASE(RightParenthesis)
			CASE(LeftBracket)
			CASE(RightBracket)
			CASE(LeftBrace)
			CASE(RightBrace)
			CASE(Plus)
			CASE(Minus)
			CASE(Asterisk)
			CASE(Slash)
			CASE(Mod)
			CASE(AndToken)
			CASE(OrToken)
			CASE(XorToken)
			CASE(Not)
			CASE(ShiftLeft)
			CASE(ShiftRight)
			CASE(Equal)
			CASE(NotEqual)
			CASE(Less)
			CASE(LessOrEqual)
			CASE(Greater)
			CASE(GreaterOrEqual)
			CASE(CondAndToken)
			CASE(CondOrToken)
			CASE(CondNot)
			CASE(Assign)
			CASE(AddAssign)
			CASE(SubAssign)
			CASE(MulAssign)
			CASE(DivAssign)
			CASE(ModAssign)
			CASE(AndAssign)
			CASE(OrAssign)
			CASE(XorAssign)
			CASE(ShiftLeftAssign)
			CASE(ShiftRightAssign)
			CASE(Increment)
			CASE(Decrement)
			CASE(QuestionMark)
			CASE(Colon)
			CASE(DocComment)
			CASE(PasteTokens)
			CASE(AbstractToken)
			CASE(Break)
			CASE(Case)
			CASE(Class)
			CASE(Const)
			CASE(Continue)
			CASE(Default)
			CASE(Delegate)
			CASE(Delete)
			CASE(Do)
			CASE(Else)
			CASE(Enum)
			CASE(For)
			CASE(Goto)
			CASE(If)
			CASE(InternalToken)
			CASE(Macro)
			CASE(Native)
			CASE(New)
			CASE(OverrideToken)
			CASE(PublicToken)
			CASE(Return)
			CASE(StaticToken)
			CASE(Switch)
			CASE(Throw)
			CASE(VirtualToken)
			CASE(Void)
			CASE(While)
			CASE(EndOfLine)
			CASE(PreIf)
			CASE(PreElIf)
			CASE(PreElse)
			CASE(PreEndIf)
		}
		#undef CASE
		return IntStr(i);
	}
	
};

}


#endif
