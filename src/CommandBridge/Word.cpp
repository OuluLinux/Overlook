#include "Word.h"


FileSel& WordFs()
{
	static FileSel fs;
	return fs;
}

FileSel& PdfFs()
{
	static FileSel fs;
	return fs;
}

void Word::FileBar(Bar& bar)
{
	/*bar.Add("New", CtrlImg::new_doc(), THISBACK(New))
	   .Key(K_CTRL_N)
	   .Help("Open new window");*/
	bar.Add("Open..", CtrlImg::open(), THISBACK(Open))
	   .Key(K_CTRL_O)
	   .Help("Open existing document");
	bar.Add(editor.IsModified(), "Save", CtrlImg::save(), THISBACK(Save))
	   .Key(K_CTRL_S)
	   .Help("Save current document");
	bar.Add("SaveAs", CtrlImg::save_as(), THISBACK(SaveAs))
	   .Help("Save current document with a new name");
	bar.ToolGap();
	bar.MenuSeparator();
	bar.Add("Print..", CtrlImg::print(), THISBACK(Print))
	   .Key(K_CTRL_P)
	   .Help("Print document");
	bar.Add("Export to PDF..", CommandBridgeImg::pdf(), THISBACK(Pdf))
	   .Help("Export document to PDF file");
	if(bar.IsMenuBar()) {
		if(lrufile().GetCount())
			lrufile()(bar, THISBACK(OpenFile));
		/*bar.Separator();
		bar.Add("Exit", THISBACK(Destroy));*/
	}
}


bool IsRTF(const char *fn)
{
	return ToLower(GetFileExt(fn)) == ".rtf";
}

String Word::GetTitle() {
	if (filename.IsEmpty())
		return "unnamed.qtf";
	return GetFileName(filename);
}

void Word::Load(const String& name)
{
	lrufile().NewEntry(name);
	if(IsRTF(name))
		editor.Pick(ParseRTF(LoadFile(name)));
	else
		editor.SetQTF(LoadFile(name));
	filename = name;
	editor.ClearModify();
	WhenTitle();
}

void Word::OpenFile(const String& fn)
{
	if(filename.IsEmpty() && !editor.IsModified())
		Load(fn);
	else
		(new Word)->Load(fn);
}

void Word::Open()
{
	FileSel& fs = WordFs();
	if(fs.ExecuteOpen())
		OpenFile(fs);
	else
		statusbar.Temporary("Loading aborted.");
}

void Word::DragAndDrop(Point, PasteClip& d)
{
	if(IsAvailableFiles(d)) {
		Vector<String> fn = GetFiles(d);
		for(int open = 0; open < 2; open++) {
			for(int i = 0; i < fn.GetCount(); i++) {
				String ext = GetFileExt(fn[i]);
				if(FileExists(fn[i]) && (ext == ".rtf" || ext == ".qtf")) {
					if(open)
						OpenFile(fn[i]);
					else {
						if(d.Accept())
							break;
						return;
					}
				}
			}
			if(!d.IsAccepted())
				return;
		}
	}
}

void Word::FrameDragAndDrop(Point p, PasteClip& d)
{
	DragAndDrop(p, d);
}

void Word::Save0()
{
	lrufile().NewEntry(filename);
	if(filename.IsEmpty())
		SaveAs();
	else {
		if(SaveFile(filename, IsRTF(filename) ? EncodeRTF(editor.Get()) : editor.GetQTF())) {
			statusbar.Temporary("File " + filename + " was saved.");
			ClearModify();
		}
		else
			Exclamation("Error saving the file [* " + DeQtf(filename) + "]!");
	}
}

void Word::Save()
{
	if(!editor.IsModified()) return;
	Save0();
}

void Word::SaveAs()
{
	FileSel& fs = WordFs();
	if(fs.ExecuteSaveAs()) {
		filename = fs;
		WhenTitle();
		Save0();
	}
}

void Word::Print()
{
	editor.Print();
}

void Word::Pdf()
{
	FileSel& fs = PdfFs();
	if(!fs.ExecuteSaveAs("Output PDF file"))
		return;
	Size page = Size(3968, 6074);
	PdfDraw pdf;
	UPP::Print(pdf, editor.Get(), page);
	SaveFile(~fs, pdf.Finish());
}

void Word::About()
{
	PromptOK("[A5 uWord]&Using [*^www://upp.sf.net^ Ultimate`+`+] technology.");
}

void Word::Destroy()
{
	if(editor.IsModified()) {
		switch(PromptYesNoCancel("Do you want to save the changes to the document?")) {
		case 1:
			Save();
			break;
		case -1:
			return;
		}
	}
	delete this;
}

void Word::MainBar(Bar& bar)
{
	FileBar(bar);
	bar.Separator();
	editor.DefaultBar(bar);
}

Word::Word()
{
	AddFrame(TopSeparatorFrame());
	AddFrame(toolbar);
	AddFrame(statusbar);
	Add(editor.SizePos());
	static int doc;
	editor.ClearModify();
	toolbar.Set(THISBACK(MainBar));
}

void Word::SerializeApp(Stream& s)
{
	int version = 1;
	s / version;
	s % WordFs()
	  % PdfFs();
	if(version >= 1)
		s % lrufile();
}


void InitWord() {
	
	WordFs().Type("QTF files", "*.qtf")
	         .Type("RTF files", "*.rtf")
	         .AllFilesType()
	         .DefaultExt("qtf");
	PdfFs().Type("PDF files", "*.pdf")
	       .AllFilesType()
	       .DefaultExt("pdf");
}

