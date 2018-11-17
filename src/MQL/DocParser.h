#ifndef _Ci_CiDocParser_h_
#define _Ci_CiDocParser_h_

namespace Ci {

class CiDocParser : public CiDocLexer {
	
public:
	CiDocParser(CiLexer* ci_lexer) : CiDocLexer(ci_lexer) {
		
	}
	
	bool See(CiDocToken token) {
		return current_token == token;
	}
	
	bool Eat(CiDocToken token) {
		if (See(token)) {
			NextToken();
			return true;
		}
		
		return false;
	}
	
	void Expect(CiDocToken expected) {
		if (!See(expected))
			throw Exc("Expected " + CiLexer::TokenStr(expected) + " got " + CiLexer::TokenStr(current_token));
			
		NextToken();
	}
	
	String ParseText() {
		String sb;
		
		while (See(Char)) {
			sb.Cat((char) current_char);
			NextToken();
		}
		
		if (sb.GetCount() > 0 && sb[sb.GetCount() - 1] == '\n')
			sb = sb.Left(sb.GetCount());
			
		return sb;
	}
	
	CiDocPara* ParsePara() {
		Vector<CiDocInline*> children;
		
		for (;;) {
			if (See(Char)) {
				children.Add(new CiDocInline(ParseText()));
			}
			
			else
			if (Eat(CodeDelimiter)) {
				children.Add(new CiDocInline(ParseText()));
				Expect(CodeDelimiter);
			}
			
			else
				break;
		}
		
		return new CiDocPara(children);
	}
	
	CiDocBlock* ParseBlock() {
		if (Eat(Bullet)) {
			Vector<CiDocPara*> items;
			
			do
				items.Add(ParsePara());
			while (Eat(Bullet));
			
			Eat(Para);
			
			return new CiDocList(items);
		}
		
		return ParsePara();
	}
	
	CiCodeDoc* ParseCodeDoc() {
		CiDocPara* summary = ParsePara();
		Vector<CiDocBlock*> details;
		
		if (Eat(Period)) {
			while (!See(EndOfFile_))
				details.Add(ParseBlock());
		}
		
		return new CiCodeDoc(summary, details);
	}
};

}


#endif
