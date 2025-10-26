// Add Virtual Interior Search Node (AdminPebble)

class TR_ActionAddVirtualSearchNode : ActionInteractBase
{
    void TR_ActionAddVirtualSearchNode()
    {
        TR_Debug.Log("[Virtual Node Add] TR_ActionAddVirtualSearchNode LOADED (namespaced, parent-aware=True)");
        m_Text       = "Add virtual interior node";
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
    }

    override void CreateConditionComponents()
    {
        m_ConditionTarget = new CCTCursorParent;
        m_ConditionItem   = new CCINone;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!player) return false;

        ItemBase inHands; if (!Class.CastTo(inHands, player.GetItemInHands())) inHands = null;
        if (!inHands || !inHands.IsKindOf("AdminPebble")) return false;

        Building b = ResolveBuildingFromTargetOrNear(player, target, 6.0);
        return b != null;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        PlayerBase pb; if (!Class.CastTo(pb, action_data.m_Player)) return;
        if (!pb) return;

        Building b = ResolveBuildingFromTargetOrNear(pb, action_data.m_Target, 6.0);
        if (!b)
        {
            TR_Notify.Send(pb, "[Virtual Node Add] No valid building found nearby.");
            return;
        }

        string buildingKey = b.GetType();
        buildingKey.ToLower();

        string token = TR_Interior.ServerComputeFurnitureToken(pb, b);

        TR_SearchNodesDb.AddInteriorPieceNode(buildingKey, token, "general", "virtual");

        TR_Notify.Send(pb, "[Virtual Node Add] Added: " + buildingKey + " @ " + token + " [virtual]");
        TR_Debug.Log("[AddVirtualNode] Adding -> class=" + buildingKey + " @" + token + " label=virtual");
    }

    protected Building ResolveBuildingFromTargetOrNear(PlayerBase player, ActionTarget target, float radius)
    {
        Object hit = ResolveHit(target);
        Building b = FindBuildingAncestor(hit);
        if (b) return b;
        return FindNearestBuildingWithin(player, radius);
    }

    protected Object ResolveHit(ActionTarget t)
    {
        if (!t) return null;
        Object o = t.GetObject();
        if (o) return o;
        return t.GetParent();
    }

    protected Building FindBuildingAncestor(Object start)
    {
        if (!start) return null;
        Object cur = start;
        int hops = 0;
        while (cur && hops < 32)
        {
            Building b;
            if (Class.CastTo(b, cur)) return b;
            cur = cur.GetParent();
            hops++;
        }
        return null;
    }

    protected Building FindNearestBuildingWithin(PlayerBase player, float radius)
    {
        if (!player) return null;

        ref array<Object> objs = new array<Object>;
        ref array<CargoBase> dummy = new array<CargoBase>;
        GetGame().GetObjectsAtPosition3D(player.GetPosition(), radius, objs, dummy);

        int n = objs.Count();
        float bestD2 = 3.4e38;
        Building best = null;

        float px = player.GetPosition()[0];
        float py = player.GetPosition()[1];
        float pz = player.GetPosition()[2];

        for (int i = 0; i < n; i++)
        {
            Object o = objs.Get(i);
            if (!o) continue;

            Building b; if (!Class.CastTo(b, o)) continue;

            float ox = o.GetPosition()[0];
            float oy = o.GetPosition()[1];
            float oz = o.GetPosition()[2];

            float dx = ox - px;
            float dy = oy - py;
            float dz = oz - pz;
            float d2 = dx*dx + dy*dy + dz*dz;
            if (d2 < bestD2)
            {
                bestD2 = d2;
                best = b;
            }
        }
        return best;
    }
}
