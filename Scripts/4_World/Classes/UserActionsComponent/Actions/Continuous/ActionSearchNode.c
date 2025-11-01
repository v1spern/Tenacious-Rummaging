// Action SearchNode

class ActionSearchNodeCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(3.0);
	}
}

class ActionSearchNode : ActionContinuousBase
{
	protected EffectSound m_RummageSfx;
	protected bool m_RummagePlayed;

	static string TR_ASN_Name(PlayerBase pb)
	{
		if (!pb || !pb.GetIdentity()) return "";
		return pb.GetIdentity().GetName();
	}

	static string TR_ASN_Steam(PlayerBase pb)
	{
		if (!pb || !pb.GetIdentity()) return "";
		return pb.GetIdentity().GetId();
	}

	static string TR_ASN_Type(Object o)
	{
		if (!o) return "";
		return o.GetType();
	}

	static int TR_ASN_ShortHash(string str)
	{
		int h = str.Hash();
		if (h < 0) h = -h;
		return h % 10000000;
	}

	static string TR_ASN_JoinCSV(TStringArray arr)
	{
	    string acc = "";
	    if (!arr) return acc;
	    int n = arr.Count();
	    int i = 0;
	    while (i < n)
	    {
	        if (i > 0) acc += ",";
	        acc += arr.Get(i);
	        i++;
	    }
	    return acc;
	}

	static void TR_ASN_LogSummary(PlayerBase pb, Object tgt, string nodeKey, string lootGroup, float chancePct, float rollPct, string outcome, int spawnedCount, string itemsCsv)
	{
		string n = TR_ASN_Name(pb);
		string s64 = TR_ASN_Steam(pb);
		string t = TR_ASN_Type(tgt);
		string k = nodeKey;
		string g = lootGroup;
		if (k == "") k = "<unknown>";
		if (g == "") g = "<unknown>";
		string c = chancePct.ToString();
		string r = rollPct.ToString();
		string sp = spawnedCount.ToString();
		int kh = TR_ASN_ShortHash(k);
		string khs = kh.ToString();

		string line = "[RummageResult] ";
		line += "player='";  line += n;    line += "' ";
		line += "steam64='"; line += s64;  line += "' ";
		line += "target='";  line += t;    line += "' ";
		line += "node='";    line += k;    line += "' ";
		line += "group='";   line += g;    line += "' ";
		line += "chance='";  line += c;    line += "' ";
		line += "roll='";    line += r;    line += "' ";
		line += "outcome='"; line += outcome; line += "' ";
		line += "spawned='"; line += sp;   line += "' ";
		line += "items='";   line += itemsCsv; line += "' ";
		line += "keyhash='"; line += khs;  line += "'";
		TR_Debug.Log(line);
	}

	void ActionSearchNode()
	{
		m_CallbackClass = ActionSearchNodeCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = TR_Constants.DEFAULT_RUMMAGE_PROMPT;
		m_RummagePlayed = false;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player) return false;
		if (player.GetItemInHands()) return false;
		if (!target) return false;

		Object obj = target.GetObject();
		Object par = target.GetParent();

		if (obj && TR_SearchNodeUtils.CanSearchNode(obj)) { TR_SearchNode def = TR_SearchNodesDb.Match(obj); string src; string __np = ""; string __lg = ""; if (def) { if (def.PromptText != "") __np = def.PromptText; __lg = def.LootGroup; } m_Text = TR_LootSettingsManager.ResolvePromptText(__np, __lg, src); return true; }
		if (par && TR_SearchNodeUtils.CanSearchNode(par)) { TR_SearchNode def2 = TR_SearchNodesDb.Match(par); string src2; string __np2 = ""; string __lg2 = ""; if (def2) { if (def2.PromptText != "") __np2 = def2.PromptText; __lg2 = def2.LootGroup; } m_Text = TR_LootSettingsManager.ResolvePromptText(__np2, __lg2, src2); return true; }

		House hObj = House.Cast(obj);
		House hPar = House.Cast(par);
		House h = hObj;
		if (!h) h = hPar;
		if (!h) return false;

		string tok = TR_Interior.ComputeFurnitureToken(player, h);
		string __lgI; string __tokI;
		if (TR_SearchNodesDb.ResolveInteriorMatchNear(h, tok, player.GetPosition(), 2.0, __lgI, __tokI))
		{
			string __srcI;
			string __npI = TR_SearchNodesDb.GetInteriorPromptFor(h, __tokI);
			m_Text = TR_LootSettingsManager.ResolvePromptText(__npI, __lgI, __srcI);
			return true;
		}

