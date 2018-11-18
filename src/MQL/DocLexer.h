#ifndef _Ci_CiDocLexer_h_
#define _Ci_CiDocLexer_h_


namespace Ci
{

typedef enum
{
	EndOfFile_,
	Char,
	CodeDelimiter,
	Bullet,
	Para,
	Period
} CiDocToken;

struct CiDocLexer
{
	CiLexer* ci_lexer = NULL;
	bool check_period = false;
	CiDocToken current_token;
	int current_char = '\n';

	CiDocLexer(CiLexer* ci_lexer) : ci_lexer(ci_lexer)
	{
		check_period = true;
		NextToken();
	}

	int PeekChar()
	{
		return ci_lexer->PeekChar();
	}

	int ReadChar()
	{
		int c = ci_lexer->ReadChar();
		if (c == '\n' && ci_lexer->NextToken() != DocComment)
			return -1;
		return c;
	}

	CiDocToken ReadToken()
	{
		int lastChar = this->current_char;
		for (;;) {
			int c = ReadChar();
			this->current_char = c;
			switch (c) {
			case -1:
				return EndOfFile_;
			case '`':
				return CodeDelimiter;
			case '*':
				if (lastChar == '\n' && PeekChar() == ' ') {
					ReadChar();
					return Bullet;
				}
				return Char;
			case '\r':
				continue;
			case '\n':
				if (this->check_period && lastChar == '.') {
					this->check_period = false;
					return Period;
				}
				if (lastChar == '\n')
					return Para;
				return Char;
			default:
				return Char;
			}
		}
	}

	CiDocToken NextToken()
	{
		CiDocToken token = ReadToken();
		current_token = token;
		return token;
	}
};

}


#endif
