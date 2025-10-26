// Player Search Logger

class TR_PlayerSearchLogger
{
	protected static ref TR_PlayerSearchLogger s_Instance;

	protected string m_LogPath;
	protected string m_CsvPath;
	protected bool   m_CsvEnabled;

	static TR_PlayerSearchLogger Get()
	{
		if (!s_Instance) s_Instance = new TR_PlayerSearchLogger();
		return s_Instance;
	}

	void TR_PlayerSearchLogger()
	{
		MakeDirectory("$profile:TenaciousRummaging");
		MakeDirectory("$profile:TenaciousRummaging/Logs");

		m_LogPath    = "$profile:TenaciousRummaging/Logs/PlayerSearches.log";
		m_CsvPath    = "$profile:TenaciousRummaging/Logs/PlayerSearches.csv";
		m_CsvEnabled = TR_LootSettingsManager.IsCsvLoggingEnabled();
	}

	void RefreshCsvEnabled()
	{
		m_CsvEnabled = TR_LootSettingsManager.IsCsvLoggingEnabled();
	}

	string NowIsoUtc()
	{
		int y; int mo; int d; int h; int mi; int s;
		GetYearMonthDayUTC(y, mo, d);
		GetHourMinuteSecondUTC(h, mi, s);
		return ZeroPad(y, 4) + "-" + ZeroPad(mo, 2) + "-" + ZeroPad(d, 2) + " " + ZeroPad(h, 2) + ":" + ZeroPad(mi, 2) + ":" + ZeroPad(s, 2) + "Z";
	}

	int NowMs()
	{
		if (GetGame()) return GetGame().GetTime();
		return 0;
	}

	void LogSearchAttempt(PlayerBase player, Object target, string nodeKey)
	{
		LogSearchAttemptEx(player, target, nodeKey, "", "", "");
	}

	void LogNoLoot(PlayerBase player, Object target, string nodeKey, float roll01, float chance01)
	{
		LogNoLootEx(player, target, nodeKey, roll01, chance01, "", "", "");
	}

	void LogLoot(PlayerBase player, Object target, string nodeKey, EntityAI item)
	{
		LogLootEx(player, target, nodeKey, item, "", "", "");
	}

	void LogSearchAttemptEx(PlayerBase player, Object target, string nodeKey, string lootCategory, string nodeClass, string nodeModel)
	{
		string ts = NowIsoUtc();

		string pName;
		string pSteam64;
		GetPlayerIdentityParts(player, pName, pSteam64);

		string tgtType;
		int tx; int ty; int tz;
		DescribeTargetParts(target, tgtType, tx, ty, tz);

		if (nodeModel == "" || IsUnknownModel(nodeModel))
		{
			nodeModel = ResolveModelForTarget(target);
			if (nodeModel == "" || IsUnknownModel(nodeModel))
			{
				nodeModel = ExtractModelFromKey(nodeKey);
			}
		}

		string line = "[" + ts + "] SEARCH_ATTEMPT";
		line = line + " player=" + pName;
		line = line + " steam64=" + pSteam64;
		line = line + " key=" + nodeKey;
		line = line + " lootcat=" + lootCategory;
		line = line + " node_class=" + nodeClass;
		line = line + " node_model=" + nodeModel;
		line = line + " target=" + tgtType + "@" + tx.ToString() + "," + ty.ToString() + "," + tz.ToString();
		WriteLine(line);
	}

	void LogNoLootEx(PlayerBase player, Object target, string nodeKey, float roll01, float chance01, string lootCategory, string nodeClass, string nodeModel)
	{
		string ts = NowIsoUtc();

		string pName;
		string pSteam64;
		GetPlayerIdentityParts(player, pName, pSteam64);

		string tgtType;
		int tx; int ty; int tz;
		DescribeTargetParts(target, tgtType, tx, ty, tz);

		if (nodeModel == "" || IsUnknownModel(nodeModel))
		{
			nodeModel = ResolveModelForTarget(target);
			if (nodeModel == "" || IsUnknownModel(nodeModel))
			{
				nodeModel = ExtractModelFromKey(nodeKey);
			}
		}

		string line = "[" + ts + "] NO_LOOT";
		line = line + " player=" + pName;
		line = line + " steam64=" + pSteam64;
		line = line + " key=" + nodeKey;
		line = line + " roll=" + Float01Str(roll01);
		line = line + " chance=" + Float01Str(chance01);
		line = line + " lootcat=" + lootCategory;
		line = line + " node_class=" + nodeClass;
		line = line + " node_model=" + nodeModel;
		line = line + " target=" + tgtType + "@" + tx.ToString() + "," + ty.ToString() + "," + tz.ToString();
		WriteLine(line);
	}