		return false;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem   = new CCINone;
		m_ConditionTarget = new CCTCursor(2.0);
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override string GetText()
	{
		return m_Text;
	}

	override bool AddActionJuncture(ActionData action_data) { return true; }

	override void OnStartClient(ActionData action_data)
	{
		if (!m_RummagePlayed)
		{
			PlayerSearchingSound(action_data);
			m_RummagePlayed = true;
		}
	}

	override void OnEndAnimationLoopClient(ActionData action_data)
	{
		super.OnEndAnimationLoopClient(action_data);
		StopSearchingSound(action_data);
		m_RummagePlayed = false;
	}

	override void OnEndClient(ActionData action_data)
	{
		super.OnEndClient(action_data);
		m_RummagePlayed = false;
	}

	void PlayerSearchingSound(ActionData action_data)
	{
		PlayerBase pb = PlayerBase.Cast(action_data.m_Player);
		if (!pb) return;

		array<string> sets = new array<string>;
		sets.Insert("trash_search_1_SoundSet");
		sets.Insert("trash_search_2_SoundSet");
		sets.Insert("trash_search_3_SoundSet");
		sets.Insert("trash_search_4_SoundSet");
		sets.Insert("trash_search_5_SoundSet");

		int idx = Math.RandomInt(0, sets.Count());
		TR_Audio.PlayExactSetOnPlayer(pb, sets.Get(idx));
	}

	void StopSearchingSound(ActionData action_data) { m_RummageSfx = null; }

	protected float Roll01FromPercent()
	{
		int r = Math.RandomIntInclusive(0, 100);
		return r * 0.01;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);
		PlayerBase player = PlayerBase.Cast(action_data.m_Player);
		if (!player || !action_data || !action_data.m_Target) return;

		Object obj = action_data.m_Target.GetObject();
		Object par = action_data.m_Target.GetParent();

		TR_SearchNode def = null;
		Object usedObj = null;

		if (obj) { def = TR_SearchNodesDb.Match(obj); if (def) usedObj = obj; }
		if (!def && par) { def = TR_SearchNodesDb.Match(par); if (def) usedObj = par; }

		if (def && usedObj)
		{
			SpawnLootResolved(player, usedObj, def);
			return;
		}

		House house = House.Cast(par);
		if (!house) house = House.Cast(obj);
		if (!house) return;

		string reqTok = TR_Interior.ServerComputeFurnitureToken(player, house);

		string matchedTok;
		string lootGroup;
		bool ok = TR_SearchNodesDb.ResolveInteriorMatchNear(house, reqTok, player.GetPosition(), 2.0, lootGroup, matchedTok);
		if (!ok || lootGroup == "")
			return;

