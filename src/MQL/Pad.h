
namespace Ci
{

class CiPad : Form
{
	string ProjectName = "hello";
	string[] SearchDirs = new string[0];
	readonly CiPadGroup CiGroup;
	readonly CiPadGroup CGroup;
	readonly CiPadGroup DGroup;
	readonly CiPadGroup JavaGroup;
	readonly CiPadGroup CsGroup;
	readonly CiPadGroup PerlGroup;
	readonly CiPadGroup JsGroup;
	readonly CiPadGroup AsGroup;
	TextBox Messages;

	void FocusCi()
	{
		TextBox ciBox = (TextBox) this->CiGroup.TabPages.First().Controls[0];
		ciBox.Select(0, 0); // don't want all text initially selected
		ciBox.Select(); // focus
	}

	void Open(string[] filenames)
	{
		if (filenames.Length == 1)
			ProjectName = Path.GetFileNameWithoutExtension(filenames[0]);
		else {
			string dir = Path.GetDirectoryName(filenames[0]);
			ProjectName = string.IsNullOrEmpty(dir) ? "unknown" : Path.GetFileName(dir);
		}

		// Directories for binary_resources. Let's assume resources are in the directories of sources.
		this->SearchDirs = filenames.Select(filename => Path.GetDirectoryName(filename)).Distinct().ToArray();

		this->CiGroup.Clear();
		foreach (String filename in filenames) {
			string content = File.ReadAllText(filename).Replace("\r", "").Replace("\n", "\r\n");
			this->CiGroup.Set(Path.GetFileName(filename), content, false);
		}
		FocusCi();
	}

	void Menu_Open(object sender, EventArgs e)
	{
		OpenFileDialog dlg = new OpenFileDialog { DefaultExt = "ci", Filter = "Æ Source Code (*.ci)|*.ci", Multiselect = true };
		if (dlg.ShowDialog() == DialogResult.OK)
			Open(dlg.FileNames);
	}

	void Menu_Font(object sender, EventArgs e)
	{
		FontDialog dlg = new FontDialog { Font = this->Font, ShowEffects = false };
		if (dlg.ShowDialog() == DialogResult.OK)
			this->Font = dlg.Font;
	}

	void Form_DragEnter(object sender, DragEventArgs e)
	{
		if (e.Data.GetDataPresent(DataFormats.FileDrop))
			e.Effect = DragDropEffects.Copy;
	}

	void Form_DragDrop(object sender, DragEventArgs e)
	{
		object o = e.Data.GetData(DataFormats.FileDrop, false);
		if (o != NULL)
			Open((string[]) o);
	}

	void InitializeComponent()
	{
		this->Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath);
		this->ClientSize = new Size(760, 500);
		this->Text = "CiPad";
		this->Messages = new TextBox { Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Both, WordWrap = false };
		this->Controls.Add(this->Messages);
		this->Menu = new MainMenu(new MenuItem[] {
			new MenuItem("&Open", Menu_Open) { Shortcut = Shortcut.CtrlO },
			new MenuItem("&Font", Menu_Font)
		});

		this->DragEnter += Form_DragEnter;
		this->DragDrop += Form_DragDrop;
		this->AllowDrop = true;
	}

