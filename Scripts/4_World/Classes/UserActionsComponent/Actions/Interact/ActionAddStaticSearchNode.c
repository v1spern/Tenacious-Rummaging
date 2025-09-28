// Add Static Search Node Action

class ActionAddStaticSearchNode : ActionInteractBase
{
    void ActionAddStaticSearchNode()
    {
        m_Text = "Make this specific node rummageable (Static node)";
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
    }

    override void CreateConditionComponents()
    {
        m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
        m_ConditionItem   = new CCINone;
    }

    override typename GetInputType()
    {
        return DefaultActionInput;
    }

    override bool UseAcknowledgment()
    {
        return false;
    }

    // Ensure action works on world objects, not inventory junctures
    override bool HasTarget()
    {
        return true;
    }

    override string GetText()
    {
        return m_Text;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!player || !target) return false;

        ItemBase inHands = ItemBase.Cast(player.GetItemInHands());
        if (!inHands) return false;

        // Accept both possible admin stone class names
        bool hasAdminStone = inHands.IsKindOf("TR_AdminStone") || inHands.IsKindOf("AdminStone");
        if (!hasAdminStone) return false;

        Object obj = target.GetObject();
        if (!obj || target.IsProxy()) return false;

        // Hide when aiming at any house/building (interior or wall/exterior)
        if (TR_Interior.IsHouseTarget(obj)) return false;

        return true;
    }

    // IMPORTANT: override juncture and avoid touching target inventory (static props have none)
    override bool AddActionJuncture(ActionData action_data)
    {
        PlayerBase player = PlayerBase.Cast(action_data.m_Player);
        if (!player) return false;

        // Admin stone in hands check
        ItemBase hands = player.GetItemInHands();
        if (!hands) return false;
        if (!hands.IsKindOf("TR_AdminStone") && !hands.IsKindOf("AdminStone")) return false;

        TR_Debug.Log("[Admin] AddStaticNode Juncture: bypassed base to avoid target-inventory on static props");
        return true;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        PlayerBase player = PlayerBase.Cast(action_data.m_Player);
        if (!player || !action_data || !action_data.m_Target) return;

        Object obj = action_data.m_Target.GetObject();
        if (!obj)
        {
            player.MessageStatus("No target found.");
            TR_Debug.Warn("ADD-STATIC failed: ActionTarget.GetObject() returned null");
            return;
        }

        // Safety: ensure admin stone server-side too!!
        ItemBase hands = player.GetItemInHands();
        bool hasAdminStone = false;
        if (hands)
        {
            if (hands.IsKindOf("TR_AdminStone")) hasAdminStone = true;
            else if (hands.IsKindOf("AdminStone")) hasAdminStone = true;
        }
        if (!hasAdminStone)
        {
            player.MessageStatus("You must hold the Admin Stone to add nodes.");
            TR_Debug.Warn("ADD-STATIC failed: missing Admin Stone in hands");
            return;
        }

        // Resolve class and model
        string className = obj.GetType();
        if (className == "")
        {
            string dbgName = obj.GetDebugNameNative();
            TStringArray parts = {};
            dbgName.Split(":", parts);
            if (parts.Count() > 0) className = parts[0].Trim();
        }

        string modelName = TR_Util.GetObjectModel(obj);
        vector pos = obj.GetPosition();

        // Client toast + abort if unsupported (no usable class or model)
        bool unsupported = false;
        if (className == "") unsupported = true;
        if (modelName == "") unsupported = true;
        if (unsupported)
        {
            player.MessageStatus("Target not supported for static add.");
            TR_Debug.Warn("ADD-STATIC aborted: unsupported target (class or model missing). Obj=" + obj.ToString());
            return;
        }

        // Add node to DB
        TR_SearchNodesDb.AddStaticNode(className, modelName, pos);
        TR_SearchNodesDb.Save();

        string defGroup = TR_LootSettingsManager.GetDefaultGroup();

        string msg = "Added static node: " + className;
        if (modelName != "") msg += " (model: " + modelName + ")";
        msg += " @" + pos.ToString();
        if (defGroup != "") msg += " [" + defGroup + "]";
        player.MessageStatus(msg);

        TR_Debug.Log("ADD-STATIC -> class=" + className + " model=" + modelName + " @ " + pos.ToString() + " group=" + defGroup);
    }
}
