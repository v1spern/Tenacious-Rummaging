// Action: Add Interior SearchNode

class ActionAddInteriorPieceSearchNode : ActionInteractBase
{
	void ActionAddInteriorPieceSearchNode()
	{
		m_Text       = "Make this furniture rummageable (Interior)";
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursorParent(UAMaxDistances.DEFAULT);
		m_ConditionItem   = new CCINone;
	}

	override typename GetInputType()   { return DefaultActionInput; }
	override bool UseAcknowledgment()  { return false; }
	override bool HasTarget()          { return true; }
	override string GetText()          { return m_Text; }

	protected static House ResolveHouse(ActionTarget t)
	{
		if (!t) return null;
		Object p = t.GetParent();
		if (p)
		{
			House hp = House.Cast(p);
			if (hp) return hp;
		}
		Object o = t.GetObject();
		if (o)
		{
			House ho = House.Cast(o);
			if (ho) return ho;
		}
		return null;
	}

	protected static string ReadSelectionName(ActionTarget t, House h, int compIdx)
	{
		string s = "";
		if (compIdx >= 0)
		{
			Object o = t.GetObject();
			if (o)
			{
				string so;
				o.GetActionComponentName(compIdx, so);
				if (so != "") s = so;
			}
			if (s == "" && h)
			{
				string sh;
				h.GetActionComponentName(compIdx, sh);
				if (sh != "") s = sh;
			}
		}
		s.ToLower();
		s.Trim();
		return s;
	}

	protected static bool IsWallishSelection(string s)
	{
		if (s.Contains("wall"))     return true;
		if (s.Contains("ceiling"))  return true;
		if (s.Contains("floor"))    return true;
		if (s.Contains("roof"))     return true;
		if (s.Contains("stairs"))   return true;
		if (s.Contains("pillar"))   return true;
		if (s.Contains("window"))   return true;
		if (s.Contains("frame"))    return true;
		if (s.Contains("door"))     return true;
		if (s.Contains("arch"))     return true;
		if (s.Contains("molding"))  return true;
		if (s.Contains("trim"))     return true;
		if (s.Contains("beam"))     return true;
		if (s.Contains("ledge"))    return true;
		if (s.Contains("sill"))     return true;
		return false;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player || !target) return false;
		ItemBase inHands = ItemBase.Cast(player.GetItemInHands());
		if (!inHands) return false;
		if (!inHands.IsKindOf("AdminStone")) return false;
		House h = ResolveHouse(target);
		if (!h) return false;
		int compIdx = target.GetComponentIndex();
		if (compIdx < 0) return false;
		string selName = ReadSelectionName(target, h, compIdx);
		if (selName != "" && IsWallishSelection(selName)) return false;
		return true;
	}

	override bool AddActionJuncture(ActionData action_data)
	{
		PlayerBase pb = PlayerBase.Cast(action_data.m_Player);
		if (!pb) return false;
		ItemBase hands = ItemBase.Cast(pb.GetItemInHands());
		return hands && (hands.IsKindOf("AdminStone"));
	}

	override void OnExecuteServer(ActionData action_data)
	{
		PlayerBase pb = PlayerBase.Cast(action_data.m_Player);
		if (!pb) return;
		House h = ResolveHouse(action_data.m_Target);
		if (!h)
		{
			TR_Notify.Send(pb, "[AddInterior] Not a house.");
			return;
		}
		string hLow = h.GetType();
		string defGroup = TR_LootSettingsManager.GetDefaultGroup();
		hLow.ToLower();
		string token = TR_Interior.ServerComputeFurnitureToken(pb, h);
		int compIdx = action_data.m_Target.GetComponentIndex();
		string label = ReadSelectionName(action_data.m_Target, h, compIdx);
		if (label == "") label = "interior";
		TR_SearchNodesDb.AddInteriorPieceNode(hLow, token, defGroup, label);
		string msg = "Added interior node '" + hLow + "' @ '" + token + "' ['" + label + "'] (group: '" + defGroup + "')";
		TR_Notify.Send(pb, msg);
		TR_Debug.Log("[AddNode] Type: Interior - '" + hLow + "' @ '" + token + "', label '" + label + ", group '" + defGroup + "'");
	}
}
