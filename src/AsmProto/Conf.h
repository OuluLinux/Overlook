#ifndef _AsmProto_Conf_h_
#define _AsmProto_Conf_h_

enum {
	CONF_LINE = 1,
	CONF_ITEM,
	CONF_GROUP
};

struct Conf : Moveable<Conf> {
	Vector<Conf> sub;
	VectorMap<String, Value> args;
	String arg_s;
	int type = 0;
	int arg_i = 0;
	
	Conf() {}
	Conf(const Conf& c) {*this = c;}
	void operator=(const Conf& c) {
		sub <<= c.sub;
		args <<= c.args;
		type = c.type;
		arg_s = c.arg_s;
		arg_i = c.arg_i;
	}
	Conf& Arg(String key, Value value) {args.Add(key, value); return *this;}
	String ToString(int indent=0) const;
};

Conf NLine0(const Conf **v, int count);
#define E__NFConf(I)  const Conf& COMBINE(p, I)
#define E__NFSetArg(I) arg[I - 1] = &COMBINE(p, I)
#define E__FBody(I) \
inline Conf Line(__List##I(E__NFConf)) \
{\
	const Conf *arg[I]; \
	__List##I(E__NFSetArg); \
	return NLine0(arg, I); \
}
__Expand20(E__FBody)

Conf Item(String type);
Conf Group(int size, Conf base);



#endif
