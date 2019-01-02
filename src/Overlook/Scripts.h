#ifndef _Overlook_Scripts_h_
#define _Overlook_Scripts_h_

namespace Overlook {

void LoadSymbol(CoreList& cl_sym, int symbol, int tf);
void LoadNetSymbols(CoreList& cl_sym, int tf);
void LoadNets(CoreList& cl_net, int tf);
void LoadDataPriceInput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int windowsize);
void LoadDataPipOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int postpips_count);
void LoadDataVolatOutput(ConvNet::Session& ses, CoreList& cl_net, int begin, int count, int postpips_count);
void TrainSession(ConvNet::Session& ses, int iterations);

}

#endif
