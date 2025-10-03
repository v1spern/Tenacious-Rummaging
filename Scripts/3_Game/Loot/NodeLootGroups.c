// Loot groups DB

class TR_LootEntry
{
	string type;
	int    weight;

	float  MinHealth;
	float  MaxHealth;
	int    MinQuantity;
	int    MaxQuantity;

	// Manual attachments (applied by script)
	ref array<string> Attachments;

	// Random selection bounds for Attachments (unique picks)
	int    AttachCountMin;
	int    AttachCountMax;

	void TR_LootEntry()
	{
		type = "";
		weight = 0;

		MinHealth = 0.00;
		MaxHealth = 1.00;
		MinQuantity = -1;
		MaxQuantity = -1;

		// Arrays: allocate so JSON omission is harmless
		Attachments = new array<string>;

		// Default: do not auto-attach anything unless configured
		AttachCountMin = 0;
		AttachCountMax = 0;
	}
}

class TR_LootGroupDef
{
	float chance;
	int   minItems;
	int   maxItems;
	ref array<ref TR_LootEntry> items;

	// Array of strings; one is chosen at random when cooldown blocks
	ref array<string> cooldownMessage;

	void TR_LootGroupDef()
	{
		chance   = 1.0;
		minItems = 1;
		maxItems = 1;
		items    = new array<ref TR_LootEntry>;
		cooldownMessage = new array<string>;
	}
}

class TR_LootGroupsFile
{
	ref map<string, ref TR_LootGroupDef> groups;
	void TR_LootGroupsFile() { groups = new map<string, ref TR_LootGroupDef>; }
}

class TR_LootGroups
{
	protected static ref TR_LootGroupsFile s_File;

	protected static void _Normalize()
	{
		if (!s_File) return;

		foreach (string name, TR_LootGroupDef def : s_File.groups)
		{
			if (!def) continue;

			if (def.chance < 0.0) def.chance = 0.0;
			if (def.chance > 1.0) def.chance = 1.0;
			if (def.minItems < 0) def.minItems = 0;
			if (def.maxItems < def.minItems) def.maxItems = def.minItems;

			if (!def.items) def.items = new array<ref TR_LootEntry>;
			if (!def.cooldownMessage) def.cooldownMessage = new array<string>;

			for (int ii = 0; ii < def.items.Count(); ii++)
			{
				TR_LootEntry e = def.items.Get(ii);
				if (!e) continue;

				// Ensure array exists
				if (!e.Attachments) e.Attachments = new array<string>;

				// Strip empty strings from attachments
				for (int k = e.Attachments.Count() - 1; k >= 0; k--)
				{
					if (e.Attachments.Get(k) == "") e.Attachments.Remove(k);
				}

				// Clamp attachment pick bounds
				if (e.AttachCountMin < 0) e.AttachCountMin = 0;
				if (e.AttachCountMax < e.AttachCountMin) e.AttachCountMax = e.AttachCountMin;
				// Do not clamp to list size here; we cap at spawn-time after load/edits.
			}
		}
	}

