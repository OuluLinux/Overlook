#ifndef _Earworm_Earworm_h
#define _Earworm_Earworm_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include "Scaper.h"
#include "MPlayer.h"
#include "Youtube.h"
#include "MediaPlayer.h"

#define LAYOUTFILE <Earworm/Earworm.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS Imgs
#define IMAGEFILE <Earworm/Earworm.iml>
#include <Draw/iml_header.h>


class Earworm : public WithEarwormLayout<TopWindow> {
	MediaPlayer mediaplayer;
	Scraper scraper;
	Vector<String> opt_strs;
	bool playing;
	
public:
	typedef Earworm CLASSNAME;
	Earworm();
	
	void InitSongs();
	void RefreshCharts();
	void RefreshSongs();
	void Play();
	void QueueSelected();
	void PlayNext();
	void PostPlayNext() {PostCallback(THISBACK(PlayNext));}
	void RefreshState();
	
};

#endif
