#include "AsmProto.h"


Conf NLine0(const Conf **v, int count) {
	Conf c;
	c.type = CONF_LINE;
	for(int i = 0; i < count; i++)
		c.sub.Add(*v[i]);
	return c;
}

Conf Item(String type) {
	Conf c;
	c.type = CONF_ITEM;
	c.arg_s = type;
	return c;
}

Conf Group(int size, Conf base) {
	Conf c;
	c.type = CONF_GROUP;
	c.arg_i = size;
	c.sub.Add(base);
	return c;
}

String Conf::ToString(int indent) const {
	String s;
	s.Cat('\t', indent);
	switch (type) {
		case CONF_LINE:			s << "line"; break;
		case CONF_ITEM:			s << "item " + arg_s; break;
		case CONF_GROUP:		s << "group " + IntStr(arg_i); break;
	}
	s << "\n";
	for(int i = 0; i < sub.GetCount(); i++) {
		s << sub[i].ToString(indent + 1);
	}
	return s;
}





