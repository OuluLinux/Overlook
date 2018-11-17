#include "MQL.h"

using namespace Ci;
using namespace Upp;

void Usage() {
	Cout() << "Usage:\n";
	Cout() << "    --help\n";
	Cout() << "    --version\n";
	Cout() << "    -l c|c99|java|cs|js|js-ta|as|d|pm|pm510\n";
	Cout() << "    -o outputfile\n";
	Cout() << "    -D define_variable\n";
	Cout() << "    -I include_dir\n";
}

CONSOLE_APP_MAIN {
	Index<String> pre_symbols;
	pre_symbols.Add("true");
	
	Vector<String> inputFiles;
	Vector<String> searchDirs;
	
	String lang;
	String outputFile;
	String namespace_;
	
	auto& args = CommandLine();
	for (int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if (arg[0] == '-') {
			if (arg == "--help") {
				Usage();
				return;
			}
			else if (arg == "--version") {
				Cout() << "cito 0.4.0";
				return;
			}
			else if (arg == "-l") {
				lang = args[++i];
			}
			else if (arg == "-o") {
				outputFile = args[++i];
			}
			else if (arg == "-n") {
				namespace_ = args[++i];
			}
			else if (arg == "-D") {
				String symbol = args[++i];
				if (symbol == "true" || symbol == "false")
					throw ArgumentException(symbol + " is reserved");
				pre_symbols.Add(symbol);
			}
			else if (arg == "-I") {
				searchDirs.Add(args[++i]);
			}
			
			throw ArgumentException("Unknown option: " + arg);
		}
		else {
			inputFiles.Add(arg);
		}
	}
	if (lang.IsEmpty() && !outputFile.IsEmpty()) {
		String ext = GetFileExt(outputFile);
		if (ext.GetCount() >= 2)
			lang = ext.Mid(1);
	}
	if (lang.IsEmpty() || outputFile.IsEmpty() || inputFiles.GetCount() == 0) {
		Usage();
		SetExitCode(1);
		return;
	}

	CiParser* parser = new CiParser();
	parser->pre_symbols <<= pre_symbols;
	for (String inputFile : inputFiles) {
		try {
			parser->Parse(inputFile, new StringReader(LoadFile(inputFile)));
		} catch (Exc ex) {
			Cerr() << (inputFile + ":" + IntStr(parser->input_line_no) + ": ERROR: " + ex);
			parser->PrintMacroStack();
			if (parser->current_method != NULL)
				Cerr() << "   in method " << parser->current_method->name;
			SetExitCode(1);
			return;
		}
	}
	CiProgram* program = parser->Program();

	CiResolver* resolver = new CiResolver();
	resolver->search_dirs <<= searchDirs;
	try {
		resolver->Resolve(program);
	} catch (Exc ex) {
		if (resolver->current_class != NULL) {
			Cerr() << resolver->current_class->source_filename;
			Cerr() << ": ";
		}
		Cerr() << "ERROR: " << ex;
		if (resolver->current_method != NULL)
			Cerr() << "   in method " << resolver->current_method->name;
		SetExitCode(1);
		return;
	}

	SourceGenerator* gen;
	if      (lang == "c")		gen = new GenC89();
	else if (lang == "c99")		gen = new GenC();
	else if (lang == "java")	gen = new GenJava(namespace_);
	else if (lang == "cs")		gen = new GenCs(namespace_);
	else if (lang == "js")		gen = new GenJs();
	else if (lang == "js-ta")	gen = new GenJsWithTypedArrays();
	else if (lang == "as")		gen = new GenAs(namespace_);
	else if (lang == "d")		gen = new GenD();
	else if (lang == "pm")		gen = new GenPerl58(namespace_);
	else if (lang == "pm510")	gen = new GenPerl510(namespace_);
	else throw ArgumentException("Unknown language: " + lang);
	
	gen->output_file = outputFile;
	gen->Write(program);
	
	delete gen;
	delete resolver;
	delete parser;
	
	return 0;
}