	void LogLootEx(PlayerBase player, Object target, string nodeKey, EntityAI item, string lootCategory, string nodeClass, string nodeModel)
	{
		string ts = NowIsoUtc();

		string pName;
		string pSteam64;
		GetPlayerIdentityParts(player, pName, pSteam64);

		string tgtType;
		int tx; int ty; int tz;
		DescribeTargetParts(target, tgtType, tx, ty, tz);

		if (nodeModel == "" || IsUnknownModel(nodeModel))
		{
			nodeModel = ResolveModelForTarget(target);
			if (nodeModel == "" || IsUnknownModel(nodeModel))
			{
				nodeModel = ExtractModelFromKey(nodeKey);
			}
		}

		string itemClass = "null";
		int qty = -1;
		float h01 = -1.0;
		string cond = "Unknown";
		string loc = "unknown";

		if (item)
		{
			itemClass = item.GetType();
			h01 = item.GetHealth01("", "");
			cond = ConditionFrom01(h01);
			loc = GetItemLocation(item);

			ItemBase ib = ItemBase.Cast(item);
			if (ib && ib.HasQuantity())
			{
				float qf = ib.GetQuantity();
				qty = Math.Round(qf);
			}
		}

		string line = "[" + ts + "] LOOT";
		line = line + " player=" + pName;
		line = line + " steam64=" + pSteam64;
		line = line + " key=" + nodeKey;
		line = line + " item=" + itemClass;
		line = line + " qty=" + qty.ToString();
		line = line + " health01=" + Float01Str(h01);
		line = line + " condition=" + cond;
		line = line + " loc=" + loc;
		line = line + " lootcat=" + lootCategory;
		line = line + " node_class=" + nodeClass;
		line = line + " node_model=" + nodeModel;
		line = line + " target=" + tgtType + "@" + tx.ToString() + "," + ty.ToString() + "," + tz.ToString();
		WriteLine(line);
	}

	void LogCooldownBlockedEx(PlayerBase player, Object target, string nodeKey, int remainingSec, string lootCategory, string nodeClass, string nodeModel)
	{
		string ts = NowIsoUtc();

		string pName;
		string pSteam64;
		GetPlayerIdentityParts(player, pName, pSteam64);

		string tgtType;
		int tx; int ty; int tz;
		DescribeTargetParts(target, tgtType, tx, ty, tz);

		if (nodeModel == "" || IsUnknownModel(nodeModel))
		{
			nodeModel = ResolveModelForTarget(target);
			if (nodeModel == "" || IsUnknownModel(nodeModel))
			{
				nodeModel = ExtractModelFromKey(nodeKey);
			}
		}

		if (remainingSec < 0) remainingSec = 0;

		string line = "[" + ts + "] COOLDOWN_BLOCKED";
		line = line + " player=" + pName;
		line = line + " steam64=" + pSteam64;
		line = line + " key=" + nodeKey;
		line = line + " remaining_s=" + remainingSec.ToString();
		line = line + " lootcat=" + lootCategory;
		line = line + " node_class=" + nodeClass;
		line = line + " node_model=" + nodeModel;
		line = line + " target=" + tgtType + "@" + tx.ToString() + "," + ty.ToString() + "," + tz.ToString();
		WriteLine(line);
	}

	void LogCooldownBlocked(PlayerBase player, Object target, string nodeKey, int remainingSec)
	{
		LogCooldownBlockedEx(player, target, nodeKey, remainingSec, "", "", "");
	}

