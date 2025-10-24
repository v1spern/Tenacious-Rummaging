// Add Class-Wide Searchable Node

class ActionAddClassSearchNode : ActionInteractBase
{
    void ActionAddClassSearchNode()
    {
        m_Text = "Make this class rummageable (Class-wide)";
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

        // Accept both old and new admin stone class names
        bool hasAdminStone = inHands.IsKindOf("TR_AdminStone") || inHands.IsKindOf("AdminStone");
        if (!hasAdminStone) return false;

        Object obj = target.GetObject();
        if (!obj || target.IsProxy()) return false;

        // Hide when looking at houses
        if (TR_Interior.IsHouseTarget(obj)) return false;

        return true;
    }

    // override juncture and avoid touching target inventory
    override bool AddActionJuncture(ActionData action_data)
    {
        PlayerBase player = PlayerBase.Cast(action_data.m_Player);
        if (!player) return false;

        // Admin stone checkz
        ItemBase hands = player.GetItemInHands();
        if (!hands) return false;
        if (!hands.IsKindOf("TR_AdminStone") && !hands.IsKindOf("AdminStone")) return false;

        TR_Debug.Log("[Admin] AddClassWide Juncture: bypassed base to avoid target-inventory on static props");
        return true;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        PlayerBase player = PlayerBase.Cast(action_data.m_Player);
        if (!player || !action_data || !action_data.m_Target) return;

        Object obj = action_data.m_Target.GetObject();
        if (!obj)
        {
            TR_Notify.Send(player, "No target found.");
            TR_Debug.Warn("ADD-CLASSWIDE failed: ActionTarget.GetObject() returned null");
            return;
        }

        // Safety: ensure admin stone server-side too!
        ItemBase hands = player.GetItemInHands();
        bool hasAdminStone = false;
        if (hands)
        {
            if (hands.IsKindOf("TR_AdminStone")) hasAdminStone = true;
            else if (hands.IsKindOf("AdminStone")) hasAdminStone = true;
        }
        if (!hasAdminStone)
        {
            TR_Notify.Send(player, "You must hold the Admin Stone to add nodes.");
            TR_Debug.Warn("ADD-CLASSWIDE failed: missing Admin Stone in hands");
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

        // Abort if unsupported (no usable class or model)
        bool unsupported = false;
        if (className == "") unsupported = true;
        if (modelName == "") unsupported = true;
        if (unsupported)
        {
            TR_Notify.Send(player, "Target not supported for class-wide add.");
            TR_Debug.Warn("ADD-CLASSWIDE aborted: unsupported target (class or model missing). Obj=" + obj.ToString());
            return;
        }

        // Duplicate guard: class-wide uniqueness is (class + model)
        if (TR_SearchNodesDb.ExistsClassWideNode(className, modelName))
        {
            TR_Notify.Send(player, "Duplicate: class-wide node already exists for " + className + ".");
            TR_Debug.Log("ADD-CLASSWIDE DUPLICATE -> class=" + className + " model=" + modelName);
            return;
        }

        // Add & persist
        TR_SearchNodesDb.AddClassWideNode(className, modelName);
        TR_SearchNodesDb.Save();

        string defGroup = TR_LootSettingsManager.GetDefaultGroup();

        string msg = "Added class-wide: " + className;
        if (modelName != "") msg += " (model: " + modelName + ")";
        if (defGroup != "") msg += " [" + defGroup + "]";
        TR_Notify.Send(player, msg);

        TR_Debug.Log("ADD-CLASSWIDE OK -> class=" + className + " model=" + modelName + " group=" + defGroup);
    }
}
