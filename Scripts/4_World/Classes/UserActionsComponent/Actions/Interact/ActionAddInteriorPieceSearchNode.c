// Add Interior Node

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
		// Parent-first so we operate on the building/house parent
		m_ConditionTarget = new CCTCursorParent(UAMaxDistances.DEFAULT);
		m_ConditionItem   = new CCINone;
	}

	override typename GetInputType()   { return DefaultActionInput; }
	override bool UseAcknowledgment()  { return false; }
	override bool HasTarget()          { return true; }
	override string GetText()          { return m_Text; }

	// Resolve the house we act on (parent preferred, then object) - satan i helvetes verk omg
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

	// Read a selection/component name. Try hit object first, then the house parent. Then go to hell. 
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

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player || !target) return false;

		// Require admin stone in hands
		ItemBase inHands = ItemBase.Cast(player.GetItemInHands());
		if (!inHands) return false;
		if (!inHands.IsKindOf("TR_AdminStone") && !inHands.IsKindOf("AdminStone")) return false;

		// Must resolve to a House
		House h = ResolveHouse(target);
		if (!h) return false;

		// Need a valid component index from the aim
		int compIdx = target.GetComponentIndex();
		if (compIdx < 0) return false;

		// Fetch a usable selection label
		string selName = ReadSelectionName(target, h, compIdx);

		// ALLOW EVERYTHING EXCEPT WALLS/STRUCTURAL
		if (selName != "" && IsWallishSelection(selName)) return false;

		// Otherwise allow
		return true;
	}

	// Structural deny-list (conservative) --- FOR TESTING, KEEP IF IT WORKS DONT FCKING TOUCH
	protected static bool IsWallishSelection(string s)
	{
		if (s.Contains("wall"))     return true;  // are these even in the game
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

	override bool AddActionJuncture(ActionData action_data)
	{
		PlayerBase pb = PlayerBase.Cast(action_data.m_Player);
		if (!pb) return false;
		ItemBase hands = ItemBase.Cast(pb.GetItemInHands());
		return hands && (hands.IsKindOf("TR_AdminStone") || hands.IsKindOf("AdminStone"));
	}

	override void OnExecuteServer(ActionData action_data)
	{
		PlayerBase pb = PlayerBase.Cast(action_data.m_Player);
		if (!pb) return;

		// Resolve house (parent first, then object)
		House h = ResolveHouse(action_data.m_Target);
		if (!h)
		{
			pb.MessageStatus("[AddInterior] Not a house.");
			return;
		}

		string hLow = h.GetType();
		hLow.ToLower();

		// Compute per-piece token in building-local space (server-side)
		string token = TR_Interior.ServerComputeFurnitureToken(pb, h);

		// Try to capture a human-readable selection/component name for the Comment field
		int compIdx = action_data.m_Target.GetComponentIndex();
		string label = ReadSelectionName(action_data.m_Target, h, compIdx);
		if (label == "") label = "interior";

		// Persist: ModelName="__interior_tok__", Position="<xi, yi, zi>", LootGroup="general", Comment=label
		TR_SearchNodesDb.AddInteriorPieceNode(hLow, token, "general", label);

		pb.MessageStatus("[AddInterior] Added: " + hLow + " @ " + token + " [" + label + "] (group: general)");
	}
}