	protected void GetPlayerIdentityParts(PlayerBase player, out string name, out string steam64)
	{
		name = "unknown";
		steam64 = "unknown";
		if (player)
		{
			PlayerIdentity id = player.GetIdentity();
			if (id)
			{
				name = id.GetName();
				steam64 = id.GetPlainId();
				if (steam64 == "")
				{
					steam64 = id.GetId();
				}
			}
		}
	}

	protected void DescribeTargetParts(Object target, out string typeName, out int x, out int y, out int z)
	{
		typeName = "null";
		x = 0;
		y = 0;
		z = 0;
		if (target)
		{
			typeName = target.GetType();
			vector p = target.GetPosition();
			x = Math.Round(p[0]);
			y = Math.Round(p[1]);
			z = Math.Round(p[2]);
		}
	}

	protected bool IsUnknownModel(string model)
	{
		if (model == "") return true;
		string low = model;
		low.ToLower();
		if (low == "unknown") return true;
		if (low == "unknown_p3d_file") return true;
		if (low == "<unknown>") return true;
		return false;
	}

	protected string ResolveModelForTarget(Object target)
	{
		if (!target) return "";

		string model = target.GetModelName();
		if (model != "" && !IsUnknownModel(model))
		{
			return model;
		}

		string type = target.GetType();

		string cfgVeh = "CfgVehicles " + type + " model";
		bool gotVeh = GetGame().ConfigGetText(cfgVeh, model);
		if (gotVeh && model != "")
		{
			return model;
		}

		string cfgNonAI = "CfgNonAIVehicles " + type + " model";
		bool gotNonAI = GetGame().ConfigGetText(cfgNonAI, model);
		if (gotNonAI && model != "")
		{
			return model;
		}

		return "";
	}

	protected string ExtractModelFromKey(string key)
	{
		if (key == "") return "";
	
		int a = key.IndexOf(":");
		if (a < 0) return "";
	
		int tail1Start = a + 1;
		string tail1 = key.Substring(tail1Start, key.Length() - tail1Start);
		int bRel = tail1.IndexOf(":");
		if (bRel < 0) return "";
		int b = tail1Start + bRel;
	
		int tail2Start = b + 1;
		string tail2 = key.Substring(tail2Start, key.Length() - tail2Start);
		int cRel = tail2.IndexOf(":");
		int c = -1;
		if (cRel >= 0) c = tail2Start + cRel;
	
		int startPos = b + 1;
		int endPos;
		if (c >= 0) endPos = c;
		else endPos = key.Length();
	
		int len = endPos - startPos;
		if (len <= 0) return "";
	
		return key.Substring(startPos, len);
	}
	
	protected string GetItemLocation(EntityAI item)
	{
		if (!item) return "unknown";
		if (item.GetHierarchyParent()) return "inventory";
		return "world";
	}

	protected string ConditionFrom01(float h01)
	{
		if (h01 <= 0.0) return "Ruined";
		if (h01 < 0.30) return "BadlyDamaged";
		if (h01 < 0.50) return "Damaged";
		if (h01 < 0.95) return "Worn";
		return "Pristine";
	}

	protected string ZeroPad(int v, int width)
	{
		string s = v.ToString();
		while (s.Length() < width)
		{
			s = "0" + s;
		}
		return s;
	}

	protected string Float01Str(float v)
	{
		if (v < 0.0) v = 0.0;
		if (v > 1.0) v = 1.0;
		int milli = Math.Round(v * 1000.0);
		int whole = milli / 1000;
		int frac = milli % 1000;
		return whole.ToString() + "." + ZeroPad(frac, 3);
	}

	protected void WriteLine(string line)
	{
		FileHandle fh = OpenFile(m_LogPath, FileMode.APPEND);
		if (fh)
		{
			FPrint(fh, line + "\n");
			CloseFile(fh);
		}

		if (m_CsvEnabled)
		{
			WriteCsvMirror(line);
		}
	}

