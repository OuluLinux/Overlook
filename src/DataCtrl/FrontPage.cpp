#include "DataCtrl.h"
#include <plugin/jpg/jpg.h>

namespace DataCtrl {

FrontPageDraw::FrontPageDraw(FrontPage* fp) : fp(fp) {
	
}

bool FrontPageDraw::HasCacheFile(String path) {
	if (FileExists(path))
		return true;
	
	// Try to download file
	path = ConfigFile("pics") + DIR_SEPS + GetFileName(path);
	
	return FileExists(path);
}

void FrontPageDraw::MakeBackground() {
	String sym = fp->GetSymbol();
	
	String pic_dir = ConfigFile("pics");
	RealizeDirectory(pic_dir);
	String path = AppendFileName(pic_dir, sym + ".jpg");
	path.Replace("#", "");
	
	
	if (!HasCacheFile(path)) {
		String a = sym.Left(3);
		String b = sym.Right(3);
		String path_a = AppendFileName(pic_dir, a + ".jpg");
		String path_b = AppendFileName(pic_dir, b + ".jpg");
		
		if (HasCacheFile(path_a) && HasCacheFile(path_b)) {
			ImageBuffer id(1280, 720);
			
			Size sz(1280, 720);
			
			Image left = StreamRaster::LoadFileAny(path_a);
			Image right = StreamRaster::LoadFileAny(path_b);
			
			
			// Resize left by height
			Size left_sz = left.GetSize();
			int y_fac = left_sz.cy * 100000 / sz.cy;
			Size left_new_sz = left_sz * 100000 / y_fac;
			left = RescaleFilter(left, left_new_sz, 1);
			
			// Resize right by height
			Size right_sz = right.GetSize();
			y_fac = right_sz.cy * 100000 / sz.cy;
			Size right_new_sz = right_sz * 100000 / y_fac;
			right = RescaleFilter(right, right_new_sz, 1);
			
			for(int y = 0; y < 720; y++) {
				RGBA*       dest = id.Begin()		+ 1280 * y + left_new_sz.cx - 1;
				const RGBA* src  = left.Begin()	+ left_new_sz.cx * y;
				for(int x = 0; x < left_new_sz.cx; x++) {
					*dest = *src;
					dest--;
					src++;
				}
			}
			
			for(int y = 0; y < 720; y++) {
				RGBA*       dest = id.Begin()			+ 1280 * (y+1) - 1;
				const RGBA* src  = right.Begin()		+ right_new_sz.cx * (y+1) - 1;
				for(int x = 0; x < right_new_sz.cx && x < (1280/2 + 320/2); x++) {
					// Normal copying
					if (x < (1280/2-320/2)) {
						*dest = *src;
					}
					// Fading effect
					else {
						//int i = (x - (1280/2-320/2)) * 100000 / 320;
						int i = (1 - cos(((double)x - (1280./2.-320./2.)) / 320. * M_PI)) * 100000 / 2;
						int r = (dest->r * i + src->r * (100000 - i)) / 100000;
						int g = (dest->g * i + src->g * (100000 - i)) / 100000;
						int b = (dest->b * i + src->b * (100000 - i)) / 100000;
						dest->r = r;
						dest->g = g;
						dest->b = b;
					}
					dest--;
					src--;
				}
			}
			
			bg = id;
			
			JPGEncoder jpg;
			jpg.SaveFile(path, bg);
		}
	} else {
		bg = StreamRaster::LoadFileAny(path);
	}
	
}


void DrawRect(ImageBuffer& buf, Rect r, Color c, int tp) {
	tp = 100000 - tp;
	
	Size sz = buf.GetSize();
	
	if (r.left < 0) r.left = 0;
	if (r.top < 0) r.top = 0;
	if (r.right >= sz.cx) r.right = sz.cx;
	if (r.bottom >= sz.cy) r.bottom = sz.cx;
	
	Size rsz = r.GetSize();
	
	int R = c.GetR();
	int G = c.GetG();
	int B = c.GetB();
	
	for(int i = 0; i < rsz.cy; i++) {
		RGBA* dest = buf.Begin() + (r.top + i) * sz.cx + r.left;
		for(int j = 0; j < rsz.cx; j++, dest++) {
			int r = (dest->r * tp + R * (100000 - tp)) / 100000;
			int g = (dest->g * tp + G * (100000 - tp)) / 100000;
			int b = (dest->b * tp + B * (100000 - tp)) / 100000;
			dest->r = r;
			dest->g = g;
			dest->b = b;
		}
	}
	
}

void FrontPageDraw::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, Black);
	
	ImageBuffer buf(sz);
	if (bg.GetSize().cx) {
		Size bg_sz = bg.GetSize();
		Size new_sz;
		
		int x_fac = bg_sz.cx * 100000 / sz.cx;
		int y_fac = bg_sz.cy * 100000 / sz.cy;
		if (x_fac < y_fac)
			new_sz = bg_sz * 100000 / x_fac;
		else
			new_sz = bg_sz * 100000 / y_fac;
		
		Image img = Crop(RescaleFilter(bg, new_sz, 1), -(sz.cx/2 - new_sz.cx/2), -(sz.cy/2 - new_sz.cy/2), sz.cx, sz.cy);
		buf = img;
	}
	
	Rect title_rect(RectC(10, 10, sz.cx-10, 30));
	DrawRect(buf, title_rect, White(), 75000);
	
	
	
	
	d.DrawImage(0,0,buf);
	
	// Title
	String sym = fp->GetSymbol();
	
	Font fnt = SansSerif(26);
	d.DrawText(title_rect.left + 5, title_rect.top + 0, sym, fnt, White());
	d.DrawText(title_rect.left + 7, title_rect.top + 2, sym, fnt, Black());
	
	/*
	MT4& mt4 = fp->sp->ft->mt4;
	int id = fp->sp->GetId();
	
	Rect adv_rect(RectC(10, 10+30+10, sz.cx / 2 - 20, sz.cy / 2 - 20 - 30));
	DrawRect(buf, adv_rect, White(), 75000);
	
	double price = mt4.GetAskFast(id);
	String price_str = DblStr(price);
	Size price_sz = GetTextSize(price_str, fnt);
	d.DrawText(sz.cx/2 - price_sz.cx/2, title_rect.top + 0, price_str, fnt, White());
	d.DrawText(sz.cx/2 - price_sz.cx/2+2, title_rect.top + 2, price_str, fnt, Black());
	
	const Vector<MT4Order>& orders = mt4.orders[id];
	String s;
	if (orders.GetCount()) {
		double profit = 0;
		for(int i = 0; i < orders.GetCount(); i++)
			profit += orders[i].profit;
		if (orders[0].type == 0)
			s += "Bought ";
		else
			s += "Sold ";
		if (profit > 0) s.Cat('+');
		s += DblStr(profit);
	} else {
		s += "Not open";
	}
	Size open_sz = GetTextSize(s, fnt);
	d.DrawText(title_rect.right - open_sz.cx - 5, title_rect.top + 0, s, fnt, White());
	d.DrawText(title_rect.right - open_sz.cx - 3, title_rect.top + 2, s, fnt, Black());

	
	// Advices
	Vector<Advice> advices = fp->sp->GetAdvices();
	
	AdviceSortSingleValue sorter(AdviceSortSingleValue::GAIN);
	Sort(advices, sorter);
	
	int h = 15;
	Font fnt_txt = SansSerif(h);
	
	for(int i = 0; i < advices.GetCount() && i < 10; i++) {
		Advice& adv = advices[i];
		
		String s = adv.AsString();
		d.DrawText(adv_rect.left + 5,     adv_rect.top + i*h, s, fnt_txt, White());
		d.DrawText(adv_rect.left + 5 + 1, adv_rect.top + i*h + 1, s, fnt_txt, Black());
		
	}
	*/
	
}