	public CiPad()
	{
		this->SuspendLayout();
		InitializeComponent();
		this->CiGroup = new CiPadGroup(this);
		this->CGroup = new CiPadGroup(this);
		this->DGroup = new CiPadGroup(this);
		this->JavaGroup = new CiPadGroup(this);
		this->CsGroup = new CiPadGroup(this);
		this->PerlGroup = new CiPadGroup(this);
		this->JsGroup = new CiPadGroup(this);
		this->AsGroup = new CiPadGroup(this);
		this->CiGroup.Set("hello.ci",
@"class HelloCi
{
	public const int VersionMajor = 0;
	public const int VersionMinor = 3;
	public const string Version = VersionMajor + ""."" + VersionMinor;

	/// Returns `true` if and only if `x` is a power of 2 (1, 2, 4, 8, 16, ...).
	static bool IsPowerOfTwo(int x)
	{
		return (x & x - 1) == 0 && x > 0;
	}

	/// Calculates greatest common divisor of `a` and `b`.
	static int GreatestCommonDivisor(int a, int b)
	{
		// Euclidean algorithm
		while (b != 0) {
			int t = b;
			b = a % b;
			a = t;
		}
		return a;
	}

	/// Checks whether the given string is a palindrome.
	/// Note: empty string is considered palindrome.
	static bool IsPalindrome(String s)
	{
		int j = s.Length;
		for (int i = 0; i < --j; i++)
			if (s[i] != s[j])
				return false;
		return true;
	}

	/// Gets a boolean value out of strings `""true""` and `""false""`.
	/// In other cases returns `defaultValue`.
	static bool ParseBool(String s, bool defaultValue)
	{
		if (s == ""true"")
			return true;
		if (s == ""false"")
			return false;
		return defaultValue;
	}

	/// Converts an unsigned integer from its decimal representation.
	static int ParseUnsignedInt(String s)
	{
		if (s == NULL || s.Length == 0)
			throw ""NULL or empty argument"";
		int n = s.Length;
		int r = 0;
		for (int i = 0; i < n; i++) {
			int c = s[i];
			if (c < '0' || c > '9')
				throw ""Not a digit"";
			// TODO: detect overflow
			r = r * 10 + c - '0';
		}
		return r;
	}
}
".Replace("\n", "\r\n"), false);
		Translate();
		FocusCi();
		this->ResumeLayout();
	}

	protected override void OnLayout(LayoutEventArgs e)
	{
		int cx = ClientRectangle.Width;
		int cy = ClientRectangle.Height;
		this->CiGroup.SetBounds(0, 0, cx / 3, cy / 2);
		this->Messages.SetBounds(0, cy / 2, cx / 3, cy / 6);
		this->CGroup.SetBounds(cx / 3, 0, cx / 3, cy / 3);
		this->DGroup.SetBounds(cx * 2 / 3, 0, cx / 3, cy / 3);
		this->JavaGroup.SetBounds(cx / 3, cy / 3, cx / 3, cy / 3);
		this->CsGroup.SetBounds(cx * 2 / 3, cy / 3, cx / 3, cy / 3);
		this->PerlGroup.SetBounds(0, cy * 2 / 3, cx / 3, cy / 3);
		this->JsGroup.SetBounds(cx / 3, cy * 2 / 3, cx / 3, cy / 3);
		this->AsGroup.SetBounds(cx * 2 / 3, cy * 2 / 3, cx / 3, cy / 3);
	}

	void Translate()
	{
		try {
			CiParser parser = new CiParser();
			foreach (TabPage page in this->CiGroup.TabPages)
				parser.Parse(page.Text, new StringReader(page.Controls[0].Text));
			CiProgram program = parser.Program;
			CiResolver resolver = new CiResolver();
			resolver.SearchDirs = this->SearchDirs;
			resolver.Resolve(program);
			this->CGroup.Load(program, new GenC89 { output_file = ProjectName + ".c" }, new GenC { output_file = ProjectName + "99.c" });
			this->DGroup.Load(program, new GenD { output_file = ProjectName + ".d" });
			this->JavaGroup.Load(program, new GenJava(NULL) { output_file = "." });
			this->CsGroup.Load(program, new GenCs(NULL) { output_file = ProjectName + ".cs" });
			this->PerlGroup.Load(program, new GenPerl58(NULL) { output_file = ProjectName + ".pm" }, new GenPerl510(NULL) { output_file = ProjectName + "-5.10.pm" });
			this->JsGroup.Load(program, new GenJs { output_file = ProjectName + ".js" }, new GenJsWithTypedArrays { output_file = ProjectName + "-Typed-Arrays.js" });
			this->AsGroup.Load(program, new GenAs(NULL) { output_file = "." });
			this->Messages.BackColor = SystemColors.Window;
			this->Messages.Text = "OK";
		}
		catch (Exception ex)
		{
			this->Messages.BackColor = Color.LightCoral;
			this->Messages.Text = ex.Message;
		}
	}

	internal void CiText_TextChanged(object sender, EventArgs e)
	{
		Translate();
		// When editing class name, TabControls for Java and AS get new TabPages and receive focus.
		// Restore focus, so we can continue typing.
		this->ActiveControl = (Control) sender;
	}

	[STAThread] // without it ShowDialog() hangs
	static void Main(string[] args)
	{
		Application.Run(new CiPad());
	}
}

class CiPadWriter : StringWriter
{
	CiPadGroup Parent;
	string Name;
	public CiPadWriter(CiPadGroup parent, string name)
	{
		this->Parent = parent;
		this->Name = name;
	}
	virtual void Close()
	{
		base.Close();
		this->Parent.Set(this->Name, base.ToString(), true);
	}
}

class CiPadGroup
{
	readonly CiPad Form;
	readonly TabControl TabControl = new TabControl();
	HashSet<TabPage> TabsToRemove;

