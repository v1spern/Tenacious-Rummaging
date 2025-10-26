// Search Node Utility

class TR_SearchNodeUtils
{
    protected static int s_LastPrintMs = 0;
    protected static string s_LastKey = "";
    protected static bool s_LastAllowed = false;

    static bool CanSearchNode(Object targetObj)
    {
        if (!targetObj) return false;

        TR_SearchNode node = TR_SearchNodesDb.MatchEx(targetObj, false);
        bool allowed = (node != null);

        string rawModel = TR_Util.GetObjectModel(targetObj);
        string normKey  = TR_SearchNodesDb.NormalizeModelKey(rawModel);
        int now = GetGame().GetTime();

        bool keyChanged   = (normKey != s_LastKey);
        bool stateChanged = (allowed != s_LastAllowed);
        bool timeOk       = (now - s_LastPrintMs) >= 3250;

        if (keyChanged || stateChanged || timeOk)
        {
            if (allowed)
                TR_Debug.Log("[Util] CanSearchNode = TRUE, key = '" + normKey + "'");
            s_LastKey = normKey;
            s_LastAllowed = allowed;
            s_LastPrintMs = now;
        }

        return allowed;
    }

    // Client-side interior prompt gate wrapper
    static bool CanSearchInterior(PlayerBase player, Object obj)
    {
        return TR_Interior.ClientCanPrompt(player, obj);
    }
}
