#ifndef _Earworm_MediaPlayer_h_
#define _Earworm_MediaPlayer_h_

#include "Youtube.h"
#include "MPlayer.h"

class MediaPlayer {
	YouTubeDL ytdl;
	YouTubeSearch ytsearch;
	MPlayer mplayer;
	
	Mutex queue_mtx;
	Vector<String> queue, search_queue;
	Vector<bool> search_random;
	
	bool running, stopped;
	
protected:
	void AddQueue(String path);
	void Run();
	
public:
	typedef MediaPlayer CLASSNAME;
	MediaPlayer();
	
	void StoreThis() {StoreToFile(*this, ConfigFile("player.bin"));}
	void LoadThis() {LoadFromFile(*this, ConfigFile("player.bin"));}
	void Serialize(Stream& s) {s % queue % search_queue % search_random;}
	void Start();
	void Stop();
	void StopRunning() {running = false;}
	void RunDownloader();
	void Next();
	void AddSearchQueue(String search_str, bool random_result=false) {queue_mtx.Enter(); search_queue.Add(search_str); search_random.Add(random_result); queue_mtx.Leave();}
	void Reset();
	
	Callback WhenStopped;
	Callback WhenSearchQueueAlmostEmpty;
};

#endif
