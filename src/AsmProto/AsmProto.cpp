#include "AsmProto.h"

int period = 24;

struct Generator {
	
	struct SymbolGenerator {
		double prev_value = 1.0;
		double prev_diff = 0.0;
		int iter = 0;
		int hurst0, hurst1;
		int correlation;
		
		SymbolGenerator() {
			hurst0 = 1 + Random(4);
			hurst1 = 5 + Random(10);
			correlation = Random(2) * 2 - 1;
		}
		
		
		double Tick() {
			double volat = (sin(2*M_PI / period * iter) + 1.5) * 0.0001;
			
			double hurst = (sin(2*M_PI / period * hurst0 * iter) + sin(2*M_PI / period * hurst1 * iter)) / 2.0;
			
			double diff;
			if (prev_diff > 0) {
				if (hurst >= 0)
					diff = +volat;
				else
					diff = -volat;
			} else {
				if (hurst >= 0)
					diff = -volat;
				else
					diff = +volat;
			}
			
			iter++;
			prev_diff = diff;
			return diff;
		}
	};
	
	Array<SymbolGenerator> sym;
	int size = 1000;
	int symbols = 8;
	
	
	Generator() {
		sym.SetCount(symbols);
	}
	
	void GenerateData(Vector<Vector<double> >& data) {
		Vector<double> tmp;
		
		tmp.SetCount(symbols);
		
		data.SetCount(symbols);
		for(int i = 0; i < symbols; i++) {
			data[i].SetCount(size);
			data[i][0] = 1.0;
		}
		
		for(int i = 1; i < size; i++) {
			
			double sum = 0.0;
			for(int j = 0; j < symbols; j++) {
				double d = sym[j].Tick();
				tmp[j] = d;
				sum += d;
			}
			sum /= symbols;
			
			for(int j = 0; j < symbols; j++) {
				double prev = data[j][i-1];
				double diff = 0.8 * sum * sym[j].correlation + 0.2 * tmp[j];
				data[j][i] = prev + diff;
			}
		}
	}
};


CONSOLE_APP_MAIN
{
	try {
		Generator gen;
		Vector<Vector<double> > data;
		
		gen.GenerateData(data);
		DUMPCC(data);
		
		
		int win_size = 8;
		int sym_count = data.GetCount();
		Session ses;
		ses.Load(Line(Group(8, Line(Item("volat"), Item("polar"), Item("mux").Arg("depth", win_size))), Item("correlation").Arg("depth", win_size).Arg("ratio", 0.8)));
		LOG(ses.GetNet().ToString());
		
		IOData window;
		window.SetCount(win_size * sym_count, sym_count);
		bool running = true;
		while (running) {
			for(int i = win_size + 1; i < data[0].GetCount(); i++) {
				int in = 0, out = 0;
				for(int j = 0; j < sym_count; j++) {
					for(int k = 0; k < win_size; k++) {
						double d = data[j][i - k - 1];
						window.in[in++] = d;
					}
					window.out[out++] = data[j][i];
				}
				
				
				ses.GetNet().Backward(window);
				int r = ses.GetNet().Forward(true);
				
				if (r == RAN)
					running = false;
			}
		}
		
		
	}
	catch (Exc e) {
		LOG("Error: " + e);
	}
}
