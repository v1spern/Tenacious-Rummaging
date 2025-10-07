// Add Interior Node (cursor-targeted admin add; thin wrapper over TR_Interior)

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
		// Use the actual cursor hit object; some large statics don't expose a useful "parent"
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
		m_ConditionItem   = new CCINone;
	}

	override typename GetInputType()   { return DefaultActionInput; }
	override bool UseAcknowledgment()  { return false; }
	override bool HasTarget()          { return true; }
	override string GetText()          { return m_Text; }

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player || !target) return false;

		// Require admin stone in hands
		ItemBase inHands = ItemBase.Cast(player.GetItemInHands());
		if (!inHands) return false;
		if (!inHands.IsKindOf("TR_AdminStone") && !inHands.IsKindOf("AdminStone")) return false;

		// Delegate gating to TR_Interior (no component/selection dependency)
		return TR_Interior.CanAdminAdd(player, target);
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

		// Admin add; default group remains "general"
		string msg = TR_Interior.AdminAddFromCursor(pb, action_data.m_Target, "general");
		if (msg == "")
			msg = "[AddInterior] Failed to add interior token.";
		pb.MessageStatus(msg);
	}
}