		string cdKey = TR_Interior.BuildInteriorCooldownKey(house, matchedTok);
		SpawnLootResolvedAdvanced(player, house, cdKey, lootGroup, house.GetType(), "__interior_tok__");
	}

	protected void PlayLootFoundClientOrRPC(PlayerBase player)
	{
		if (!player) return;
		if (!GetGame().IsMultiplayer() || GetGame().IsClient())
		{
			TR_Audio.PlayRandomLootFound(player);
			return;
		}
		Param1<int> p = new Param1<int>(TR_Constants.TR_AUDIO_KIND_FOUND);
		GetGame().RPCSingleParam(player, TR_Constants.RPC_ID_TR_AUDIO, p, true, player.GetIdentity());
	}

	void SpawnLootResolved(PlayerBase player, Object usedObj, TR_SearchNode def)
	{
		if (!player || !usedObj || !def) return;
		string key = TR_SearchNodesDb.GetCooldownKeyFor(usedObj, def);
		SpawnLootResolvedAdvanced(player, usedObj, key, def.LootGroup, def.ClassName, ResolveModelForTarget(usedObj));
	}

	void SpawnLootResolvedAdvanced(PlayerBase player, Object usedObj, string cooldownKey, string lootCategory, string nodeClass, string nodeModel)
	{
		if (!player || !usedObj) return;

		TR_PlayerSearchLogger.Get().LogSearchAttemptEx(player, usedObj, cooldownKey, lootCategory, nodeClass, nodeModel);

		TR_LootGroupDef groupDef = TR_LootGroups.Get(lootCategory);
		if (!groupDef) return;

		float attemptPct = groupDef.chance;
		if (attemptPct <= 1.0) attemptPct *= 100.0;
		if (attemptPct < 0.0) attemptPct = 0.0;
		if (attemptPct > 100.0) attemptPct = 100.0;
		float attemptFrac = attemptPct / 100.0;

		float roll01 = 0.0;
		float rollPct = 0.0;

		float cooldownTime = TR_LootSettingsManager.GetCooldownForCategory(lootCategory);
		bool globalScope = TR_LootSettingsManager.IsCooldownGlobal();

		if (TR_NodeCooldownSystem.Get().IsOnCooldown(player, cooldownKey, cooldownTime, globalScope))
		{
			int remainingSec = TR_NodeCooldownSystem.Get().Remaining(player, cooldownKey, cooldownTime, globalScope);
			string cdMsg = TR_LootGroups.GetCooldownMessage(lootCategory);
			if (cdMsg == "") cdMsg = "This spot looks like it has been recently searched.";
			TR_Notify.Send(player, cdMsg);
			TR_ASN_LogSummary(player, usedObj, cooldownKey, lootCategory, attemptPct, rollPct, "COOLDOWN", 0, "");
			TR_PlayerSearchLogger.Get().LogCooldownBlockedEx(player, usedObj, cooldownKey, remainingSec, lootCategory, nodeClass, nodeModel);
			return;
		}

		TR_NodeCooldownSystem.Get().StartCooldown(player, cooldownKey, cooldownTime, globalScope);

		roll01 = Roll01FromPercent();
		rollPct = roll01 * 100.0;

		if (roll01 > attemptFrac)
		{
			TR_ASN_LogSummary(player, usedObj, cooldownKey, lootCategory, attemptPct, rollPct, "FAIL", 0, "");
			TR_Notify.Send(player, TR_LootSettingsManager.GetNothingFoundText());
			TR_RummageEventManager.TriggerOnFailedRummage(player, lootCategory, player.GetPosition());
			TryApplySearchHazard(player, lootCategory);
			return;
		}

		int spawnedCount = 0;
		int toSpawn = Math.RandomIntInclusive(groupDef.minItems, groupDef.maxItems);
		TStringArray spawnedTypes = new TStringArray;

		for (int i = 0; i < toSpawn; i++)
		{
			TR_LootEntry entry = TR_LootGroups.WeightedPickEntry(groupDef.items);
			if (!entry) continue;

			string __resolvedType = TR_ResolveLootType(entry);
			if (__resolvedType == "") continue;

			float offsetX = Math.RandomFloatInclusive(-0.3, 0.3);
			float offsetZ = Math.RandomFloatInclusive(-0.3, 0.3);

			int flags = ECE_PLACE_ON_SURFACE;

			EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx(__resolvedType, player.GetPosition(), flags));
			if (item)
			{
				item.SetPosition(item.GetPosition() + Vector(offsetX, 0, offsetZ));
				TR_LootRuntime.ApplyOverrides(ItemBase.Cast(item), entry);

				int pool = 0;
				if (entry.Attachments) pool = entry.Attachments.Count();
				if (pool > 0)
				{
					int minPick = entry.AttachCountMin;
					int maxPick = entry.AttachCountMax;

					if (minPick < 0) minPick = 0;
					if (maxPick < minPick) maxPick = minPick;
					if (maxPick > pool) maxPick = pool;

					int want = 0;
					if (maxPick >= minPick) want = Math.RandomIntInclusive(minPick, maxPick);
					if (want > pool) want = pool;

					array<int> idx = new array<int>;
					for (int t = 0; t < pool; t++) idx.Insert(t);

					for (int k = 0; k < want; k++)
					{
						if (idx.Count() <= 0) break;
						int pick = Math.RandomInt(0, idx.Count());
						int atIndex = idx.Get(pick);
						idx.Remove(pick);

						string attType = entry.Attachments.Get(atIndex);
						item.GetInventory().CreateAttachment(attType);
					}
				}

				spawnedTypes.Insert(__resolvedType);
				spawnedCount++;
				TR_PlayerSearchLogger.Get().LogLootEx(player, usedObj, cooldownKey, item, lootCategory, nodeClass, nodeModel);
			}
		}

		if (spawnedCount == 0)
		{
			TR_ASN_LogSummary(player, usedObj, cooldownKey, lootCategory, attemptPct, rollPct, "FAIL", 0, "");
			TR_Notify.Send(player, TR_LootSettingsManager.GetNothingFoundText());
			TR_RummageEventManager.TriggerOnFailedRummage(player, lootCategory, player.GetPosition());
			TryApplySearchHazard(player, lootCategory);
		}
		else
		{
			string itemsCsv = TR_ASN_JoinCSV(spawnedTypes);
			string __src; string __np3 = ""; string __lg3 = lootCategory; TR_SearchNode __def3b = TR_SearchNodesDb.Match(usedObj); if (__def3b) { if (__def3b.PromptText != "") __np3 = __def3b.PromptText; if (__def3b.LootGroup != "") __lg3 = __def3b.LootGroup; } string __pt = TR_LootSettingsManager.ResolvePromptText(__np3, __lg3, __src); PlayerIdentity __pid = player.GetIdentity(); string __nm = ""; if (__pid) __nm = __pid.GetName(); TR_Debug.Log("[PromptText] player=" + __nm + " level=" + __src + " group=" + lootCategory + " text='" + __pt + "'");
            TR_ASN_LogSummary(player, usedObj, cooldownKey, lootCategory, attemptPct, rollPct, "SUCCESS", spawnedCount, itemsCsv);
			TR_Notify.Send(player, TR_LootSettingsManager.GetLootFoundText(spawnedCount));
			PlayLootFoundClientOrRPC(player);
			TryApplySearchHazard(player, lootCategory);
		}
	}

	protected void TryApplySearchHazard(PlayerBase player, string category)
	{
		float chance = TR_LootSettingsManager.GetHazardChanceForCategory(category);
		if (chance <= 0.0) return;

		float roll = Math.RandomFloatInclusive(0.0, 100.0);
		if (roll > chance) return;

		ItemBase gloves = GetPlayerGloves(player);

		if (gloves)
		{
			float ghp = gloves.GetHealth();
			if (ghp > 0.0)
			{
				int dmg = TR_LootSettingsManager.GetRandomGloveDamageForCategory(category);
				if (dmg > 0)
				{
					gloves.AddHealth("", "", -dmg);
					TR_Notify.Send(player, TR_LootSettingsManager.GetHazardScrapeText());
				}
				return;
			}
			ApplyRandomBleedingWound(player);
			TR_Notify.Send(player, TR_LootSettingsManager.GetHazardRuinedGlovesCutText());
			return;
		}

		ApplyRandomBleedingWound(player);
		TR_Notify.Send(player, TR_LootSettingsManager.GetHazardNoGlovesCutText());
	}

	protected ItemBase GetPlayerGloves(PlayerBase player)
	{
		EntityAI attachment = player.FindAttachmentBySlotName("Gloves");
		if (attachment) return ItemBase.Cast(attachment);

		int ac = player.GetInventory().AttachmentCount();
		for (int i = 0; i < ac; i++)
		{
			EntityAI att = player.GetInventory().GetAttachmentFromIndex(i);
			if (!att) continue;
			string slot = att.ConfigGetString("inventorySlot");
			if (slot == "Gloves") return ItemBase.Cast(att);
		}
		return null;
	}

	protected void ApplyRandomBleedingWound(PlayerBase player)
	{
		TStringArray sels = new TStringArray;
		sels.Insert("LeftArm");
		sels.Insert("RightArm");
		int idx = Math.RandomInt(0, sels.Count());
		BleedingSourcesManagerServer bms = player.GetBleedingManagerServer();
		if (bms) { bms.AttemptAddBleedingSourceBySelection(sels.Get(idx)); return; }
		player.AddHealth("", "", -2);
	}

	protected string ResolveModelForTarget(Object target)
	{
		if (!target) return "";
		string model = target.GetModelName();
		if (model != "")
		{
			string low = model; low.ToLower();
			if (low != "unknown_p3d_file" && low != "unknown") return model;
		}

		string type = target.GetType();
		string cfgVeh = "CfgVehicles " + type + " model";
		if (GetGame().ConfigGetText(cfgVeh, model) && model != "") return model;

		string cfgNonAI = "CfgNonAIVehicles " + type + " model";
		if (GetGame().ConfigGetText(cfgNonAI, model) && model != "") return model;

		return "";
	}
}