	protected void WriteCsvMirror(string raw)
	{
		string s = raw;
		s.Replace("\r", " ");
		s.Replace("\n", " ");

		string ts = "";
		string ev = "";
		string rest = s;

		int rb = s.IndexOf("]");
		if (s.Length() > 2 && s.Substring(0, 1) == "[" && rb > 0)
		{
			ts = s.Substring(1, rb - 1);
			int evStart = rb + 2;
			if (evStart < s.Length())
			{
				string tail = s.Substring(evStart, s.Length() - evStart);
				int spRel = tail.IndexOf(" ");
				if (spRel > 0)
				{
					ev = tail.Substring(0, spRel);
					rest = tail.Substring(spRel + 1, tail.Length() - (spRel + 1));
				}
				else
				{
					ev = tail;
					rest = "";
				}
			}
		}

		// Defaults
		string player = "";
		string steam64 = "";
		string key = "";
		string lootcat = "";
		string node_class = "";
		string node_model = "";
		string target = "";
		string tgtType = "";
		string txs = "";
		string tys = "";
		string tzs = "";
		string item = "";
		string qty = "";
		string health01 = "";
		string condition = "";
		string loc = "";
		string roll = "";
		string chance = "";
		string remaining_s = "";

		int i = 0;
		while (i < rest.Length())
		{
			string tail2 = rest.Substring(i, rest.Length() - i);
			int spaceRel = tail2.IndexOf(" ");
			string tok;
			if (spaceRel >= 0)
			{
				tok = tail2.Substring(0, spaceRel);
				i = i + spaceRel + 1;
			}
			else
			{
				tok = tail2;
				i = rest.Length();
			}

			int eq = tok.IndexOf("=");
			if (eq <= 0) continue;

			string k = tok.Substring(0, eq);
			string v = tok.Substring(eq + 1, tok.Length() - (eq + 1));

			if (k == "player") player = v;
			else if (k == "steam64") steam64 = v;
			else if (k == "key") key = v;
			else if (k == "lootcat") lootcat = v;
			else if (k == "node_class") node_class = v;
			else if (k == "node_model") node_model = v;
			else if (k == "item") item = v;
			else if (k == "qty") qty = v;
			else if (k == "health01") health01 = v;
			else if (k == "condition") condition = v;
			else if (k == "loc") loc = v;
			else if (k == "roll") roll = v;
			else if (k == "chance") chance = v;
			else if (k == "remaining_s") remaining_s = v;
			else if (k == "target")
			{
				target = v;
				int at = v.IndexOf("@");
				if (at > 0)
				{
					tgtType = v.Substring(0, at);
					string coords = v.Substring(at + 1, v.Length() - (at + 1));
					int c1 = coords.IndexOf(",");
					int c2 = -1;
					if (c1 >= 0)
					{
						string coordsTail = coords.Substring(c1 + 1, coords.Length() - (c1 + 1));
						int c2rel = coordsTail.IndexOf(",");
						if (c2rel >= 0) c2 = c1 + 1 + c2rel;
					}
					if (c1 >= 0) txs = coords.Substring(0, c1);
					if (c1 >= 0 && c2 > c1) tys = coords.Substring(c1 + 1, c2 - (c1 + 1));
					if (c2 > 0) tzs = coords.Substring(c2 + 1, coords.Length() - (c2 + 1));
				}
				else
				{
					tgtType = v;
				}
			}
		}

		// CSV stuff
		string dq = "\"";
		ref array<string> cols = new array<string>();
		string qv;

		qv = ts; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = ev; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = player; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = steam64; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = key; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = lootcat; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = node_class; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = node_model; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = tgtType; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = txs; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = tys; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = tzs; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = item; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = qty; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = health01; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = condition; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = loc; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = roll; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = chance; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);
		qv = remaining_s; qv.Replace(dq, dq + dq); cols.Insert(dq + qv + dq);

		string line = "";
		for (int idx = 0; idx < cols.Count(); idx++)
		{
			if (idx > 0) line = line + ",";
			line = line + cols.Get(idx);
		}

		FileHandle fh = OpenFile(m_CsvPath, FileMode.APPEND);
		if (fh)
		{
			FPrint(fh, line + "\n");
			CloseFile(fh);
		}
	}

}

class TR_LootSettingsCsvSwitch { bool EnableCsvLogging = false; }