Image FrontPageDraw::AdviceList(int cx, int cy) {
	/*ImageDraw draw(cx, cy);
	
	draw.DrawRect(0,0, cx, cy, White);
	
	Vector<Advice> advices = fp->sp->GetAdvices();
	
	AdviceSortSingleValue sorter(AdviceSortSingleValue::GAIN);
	Sort(advices, sorter);
	
	int h = 15;
	Font fnt = SansSerif(h);
	
	for(int i = 0; i < advices.GetCount() && i < 3; i++) {
		Advice& adv = advices[i];
		
		String s = adv.AsString();
		draw.DrawText(5,     i*h, s, fnt, Black());
		draw.DrawText(5 + 1, i*h + 1, s, fnt, GrayColor(64));
		
	}
	
	
	ImageBuffer src_buf(draw);
	ImageBuffer buf(cx, cy);
	const RGBA* src = src_buf.Begin();
	RGBA* cur = buf.Begin();
	RGBA* end = buf.End();
	for (; cur != end; cur++, src++) {
		*cur = *src;
		cur->a = 128+64;
	}
	
	return buf;*/
	return Image();
}

FrontPage::FrontPage() : draw(this) {
	Add(draw.VSizePos().HSizePos());
	
}

void FrontPage::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	
	i = args.Find("id");
	ASSERTREF(i != -1);
	id = args[i];
	
	i = args.Find("symbol");
	ASSERTREF(i != -1);
	symbol = args[i];
}
	
void FrontPage::Init() {
	
	SetId(id);
	
	draw.MakeBackground();
}

}
