/*
NodeLootGroups.c — TR v1.0.4d-dev
Changelog:
- Removed normalization backfill/write-back on existing LootGroups.json.
- Load() now only generates a default file when missing; it never alters an existing file.
- SaveManual() retained for explicit, manual writes (still normalizes before saving).
*/

class TR_LootEntry
{
	string type;
	ref array<string> type_range;
	int    weight;

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
	string promptText;
	int   minItems;
	int   maxItems;
	ref array<ref TR_LootEntry> items;

	ref array<string> cooldownMessage;

	void TR_LootGroupDef()
	{
		chance   = 1.0;
		promptText = "";
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
    protected static ref map<string, string> s_KeyLowerToKey;

    protected static string _NormKey(string name)
    {
        if (name == "") return "";
        string k = name;
        k.Trim();
        k.ToLower();
        return k;
    }

    protected static void _BuildLowerKeyIndex()
    {
        s_KeyLowerToKey = new map<string, string>();
        if (s_File && s_File.groups)
        {
            foreach (string key1, TR_LootGroupDef g : s_File.groups) {
                string lk = _NormKey(key1);
                if (lk != "") s_KeyLowerToKey.Set(lk, key1);
            }
        }
    }

	protected static bool Normalize()
	{
		bool mutated = false;
        if (!s_File) return false;
        if (!s_File.groups) return false;

		foreach (string name, TR_LootGroupDef def : s_File.groups)
		{
			if (!def) continue;

			if (def.chance < 0.0) { def.chance = 0.0; mutated = true; }
			if (def.chance > 100.0) { def.chance = 100.0; mutated = true; }
			if (def.minItems < 0) { def.minItems = 0; mutated = true; }
			if (def.maxItems < def.minItems) { def.maxItems = def.minItems; mutated = true; }

			if (!def.items) { def.items = new array<ref TR_LootEntry>; mutated = true; }

			for (int i = def.items.Count() - 1; i >= 0; i--)
			{
				TR_LootEntry e = def.items.Get(i);
				if (!e)
				{
					def.items.Remove(i);
					mutated = true;
					continue;
				}

				bool hasSingle = (e.type != "");
				bool hasRange  = (e.type_range && e.type_range.Count() > 0);
				if (!hasSingle && !hasRange)
				{
					def.items.Remove(i);
					mutated = true;
					continue;
				}

				if (hasSingle && hasRange)
				{
					e.type_range = null;
					mutated = true;
				}

				if (e.weight < 0) { e.weight = 0; mutated = true; }

				if (!e.Attachments) { e.Attachments = new array<string>; mutated = true; }

				for (int k = e.Attachments.Count() - 1; k >= 0; k--)
				{
					if (e.Attachments.Get(k) == "") { e.Attachments.Remove(k); mutated = true; }
				}

				if (e.AttachCountMin < 0) { e.AttachCountMin = 0; mutated = true; }
				if (e.AttachCountMax < e.AttachCountMin) { e.AttachCountMax = e.AttachCountMin; mutated = true; }
			}
		}
		return mutated;
	}

    protected static TR_LootGroupsFile _CreateDefault()
    {
        TR_LootGroupsFile defFile = new TR_LootGroupsFile;

        TR_LootGroupDef general = new TR_LootGroupDef;
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

        defFile.groups.Insert("general", general);
        return defFile;
    }

    protected static void _SaveTo(string path)
    {
        #ifdef SERVER
        if (!s_File) return;
        JsonFileLoader<TR_LootGroupsFile>.JsonSaveFile(path, s_File);
        TR_Debug.Log("[LootGroups] Wrote file '" + path + "'");
        #endif
    }

	static void Load()
	{
		s_File = new TR_LootGroupsFile();
		string path = TR_Constants.Path("LootGroups.json");
        string used = path;
		#ifndef SERVER
			string m1 = "$mission:TenaciousRummaging\\LootGroups.json";
			string m2 = "$mission:TenaciousRummaging/LootGroups.json";
			if (FileExist(m1)) { used = m1; }
			else if (FileExist(m2)) { used = m2; }
		#endif

		if (FileExist(used))
		{
			JsonFileLoader<TR_LootGroupsFile>.JsonLoadFile(used, s_File);
			_BuildLowerKeyIndex();
			int total = 0; int withPrompt = 0;
			if (s_File && s_File.groups)
			{
				total = s_File.groups.Count();
				foreach (string key2, TR_LootGroupDef gd2 : s_File.groups) {
					if (gd2 && gd2.promptText != "") withPrompt = withPrompt + 1;
				}
			}
			TR_Debug.Log("[LootGroups] Loaded file '" + used + "', counting '" + total.ToString() + "' groups and '" + withPrompt.ToString() + "' custom rummage prompts");
			// No normalization write-back on existing files.
		}
		else
		{
			#ifdef SERVER
			TR_Debug.Log("[LootGroups] Missing file '" + used + "', creating default");
			s_File = _CreateDefault();
			Normalize(); // safe to normalize defaults before first save
			_BuildLowerKeyIndex();
			_SaveTo(path);
			TR_Debug.Log("[LootGroups] Default created at: " + path);
			#else
			TR_Debug.Log("[LootGroups] Missing file '" + used + "'");
			#endif
		}
	}

	static void SaveManual()
	{
		string path = TR_Constants.Path("LootGroups.json");
		if (!s_File || !s_File.groups || s_File.groups.Count() == 0)
		{
			s_File = _CreateDefault();
		}
		Normalize();
		_BuildLowerKeyIndex();
		_SaveTo(path);
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

	static string GetPromptText(string name)
    {
        TR_LootGroupDef def = Get(name);
        if (!def && s_KeyLowerToKey)
        {
            string lk = _NormKey(name);
            if (s_KeyLowerToKey.Contains(lk))
                def = Get(s_KeyLowerToKey.Get(lk));
        }
        if (!def) return "";
        if (def.promptText != "") return def.promptText;
        return "";
    }

	static array<string> ListNames()
	{
		if (!s_File) Load();

		array<string> names = new array<string>;
		foreach (string k, TR_LootGroupDef def : s_File.groups) names.Insert(k);

		TR_Debug.Log("[LootGroups] '" + names.Count().ToString() + "' group(s) accounted for");
		return names;
	}

	static array<string> GetAllGroupNames()
	{
		return ListNames();
	}
}

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
