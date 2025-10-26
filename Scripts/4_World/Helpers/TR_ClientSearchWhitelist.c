// Client whitelist 

class TR_ClientSearchWhitelist
{
    static bool AllowForPrompt(Object obj)
    {
        if (!obj) return false;
        return TR_SearchNodeUtils.CanSearchNode(obj);
    }

    static Object PreferRootTarget(Object obj, Object parentIfAny)
    {
        if (parentIfAny) return parentIfAny;
        return obj;
    }
}
