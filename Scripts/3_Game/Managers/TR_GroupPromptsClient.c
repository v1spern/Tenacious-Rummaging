// Group Prompts Client

class TR_GroupPromptsClient
{
    protected static ref map<string, string> s_MapLower;

    protected static ref map<string, string> s_Map;
    protected static string s_Default;

    static void Ensure()
    {
        if (!s_Map) s_Map = new map<string, string>();
        if (!s_MapLower) s_MapLower = new map<string, string>();
    }

    static void Clear()
    {
        Ensure();
        s_Map.Clear();
        s_MapLower.Clear();
        s_Default = "";
    }

    static void SetDefault(string t) { s_Default = t; }
    static string GetDefault() { return s_Default; }

    static void SetGroupPrompt(string name, string text)
    {
        Ensure();
        if (name == "") return;
        string nk = name; nk.Trim();
        string lk = nk; lk.ToLower();
        s_Map.Insert(nk, text);
        s_MapLower.Insert(lk, text);
    }

    static string GetGroupPrompt(string name)
    {
        Ensure();
        if (name == "") return "";
        string nk = name; nk.Trim();
        if (s_Map.Contains(nk)) return s_Map.Get(nk);
        string lk = nk; lk.ToLower();
        if (s_MapLower.Contains(lk)) return s_MapLower.Get(lk);
        return "";
    }
}
