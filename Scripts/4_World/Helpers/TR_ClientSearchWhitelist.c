// Client whitelist 

class TR_ClientSearchWhitelist
{
    // Show the prompt only if the object is present in the client-synced SearchableNodes.json.
    static bool AllowForPrompt(Object obj)
    {
        if (!obj) return false;
        return TR_SearchNodeUtils.CanSearchNode(obj);
    }

    // Prefer the root parent if the cursor hit a proxy (keeps behavior stable with proxied statics)
    static Object PreferRootTarget(Object obj, Object parentIfAny)
    {
        if (parentIfAny) return parentIfAny;
        return obj;
    }
}
