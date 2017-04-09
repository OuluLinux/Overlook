#include "DataCore.h"


namespace DataCore {


void TimeVector::LinkPath(String dest, String src) {
	int i = linked_paths.Find(dest);
	if (i != -1) {
		ASSERT(linked_paths[i] == src);
		return;
	}
	
	linked_paths.Add(dest, src);
	
	PathLexer lex_dest(dest);
	
	// Parse source
	PathArgs src_path;
	ParsePath(src, src_path);
	
	SlotPtr slot = ResolvePath(src, src_path);
	if (!slot) {
		throw DataExc();
	}
	
	// Find slot offset in datavector
	int data_offset = 0;
	for(int i = 0; i < this->slot.GetCount(); i++)
		data_offset += this->slot[i]->GetReservedBytes();
	slot->slot_offset = data_offset;
	
	
	// Add slot to the vector
	this->slot.Add(slot);
	
	// Parse destination
	Vector<String> dest_path;
	try {
		while (lex_dest.tk != LEX_EOF) {
			lex_dest.Match(LEX_SLASH);
			String id = lex_dest.tk_str;
			if (lex_dest.tk == LEX_ID)	lex_dest.Match(LEX_ID);
			else						lex_dest.Match(LEX_INT);
			dest_path.Add(id);
		}
	}
	catch (Exc e) {
		throw DataExc();
	}
	
	PathLinkPtr p = &link_root;
	String path;
	for(int i = 0; i < dest_path.GetCount(); i++) {
		const String& s = dest_path[i];
		path += "/" + s;
		p = &p->keys.GetAdd(s);
		if (!p->link && i < dest_path.GetCount()-1) {
			/*if (i == dest_path.GetCount()-1) {
				//SlotPtr slot = new Slot();
				p->link = slot;
				p->link->SetPath(path);
				resolved_slots.Add(path, slot);
			}
			else*/ throw DataExc();
		}
	}
	
	p->path = EncodePath(src_path);
	p->link = slot;
	
	
	
}

PathLinkPtr TimeVector::FindLinkPath(String path) {
	if (path == "/")
		return &link_root;
	
	PathLexer lex(path);
	Vector<String> dest_path;
	try {
		while (lex.tk != LEX_EOF) {
			lex.Match(LEX_SLASH);
			String id = lex.tk_str;
			if (lex.tk == LEX_ID)	lex.Match(LEX_ID);
			else					lex.Match(LEX_INT);
			dest_path.Add(id);
		}
	}
	catch (Exc e) {
		throw DataExc();
	}
	
	PathLinkPtr p = &link_root;
	for(int i = 0; i < dest_path.GetCount(); i++) {
		const String& s = dest_path[i];
		int j = p->keys.Find(s);
		if (j == -1)
			return 0;
		p = &p->keys[j];
	}
	return p;
}

SlotPtr TimeVector::FindLinkSlot(const String& path) {
	PathLinkPtr link = FindLinkPath(path);
	if (!link) return SlotPtr();
	return link->link;
}

void TimeVector::ParsePath(String path, PathArgs& parsed_path) {
	PathLexer lex(path);
	try {
		int depth = 0;
		while (lex.tk != LEX_EOF) {
			String id;
			
			if (depth == 0)
				lex.Match(LEX_SLASH);
			else
				lex.Match(',');
			
			id = lex.tk_str;
			lex.Match(LEX_ID);
			
			
			// Parse arguments
			PathArg& arg = parsed_path.Add();
			arg.a = id;
			VectorMap<String, Value>& args = arg.b;
			
			if (lex.tk == LEX_ARGBEGIN) {
				lex.Match(LEX_ARGBEGIN);
				int arg_count = 0;
				while (lex.tk != LEX_SLASH && lex.tk != LEX_EOF && lex.tk != ',') {
					if (arg_count)
						lex.Match(LEX_ARGNEXT);
					String id = lex.tk_str;
					lex.Match(LEX_ID);
					lex.Match(LEX_ASSIGN);
					
					if (lex.tk == '-') {
						lex.Match(lex.tk);
						String s = lex.tk_str;
						if (lex.tk == LEX_INT)
							lex.Match(LEX_INT);
						else
							lex.Match(LEX_FLOAT);
						args.Add(id, Value(-StrDbl(s)));
					}
					else if (lex.tk == LEX_INT) {
						args.Add(id, Value(StrInt(lex.tk_str)));
						lex.Match(LEX_INT);
					}
					else if (lex.tk == LEX_FLOAT) {
						args.Add(id, Value(StrDbl(lex.tk_str)));
						lex.Match(LEX_FLOAT);
					}
					else if (lex.tk == LEX_STR) {
						args.Add(id, Value(lex.tk_str));
						lex.Match(LEX_STR);
					}
					arg_count++;
				}
			}
			
			SortByKey(args);
			
		}
		lex.Match(LEX_EOF);
	}
	catch (Exc e) {
		throw DataExc();
	}
	
}

SlotPtr TimeVector::ResolvePath(String path) {
	PathArgs parsed_path;
	ParsePath(path, parsed_path);
	return ResolvePath(path, parsed_path);
}

SlotPtr TimeVector::ResolvePath(String path, const PathArgs& parsed_path) {
	SlotPtr out;
	
	// Try to find full path first (required for resolving existing paths with array)
	String full_path = EncodePath(parsed_path);
	int i = resolved_slots.Find(full_path);
	if (i != -1)
		return resolved_slots[i];
	
	
	// Find linked paths
	i = 0;
	String seeked_path;
	{
		PathLink* existing_link = 0;
		String tmp_path;
		int i2 = -1;
		for(int j = i; j < parsed_path.GetCount(); j++) {
			const VectorMap<String, Value>& args = parsed_path[j].b;
			if (args.GetCount() != 0)
				break;
			const String& key = parsed_path[j].a;
			tmp_path = tmp_path + "/" + key;
			PathLink* link = FindLinkPath(tmp_path);
			if (!link) continue;
			if (link->path.GetCount()) {
				existing_link = link;
				i2 = j+1;
			}
		}
		if (existing_link) {
			i = i2;
			do {
				seeked_path = existing_link->path;
				out = existing_link->link;
				if (seeked_path.Find("?") != -1) break; // no arguments in links, so dont try to find
				existing_link = FindLinkPath(seeked_path);
			}
			while (existing_link);
		}
	}
	
	
	// Find already resolved paths
	for(; i < parsed_path.GetCount(); i++) {
		String tmp_path = seeked_path + "/" + EncodePath(parsed_path, i);
		int j = resolved_slots.Find(tmp_path);
		if (j == -1) break;
		out = resolved_slots[j];
		seeked_path = tmp_path;
	}
	
	
	// Create remaining part of the path
	for(; i < parsed_path.GetCount(); i++) {
		SlotPtr next;
		String path;
	
		const String& id = parsed_path[i].a;
		const VectorMap<String, Value>& args = parsed_path[i].b;
		
		int j = GetFactories().Find(id);
		if (j == -1) {
			LOG("PathResolver::ResolvePath: '" + id + "' not found");
			throw DataExc();
		}
		
		SlotFactory fac = GetFactories()[j];
		next = fac();
		
		ASSERT(next);
		
		next->SetTimeVector(this);
		
		if (!(i == 0 && !out))
			next->SetSource(out);
		
		path = seeked_path + "/" + EncodePath(parsed_path, i);
		next->SetPath(path);
		
		next->SetArguments(args);
		
		resolved_slots.Add(path, next);
		next->Init();
		
		seeked_path = path;
		out = next;
	}
	
	return out;
}











String EncodePath(const PathArgs& path) {
	String out;
	for(int i = 0; i < path.GetCount(); i++)
		out << "/" << EncodePath(path, i);
	return out;
}

String EncodePath(const PathArgs& path, int pos) {
	String out = path[pos].a;
	const VectorMap<String, Value>& args = path[pos].b;
	for(int i = 0; i < args.GetCount(); i++) {
		out.Cat(i == 0 ? '?' : '&');
		out += args.GetKey(i) + "=";
		const Value& val = args[i];
		int type = val.GetType();
		if (type == INT_V)
			out += IntStr(val);
		else if (type == DOUBLE_V)
			out += DblStr(val);
		else
			out += "\"" + val.ToString() + "\"";
	}
	return out;
}

inline bool IsWhitespace ( char ch ) {
	return ( ch == ' ' ) || ( ch == '\t' ) || ( ch == '\n' ) || ( ch == '\r' );
}

inline bool IsNumeric ( char ch ) {
	return ( ch >= '0' ) && ( ch <= '9' );
}

inline bool IsPositiveNumber ( const String &str ) {
	for ( int i = 0;i < str.GetCount();i++ )
		if ( !IsNumeric ( str[i] ) )
			return false;
	return true;
}

inline bool IsHexadecimal ( char ch ) {
	return ( ( ch >= '0' ) && ( ch <= '9' ) ) ||
		   ( ( ch >= 'a' ) && ( ch <= 'f' ) ) ||
		   ( ( ch >= 'A' ) && ( ch <= 'F' ) );
}

inline bool IsAlpha ( char ch ) {
	return ( ( ch >= 'a' ) && ( ch <= 'z' ) ) || ( ( ch >= 'A' ) && ( ch <= 'Z' ) ) || ch == '_';
}

inline bool IsIDString ( const char *s ) {
	if ( !IsAlpha ( *s ) )
		return false;
	while ( *s )
	{
		if ( ! ( IsAlpha ( *s ) || IsNumeric ( *s ) ) )
			return false;

		s++;
	}
	return true;
}

PathLexer::PathLexer(String input) {
	data = input;
	data_owned = true;
	data_start = 0;
	data_end = data.GetCount();
	Reset();
}

void PathLexer::GetNextCh() {
	curr_ch = next_ch;
	
	if ( data_pos < data_end )
		next_ch = data[data_pos];
	else
		next_ch = 0;

	data_pos++;
	
}

void PathLexer::GetNextToken() {
	tk = LEX_EOF;
	tk_str.Clear();
	
	while ( curr_ch && IsWhitespace ( curr_ch ) )
		GetNextCh();
	
	token_start = data_pos - 2;

	if ( IsAlpha ( curr_ch ) )
	{

		while ( IsAlpha ( curr_ch ) || IsNumeric ( curr_ch ) )
		{
			tk_str += curr_ch;
			GetNextCh();
		}

		tk = LEX_ID;
		
	}

	else
	if ( IsNumeric ( curr_ch ) )
	{
		bool isHex = false;

		if ( curr_ch == '0' )
		{
			tk_str += curr_ch;
			GetNextCh();
		}

		if ( curr_ch == 'x' )
		{
			isHex = true;
			tk_str += curr_ch;
			GetNextCh();
		}

		tk = LEX_INT;

		while ( IsNumeric ( curr_ch ) || ( isHex && IsHexadecimal ( curr_ch ) ) )
		{
			tk_str += curr_ch;
			GetNextCh();
		}

		if ( !isHex && curr_ch == '.' )
		{
			tk = LEX_FLOAT;
			tk_str += '.';
			GetNextCh();

			while ( IsNumeric ( curr_ch ) )
			{
				tk_str += curr_ch;
				GetNextCh();
			}
		}

		if ( !isHex && ( curr_ch == 'e' || curr_ch == 'E' ) )
		{
			tk = LEX_FLOAT;
			tk_str += curr_ch;
			GetNextCh();

			if ( curr_ch == '-' )
			{
				tk_str += curr_ch;
				GetNextCh();
			}

			while ( IsNumeric ( curr_ch ) )
			{
				tk_str += curr_ch;
				GetNextCh();
			}
		}
	}

	else
	if ( curr_ch == '"' )
	{
		GetNextCh();

		while ( curr_ch && curr_ch != '"' )
		{
			if ( curr_ch == '\\' )
			{
				GetNextCh();

				switch ( curr_ch )
				{

				case 'n' :
					tk_str += '\n';
					break;

				case '"' :
					tk_str += '"';
					break;

				case '\\' :
					tk_str += '\\';
					break;

				default:
					tk_str += curr_ch;
				}
			}

			else
			{
				tk_str += curr_ch;
			}

			GetNextCh();
		}

		GetNextCh();

		tk = LEX_STR;
	}
	
	else
	{
		tk = curr_ch;
		
		if ( tk == '=' ) {
			tk = LEX_ASSIGN;
			GetNextCh();
		}
		else if ( tk == '/' ) {
			tk = LEX_SLASH;
			GetNextCh();
		}
		else if ( tk == '?' ) {
			tk = LEX_ARGBEGIN;
			GetNextCh();
		}
		else if ( tk == '&' ) {
			tk = LEX_ARGNEXT;
			GetNextCh();
		}
		else if ( tk == '-' || tk == ',' ) {
			GetNextCh();
		}
	}

	token_last_end = token_end;

	token_end = data_pos - 3;
}

void PathLexer::Match(int expected_tk) {
	if ( tk != expected_tk )
	{
		String token_str = GetTokenStr(expected_tk);
		String msg;
		msg << "Got " << GetTokenStr ( tk ) << ", expected " << token_str
			<< " at " << data_pos;
		throw Exc ( msg );
	}

	GetNextToken();
}

String PathLexer::GetTokenStr(int token) {
	if ( token > 32 && token < 128 )
	{
		char buf[4] = "' '";
		buf[1] = ( char ) token;
		return buf;
	}

	switch ( token )
	{
		case LEX_EOF:
			return "EOF";
		case LEX_ID:
			return "ID";
		case LEX_INT:
			return "INT";
		case LEX_FLOAT:
			return "FLOAT";
		case LEX_STR:
			return "STRING";
		case LEX_ASSIGN:
			return "=";
		case LEX_SLASH:
			return "/";
		case LEX_ARGBEGIN:
			return "?";
		case LEX_ARGNEXT:
			return "&";
	}
	return "";
}

void PathLexer::Reset() {
	data_pos = data_start;
	token_start = 0;
	token_end = 0;
	token_last_end = 0;
	tk = 0;
	GetNextCh();
	GetNextCh();
	GetNextToken();
}


}