	static void Load()
	{
		if (!s_File) s_File = new TR_LootGroupsFile;

		TR_Constants.EnsureProfile();
		string path = TR_Constants.Path(TR_Constants.FILE_LOOT_GROUPS);

		if (FileExist(path))
		{
			JsonFileLoader<TR_LootGroupsFile>.JsonLoadFile(path, s_File);
		}
		else
		{
			// Seed minimal defaults (ASCII-only content)
			TR_LootGroupDef general = new TR_LootGroupDef;
			general.chance   = 0.70;
			general.minItems = 1;
			general.maxItems = 2;

			TR_LootEntry a = new TR_LootEntry;
			a.type = "Rag"; a.weight = 50; a.MinHealth = 0.70; a.MaxHealth = 1.00; a.MinQuantity = 2; a.MaxQuantity = 6;
			general.items.Insert(a);

			TR_LootEntry b = new TR_LootEntry;
			b.type = "Matchbox"; b.weight = 25; b.MinHealth = 0.20; b.MaxHealth = 0.80; b.MinQuantity = 1; b.MaxQuantity = 12;
			general.items.Insert(b);

			TR_LootEntry c = new TR_LootEntry;
			c.type = "DuctTape"; c.weight = 25; c.MinHealth = 0.80; c.MaxHealth = 1.00; c.MinQuantity = -1; c.MaxQuantity = -1;
			general.items.Insert(c);

			general.cooldownMessage.Insert("This spot looks like it has been recently searched.");
			general.cooldownMessage.Insert("Fresh scuff marks - someone beat you to it.");
			general.cooldownMessage.Insert("Too tidy. Someone picked through here already.");
			s_File.groups.Set("general", general);

			TR_LootGroupDef misc = new TR_LootGroupDef;
			misc.chance   = 0.50;
			misc.minItems = 1;
			misc.maxItems = 3;

			TR_LootEntry m1 = new TR_LootEntry;
			m1.type = "Paper"; m1.weight = 30; m1.MinHealth = 0.20; m1.MaxHealth = 0.80; m1.MinQuantity = -1; m1.MaxQuantity = -1;
			misc.items.Insert(m1);

			TR_LootEntry m2 = new TR_LootEntry;
			m2.type = "Bone"; m2.weight = 20; m2.MinHealth = 0.15; m2.MaxHealth = 0.80; m2.MinQuantity = 1; m2.MaxQuantity = 3;
			misc.items.Insert(m2);

			TR_LootEntry m3 = new TR_LootEntry;
			m3.type = "Nail"; m3.weight = 20; m3.MinHealth = 0.30; m3.MaxHealth = 1.00; m3.MinQuantity = 5; m3.MaxQuantity = 20;
			misc.items.Insert(m3);

			TR_LootEntry m4 = new TR_LootEntry;
			m4.type = "WaterBottle"; m4.weight = 15; m4.MinHealth = 0.10; m4.MaxHealth = 0.70; m4.MinQuantity = 0; m4.MaxQuantity = 50;
			misc.items.Insert(m4);

			misc.cooldownMessage.Insert("Fresh feathers and footprints - this coop was checked moments ago.");
			misc.cooldownMessage.Insert("Empty scraps and a bent latch. Someone just searched here.");
			misc.cooldownMessage.Insert("You spot disturbed dust; it's been picked clean.");
			s_File.groups.Set("misc", misc);

			_Normalize();
			JsonFileLoader<TR_LootGroupsFile>.JsonSaveFile(path, s_File);
		}

		_Normalize();

		// Debug list
		array<string> groupNames = new array<string>;
		foreach (string gname, TR_LootGroupDef def : s_File.groups) groupNames.Insert(gname);
		groupNames.Sort();
		TR_Debug.Log("LootGroups loaded with groups: " + groupNames.ToString());
	}

	// Weighted pick for loot entries
	static TR_LootEntry PickWeighted(ref array<ref TR_LootEntry> entries)
	{
		if (!entries || entries.Count() == 0) return null;

		int total = 0;
		for (int i = 0; i < entries.Count(); i++)
		{
			TR_LootEntry e = entries.Get(i);
			if (!e) continue;
			if (e.weight < 0) e.weight = 0;
			total += e.weight;
		}
		if (total <= 0) return null;

		int rnd = Math.RandomInt(0, total);
		int acc = 0;
		for (int j = 0; j < entries.Count(); j++)
		{
			TR_LootEntry x = entries.Get(j);
			if (!x) continue;
			int w = x.weight; if (w < 0) w = 0;
			acc += w;
			if (rnd < acc) return x;
		}
		return null;
	}

	// Alias to avoid breakage with prior references.
	static TR_LootEntry WeightedPickEntry(ref array<ref TR_LootEntry> entries)
	{
		return PickWeighted(entries);
	}

	// Randomly pick a cooldown message from the group (or empty if none)
	static string GetCooldownMessage(string name)
	{
		TR_LootGroupDef def = Get(name);
		if (!def) return "";

		if (!def.cooldownMessage || def.cooldownMessage.Count() == 0) return "";

		int idx = Math.RandomInt(0, def.cooldownMessage.Count());
		if (idx < 0) idx = 0;
		if (idx >= def.cooldownMessage.Count()) idx = def.cooldownMessage.Count() - 1;

		return def.cooldownMessage.Get(idx);
	}

	static TR_LootGroupDef Get(string name)
	{
		if (!s_File) Load();
		TR_LootGroupDef def;
		if (s_File.groups.Find(name, def)) return def;
		if (s_File.groups.Find("general", def)) return def; // fallback
		return null;
	}

	// Existing name used internally
	static array<string> ListNames()
	{
		if (!s_File) Load();

		array<string> names = new array<string>;
		foreach (string k, TR_LootGroupDef def : s_File.groups) names.Insert(k);

		TR_Debug.Log("LootGroups contains " + names.Count().ToString() + " group(s).");
		return names;
	}

	static array<string> GetAllGroupNames()
	{
		return ListNames();
	}
}
