#include "Earworm.h"

Earworm::Earworm()
{
	CtrlLayout(*this, "Earworm");
	
	playing = 0;
	random_song.Set(true);
	random_foundlist.Set(true);
	
	lists.AddColumn("Title");
	lists.WhenCursor = THISBACK(RefreshSongs);
	
	songs.AddColumn("Year");
	songs.AddColumn("Artist");
	songs.AddColumn("Title");
	songs.WhenLeftClick = THISBACK(Play);
	songs.ColumnWidths("1 3 3");
	
	play.WhenAction = THISBACK(RefreshState);
	play.SetLabel("Play");
	
	mediaplayer.WhenStopped = THISBACK(PostPlayNext);
	
	RefreshCharts();
}

void Earworm::RefreshCharts() {
	lists.Clear();
	for(int i = 0; i < scraper.GetChartCount(); i++) {
		lists.Add(scraper.GetChart(i).GetName());
	}
	if (lists.GetCount()) lists.SetCursor(0);
}

void Earworm::RefreshSongs() {
	int cur = lists.GetCursor();
	SingleChart& chart = scraper.GetChart(cur);
	
	songs.Clear();
	int year_end = GetSysTime().year - 1;
	for(int i = chart.GetBeginYear(); i < year_end; i++) {
		int count = chart.GetUniqueCount(i);
		for(int j = 0; j < count; j++) {
			ChartSong song = chart.GetUniqueSong(i, j);
			songs.Add(song.year, song.artist, song.title);
		}
	}
}

void Earworm::Play() {
	if (playing) return;
	
	mediaplayer.Reset();
	
	QueueSelected();
	
	play.WhenAction.Clear();
	play.Set(true);
	play.WhenAction = THISBACK(RefreshState);
}

void Earworm::RefreshState() {
	/*playing = this->play.Get();
	if (!playing)
		mediaplayer.StopRunning();
	else
		mediaplayer.Start();*/
}

void Earworm::QueueSelected() {
	int chart_id = lists.GetCursor();
	SingleChart& chart = scraper.GetChart(chart_id);
	
	int cur = songs.GetCursor();
	if (cur < 0 || cur >= songs.GetCount()) return;
	
	String search_str;
	search_str << songs.Get(cur, 1) << " " << songs.Get(cur, 2);
	
	Cout() << "Searching " << search_str << " with random=" << (int)random_foundlist.Get() << "\n";
	mediaplayer.AddSearchQueue(search_str, random_foundlist.Get());
	mediaplayer.Start();
}

void Earworm::PlayNext() {
	Cout() << "Playing next\n";
	
	bool random_list = this->random_list.Get();
	bool random_song = random_list ? true : this->random_song.Get();
	
	if (random_list) {
		int i = Random(lists.GetCount());
		lists.SetCursor(i);
		RefreshSongs();
	}
	
	if (random_song) {
		int i = Random(songs.GetCount());
		if (i >= 0 && i < songs.GetCount()) {
			songs.SetCursor(i);
			QueueSelected();
		}
	}
	else {
		int i = songs.GetCursor();
		i++;
		if (i >= songs.GetCount())
			i = 0;
		if (i >= 0 && i < songs.GetCount()) {
			songs.SetCursor(i);
			QueueSelected();
		}
	}
}
