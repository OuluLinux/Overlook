#include "Earworm.h"

MediaPlayer::MediaPlayer() {
	running = false;
	stopped = true;
	
	LoadThis();
	
	Thread::Start(THISBACK(RunDownloader));
}

void MediaPlayer::Start() {
	Stop();
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Run));
}

void MediaPlayer::Stop() {
	running = false;
	while (!stopped) Sleep(100);
}

void MediaPlayer::Reset() {
	Stop();
	queue.Clear();
	search_queue.Clear();
	search_random.Clear();
}

void MediaPlayer::Run() {
	
	while (!Thread::IsShutdownThreads() && running) {
		
		int count;
		
		queue_mtx.Enter();
		count = queue.GetCount();
		queue_mtx.Leave();
		
		for(;count && !Thread::IsShutdownThreads();) {
			Cout() << "count=" << count << "\n";
			Cout() << "Trying to play...\n";
			
			queue_mtx.Enter();
			StoreThis();
			String path = queue[0];
			queue.Remove(0);
			queue_mtx.Leave();
			
			mplayer.Play(path);
			WhenStopped();
			
			queue_mtx.Enter();
			count = queue.GetCount();
			queue_mtx.Leave();
		}
		
		if (search_queue.GetCount() < 2) {
			WhenSearchQueueAlmostEmpty();
		}
		
		Sleep(100);
	}
	
	stopped = true;
}

void MediaPlayer::RunDownloader() {
	while (!Thread::IsShutdownThreads()) {
		
		while (search_queue.GetCount()) {
			Next();
		}
		
		Sleep(100);
	}
}

void MediaPlayer::Next() {
	if (search_queue.IsEmpty()) return;
	
	queue_mtx.Enter();
	String search_str = search_queue[0];
	bool random_found = search_random[0];
	search_queue.Remove(0);
	search_random.Remove(0);
	queue_mtx.Leave();
	
	String file;
	file += search_str;
	file += ".mp4";
	file.Replace(" ", "_");
	file = ConfigFile(file);
	
	if (FileExists(file)) {
		AddQueue(file);
	} else {
		YouTubeSearch yt;
		yt.SetAvailabilityCheck(true, 3);
		yt.Search(search_str);
		if (yt.GetCount()) {
			String addr;
			
			if (!random_found) {
				addr = yt[0].GetAddress();
			} else {
				int random = Random(yt.GetCount());
				addr = yt[random].GetAddress();
				file.Replace(".mp4", IntStr(random) + ".mp4");
				Cout() << "MediaPlayer::Next: random " << random << " address " << addr << "\n";
			}
			
			Cout() << "MediaPlayer::Next: " << addr << "\n";
			
			if (ytdl.GetDownloading())
				return;
			
			ytdl.Download(file, addr, THISBACK(AddQueue));
		}
	}
}

void MediaPlayer::AddQueue(String path) {
	queue_mtx.Enter();
	
	Cout() << "AddQueue: " << path << "\n";
	queue.Add(path);
	
	queue_mtx.Leave();
	
	StoreThis();
}
