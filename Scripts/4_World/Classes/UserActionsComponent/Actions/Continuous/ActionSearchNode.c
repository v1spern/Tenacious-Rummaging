// Player rummage action

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

	void ActionSearchNode()
	{
		m_CallbackClass = ActionSearchNodeCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = "Rummage here for loot";
		m_RummagePlayed = false;
		TR_Debug.Log("[Search Node] ActionSearchNode constructed");
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player) return false;
		if (player.GetItemInHands()) return false;
		if (!target) return false;

		// Static / Class-Wide
		Object obj = target.GetObject();
		Object par = target.GetParent();

		if (obj && TR_SearchNodeUtils.CanSearchNode(obj)) return true;
		if (par && TR_SearchNodeUtils.CanSearchNode(par)) return true;

		// Interior
		House hObj = House.Cast(obj);
		House hPar = House.Cast(par);
		House h = hObj;
		if (!h) h = hPar;
		if (!h) return false;

		string tok = TR_Interior.ComputeFurnitureToken(player, h); // quantized
		if (TR_SearchNodesDb.HasInteriorPieceNear(h, tok, player.GetPosition(), 2.0))
			return true;

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

	// Client stuff
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

	// Audio helpers
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

		// Static / Class-wide path
		TR_SearchNode def = null;
		Object usedObj = null;

		if (obj) { def = TR_SearchNodesDb.Match(obj); if (def) usedObj = obj; }
		if (!def && par) { def = TR_SearchNodesDb.Match(par); if (def) usedObj = par; }

		if (def && usedObj)
		{
			SpawnLootResolved(player, usedObj, def);
			return;
		}

		// Interior, range-aware resolver
		House house = House.Cast(par);
		if (!house) house = House.Cast(obj);
		if (!house) return;

		string reqTok = TR_Interior.ServerComputeFurnitureToken(player, house);

		string matchedTok;
		string lootGroup;
		bool ok = TR_SearchNodesDb.ResolveInteriorMatchNear(house, reqTok, player.GetPosition(), 2.0, lootGroup, matchedTok);
		if (!ok || lootGroup == "")
		{
			TR_Debug.Log("[Search Node] Interior no-match near: house=" + house.GetType() + " reqTok=" + reqTok);
			return;
		}

		string cdKey = TR_Interior.BuildInteriorCooldownKey(house, matchedTok);
		SpawnLootResolvedAdvanced(player, house, cdKey, lootGroup, house.GetType(), "__interior_tok__");
	}

	// Existing helper paths
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

		float cooldownTime = TR_LootSettingsManager.GetCooldownForCategory(lootCategory);
		bool globalScope = TR_LootSettingsManager.IsCooldownGlobal();

		if (TR_NodeCooldownSystem.Get().IsOnCooldown(player, cooldownKey, cooldownTime, globalScope))
		{
			int remainingSec = TR_NodeCooldownSystem.Get().Remaining(player, cooldownKey, cooldownTime, globalScope);
			string cdMsg = TR_LootGroups.GetCooldownMessage(lootCategory);
			if (cdMsg == "") cdMsg = "This spot looks like it has been recently searched.";
			player.MessageStatus(cdMsg);
			TR_PlayerSearchLogger.Get().LogCooldownBlockedEx(player, usedObj, cooldownKey, remainingSec, lootCategory, nodeClass, nodeModel);
			return;
		}

		TR_LootGroupDef groupDef = TR_LootGroups.Get(lootCategory);
		if (!groupDef) return;

		float attemptPct = groupDef.chance;
		if (attemptPct <= 1.0) attemptPct *= 100.0;
		if (attemptPct < 0.0) attemptPct = 0.0;
		if (attemptPct > 100.0) attemptPct = 100.0;
		float attemptFrac = attemptPct / 100.0;

		TR_NodeCooldownSystem.Get().StartCooldown(player, cooldownKey, cooldownTime, globalScope);

		float roll01 = Roll01FromPercent();
		if (roll01 > attemptFrac)
		{
			player.MessageStatus("You found nothing.");
			TR_RummageEventManager.TriggerOnFailedRummage(player, lootCategory, player.GetPosition());
			TryApplySearchHazard(player, lootCategory);
			return;
		}

		int spawnedCount = 0;
		int toSpawn = Math.RandomIntInclusive(groupDef.minItems, groupDef.maxItems);

		for (int i = 0; i < toSpawn; i++)
		{
			TR_LootEntry entry = TR_LootGroups.WeightedPickEntry(groupDef.items);
			if (!entry) continue;

			float offsetX = Math.RandomFloatInclusive(-0.3, 0.3);
			float offsetZ = Math.RandomFloatInclusive(-0.3, 0.3);

			int flags = ECE_PLACE_ON_SURFACE;

			// Spawn at player's current position (avoid parser issues) then scatter
			EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx(entry.type, player.GetPosition(), flags));
			if (item)
			{
				item.SetPosition(item.GetPosition() + Vector(offsetX, 0, offsetZ));

				// Apply standard overrides (health/quantity)
				TR_LootRuntime.ApplyOverrides(ItemBase.Cast(item), entry);

				// --- Manual attachments with random count ---
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

					// Build an index bag 0..pool-1
					array<int> idx = new array<int>;
					for (int t = 0; t < pool; t++) idx.Insert(t);

					// Pick 'want' unique attachments by random removing
					for (int k = 0; k < want; k++)
					{
						if (idx.Count() <= 0) break;
						int pick = Math.RandomInt(0, idx.Count());
						int atIndex = idx.Get(pick);
						idx.Remove(pick);

						string attType = entry.Attachments.Get(atIndex);
						EntityAI att = item.GetInventory().CreateAttachment(attType);
						if (att)
							TR_Debug.Log("[ItemSpawnProperties] +Attachment OK item=" + entry.type + " -> " + attType);
						else
							TR_Debug.Log("[ItemSpawnProperties] +Attachment FAIL item=" + entry.type + " -> " + attType);
					}
				}

				// Log resulting attachments
				int ac = item.GetInventory().AttachmentCount();
				if (ac > 0)
				{
					array<string> attNames = new array<string>;
					for (int ai2 = 0; ai2 < ac; ai2++)
					{
						EntityAI a2 = item.GetInventory().GetAttachmentFromIndex(ai2);
						if (a2) attNames.Insert(a2.GetType());
					}
					TR_Debug.Log("[ItemSpawnProperties] Attached count=" + ac.ToString() + " on " + entry.type + " -> " + attNames.ToString());
				}
				else
				{
					TR_Debug.Log("[ItemSpawnProperties] Attached count=0 on " + entry.type);
				}

				spawnedCount++;
				TR_PlayerSearchLogger.Get().LogLootEx(player, usedObj, cooldownKey, item, lootCategory, nodeClass, nodeModel);
			}
		}

		if (spawnedCount == 0)
		{
			player.MessageStatus("You found nothing.");
			TR_RummageEventManager.TriggerOnFailedRummage(player, lootCategory, player.GetPosition());
			TryApplySearchHazard(player, lootCategory);
		}
		else
		{
			player.MessageStatus("You find some stuff and put it beside you on the ground. (Items: " + spawnedCount.ToString() + ")");
			PlayLootFoundClientOrRPC(player);
			TryApplySearchHazard(player, lootCategory);
		}
	}

	protected void TryApplySearchHazard(PlayerBase player, string category)
	{
		float chance = TR_LootSettingsManager.GetHazardChanceForCategory(category);
		if (chance <= 0.0) return;

		float roll = Math.RandomFloat01();
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
					player.MessageStatus("Your hands get scraped but your gloves take the hit.");
				}
				return;
			}
			ApplyRandomBleedingWound(player);
			player.MessageStatus("Your ruined gloves fail to protect you. You cut yourself while rummaging!");
			return;
		}

		ApplyRandomBleedingWound(player);
		player.MessageStatus("You cut yourself while rummaging!");
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
		array<string> sels = new array<string>;
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
