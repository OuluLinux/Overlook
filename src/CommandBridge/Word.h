#ifndef _CommandBridge_Word_h_
#define _CommandBridge_Word_h_

#include <RichEdit/RichEdit.h>
#include <PdfDraw/PdfDraw.h>
using namespace Upp;

#include "Images.h"



class Word : public ParentCtrl {
public:
	virtual void DragAndDrop(Point, PasteClip& d);
	virtual void FrameDragAndDrop(Point, PasteClip& d);

protected:
	RichEdit   editor;
	ToolBar    toolbar;
	StatusBar  statusbar;
	String     filename;

	static LRUList& lrufile() { static LRUList l; return l; }

	void Load(const String& filename);
	void OpenFile(const String& fn);
	void Open();
	void Save0();
	void Save();
	void SaveAs();
	void Print();
	void Pdf();
	void About();
	void Destroy();
	void AboutMenu(Bar& bar);
	void MainBar(Bar& bar);

public:
	typedef Word CLASSNAME;

	static void SerializeApp(Stream& s);
	
	void FileBar(Bar& bar);
	String GetTitle();
	
	Word();
	
	Callback WhenTitle;
	
};

void InitWord();

#endif
