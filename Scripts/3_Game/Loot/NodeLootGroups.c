// Loot groups DB

class TR_LootEntry
{
	string type;
	ref array<string> type_range;
	int    weight;

	// Spawn overrides
	float  MinHealth;
	float  MaxHealth;
	int    MinQuantity;
	int    MaxQuantity;

	ref array<string> Attachments;

	int    AttachCountMin;
	int    AttachCountMax;

	void TR_LootEntry()
	{
		weight       = 1;

		MinHealth    = -1.0;
		MaxHealth    = -1.0;
		MinQuantity  = -1;
		MaxQuantity  = -1;

		Attachments  = new array<string>;
		AttachCountMin = 0;
		AttachCountMax = 0;

		type_range   = null;
	}
}

class TR_LootGroupDef
{
	float chance;
	int   minItems;
	int   maxItems;
	ref array<ref TR_LootEntry> items;

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

	protected static void Normalize()
	{
		if (!s_File) return;

		foreach (string name, TR_LootGroupDef def : s_File.groups)
		{
			if (!def) continue;

			if (def.chance < 0.0) def.chance = 0.0;
			if (def.chance > 100.0) def.chance = 100.0; // we accept 0..1 or 0..100 at use site
			if (def.minItems < 0) def.minItems = 0;
			if (def.maxItems < def.minItems) def.maxItems = def.minItems;

			if (!def.items) def.items = new array<ref TR_LootEntry>;

			for (int i = def.items.Count() - 1; i >= 0; i--)
			{
				TR_LootEntry e = def.items.Get(i);
				if (!e)
				{
					def.items.Remove(i);
					continue;
				}

				bool hasSingle = (e.type != "");
				bool hasRange  = (e.type_range && e.type_range.Count() > 0);
				if (!hasSingle && !hasRange)
				{
					def.items.Remove(i);
					continue;
				}

				if (hasSingle && hasRange)
				{
					e.type_range = null;
				}

				if (e.weight < 0) e.weight = 0;

				if (!e.Attachments) e.Attachments = new array<string>;

				for (int k = e.Attachments.Count() - 1; k >= 0; k--)
				{
					if (e.Attachments.Get(k) == "") e.Attachments.Remove(k);
				}

				// Attachment pick bounds
				if (e.AttachCountMin < 0) e.AttachCountMin = 0;
				if (e.AttachCountMax < e.AttachCountMin) e.AttachCountMax = e.AttachCountMin;
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
			// Load from disk
			JsonFileLoader<TR_LootGroupsFile>.JsonLoadFile(path, s_File);
			Normalize();
			TR_Debug.Log("LootGroups loaded: " + s_File.groups.Count().ToString() + " group(s) from file.");
			return;
		}

		// Create a minimal default file the first time, to help server owners.
		ref TR_LootGroupsFile defFile = new TR_LootGroupsFile;

		ref TR_LootGroupDef general = new TR_LootGroupDef;
		{
			// Simple starter items
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
		}

		defFile.groups.Insert("general", general);
		s_File = defFile;

		Normalize();
		JsonFileLoader<TR_LootGroupsFile>.JsonSaveFile(path, s_File);
		TR_Debug.Log("LootGroups default created at: " + path);
	}

	static TR_LootEntry PickWeighted(array<ref TR_LootEntry> entries)
	{
		if (!entries || entries.Count() <= 0) return null;

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

	static TR_LootEntry WeightedPickEntry(array<ref TR_LootEntry> entries)
	{
		return PickWeighted(entries);
	}

	static string GetCooldownMessage(string name)
	{
		if (!s_File) Load();

		TR_LootGroupDef def = s_File.groups.Get(name);
		if (!def || !def.cooldownMessage || def.cooldownMessage.Count() == 0)
			return "";

		int idx = Math.RandomInt(0, def.cooldownMessage.Count());
		return def.cooldownMessage.Get(idx);
	}

	static TR_LootGroupDef Get(string name)
	{
		if (!s_File) Load();
		return s_File.groups.Get(name);
	}

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

// Resolve a concrete type from a TR_LootEntry ('type' or 'type_range')
static string TR_ResolveLootType(TR_LootEntry e)
{
	if (!e) return "";

	if (e.type != "") return e.type;

	if (e.type_range && e.type_range.Count() > 0)
	{
		int idx = Math.RandomInt(0, e.type_range.Count());
		return e.type_range.Get(idx);
	}

	return "";
}