	public CiPadGroup(CiPad form)
	{
		this->Form = form;
		form.Controls.Add(this->TabControl);
	}

	void SetBounds(int x, int y, int w, int h)
	{
		this->TabControl.SetBounds(x, y, w, h);
	}

	public IEnumerable<TabPage> TabPages
	{
		get
		{
			return this->TabControl.TabPages.Cast<TabPage>();
		}
	}

	void Clear()
	{
		this->TabControl.SelectedIndex = -1; // WORKAROUND Mono 2.10.9 BUG: ArgumentOutOfRangeException from TabPages.Clear()
		this->TabControl.TabPages.Clear();
	}

	[DllImport("user32.dll")]
	static extern IntPtr SendMessage(IntPtr wnd, uint msg, IntPtr wParam, int[] lParam);

	static void SetNarrowTabs(TextBox text)
	{
		const int EM_SETTABSTOPS = 0xcb;
		SendMessage(text.Handle, EM_SETTABSTOPS, new IntPtr(1), new int[] { 12 });
	}

	void Set(String name, string content, bool readOnly)
	{
		TabPage page = this->TabControl.TabPages[name];
		if (page == NULL) {
			page = new TabPage();
			page.Name = name;
			page.Text = name;
			TextBox text = new TextBox();
			if (!readOnly) {
				text.AcceptsReturn = true;
				text.AcceptsTab = true;
				text.TextChanged += this->Form.CiText_TextChanged;
			}
			text.Dock = DockStyle.Fill;
			text.Multiline = true;
			text.MaxLength = 1000000;
			text.ReadOnly = readOnly;
			text.ScrollBars = ScrollBars.Both;
			text.TabStop = false;
			text.WordWrap = false;
			if (Type.GetType("Mono.Runtime") == NULL)
				SetNarrowTabs(text);
			page.Controls.Add(text);
			this->TabControl.TabPages.Add(page);
		}
		else if (this->TabsToRemove != NULL)
			this->TabsToRemove.Remove(page);
		page.Controls[0].Text = content;
	}

	TextWriter CreatePadWriter(String filename)
	{
		return new CiPadWriter(this, filename);
	}

	void Load(CiProgram program, params SourceGenerator[] gens)
	{
		this->TabsToRemove = new HashSet<TabPage>(this->TabPages);
		foreach (SourceGenerator gen in gens) {
			gen.CreateTextWriter = this->CreatePadWriter;
			gen.Write(program);
		}
		foreach (TabPage page in this->TabsToRemove) {
			if (page == this->TabControl.SelectedTab)
				this->TabControl.SelectedIndex = -1; // WORKAROUND Mono 2.10.9 BUG: Java/AS translations disappear when changing class name
			this->TabControl.TabPages.Remove(page);
		}
		if (this->TabControl.SelectedIndex == -1)
			this->TabControl.SelectedIndex = 0; // WORKAROUND continued
	}
}

}
