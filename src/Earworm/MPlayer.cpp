#include "Earworm.h"

MPlayer::MPlayer() {
	
}

void MPlayer::Play(String file) {
	
	Cout() << "MPlayer::Play: " << file << "\n";

#ifdef flagWIN32
	String cmd = AppendFileName(ConfigFile("mplayer"), "mplayer.exe") + " -vo null \"" + file + "\"";
#elif defined flagPOSIX
	String cmd = "mplayer -vo null \"" + file + "\"";
	//String cmd = "cvlc --vout none \"" + file + "\"";
#endif

	String out;
	
	Cout() << cmd << "\n";
	LOG(cmd);
	Sys(cmd, out);
	
	
}
