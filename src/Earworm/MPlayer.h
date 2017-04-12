#ifndef _Earworm_MPlayer_h_
#define _Earworm_MPlayer_h_

#include <Core/Core.h>
using namespace Upp;


class MPlayer {
	Array<LocalProcess> procs;
	
	
public:
	MPlayer();
	
	void Play(String file);
	
};

#endif
