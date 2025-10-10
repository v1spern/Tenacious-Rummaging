// Add Virtual Interior Search Node (AdminPebble)
// Bound to F via AddAction(ActionAddVirtualSearchNode, InputActionMap)
// Targeting: prefer cursor hit → building ancestor; else nearest building within 6 m
// Token: player world pos transformed into building local grid
class ActionAddVirtualSearchNode : ActionInteractBase
{
    void ActionAddVirtualSearchNode()
    {
        m_Text       = "Add virtual interior node";
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
    }

    override void CreateConditionComponents()
    {
        // Get a real cursor hit when possible (proxies or mesh); if no hit, we still allow via proximity fallback.
        m_ConditionTarget = new CCTCursor;
        m_ConditionItem   = new CCINone;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!player) return false;

        ItemBase inHands = ItemBase.Cast(player.GetItemInHands());
        if (!inHands || !inHands.IsKindOf("AdminPebble")) return false;

        Building b = ResolveBuildingFromTargetOrNear(player, target, 6.0);
        return b != null;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        PlayerBase pb = PlayerBase.Cast(action_data.m_Player);
        if (!pb) return;

        Building b = ResolveBuildingFromTargetOrNear(pb, action_data.m_Target, 6.0);
        if (!b)
        {
            pb.MessageStatus("[AddVirtual] No valid building found nearby.");
            return;
        }

        // INLINE (v1.0.3 compatible): lower-cased classname
        string buildingKey = b.GetType();
        buildingKey.ToLower();

        // Quantized token from player's position in building local space
        string token = TR_Interior.ServerComputeFurnitureToken(pb, b);

        // Persist to SearchableNodes.json
        TR_SearchNodesDb.AddInteriorPieceNode(buildingKey, token, "general", "virtual");

        pb.MessageStatus("[AddVirtual] Added: " + buildingKey + " @ " + token + " [virtual]");
        TR_Debug.Log("ADD-VIRTUAL -> class=" + buildingKey + " @" + token + " label=virtual");
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
            Building b = Building.Cast(cur);
            if (b) return b;
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

            Building b = Building.Cast(o);
            if (!b)
            {
                House h = House.Cast(o);
                if (h) b = Building.Cast(o);
            }
            if (!b) continue;

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
