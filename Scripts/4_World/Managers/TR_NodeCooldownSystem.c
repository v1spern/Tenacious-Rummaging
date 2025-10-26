// Cooldown System

class TR_PlayerCooldownRow
{
    string id;
    ref map<string, int> keys;

    void TR_PlayerCooldownRow()
    {
        keys = new map<string, int>();
    }
}

class TR_CooldownsSave
{
    int version;
    string world;
    string saved_at_utc;
    ref map<string, int> global;
    ref array<ref TR_PlayerCooldownRow> players;

    void TR_CooldownsSave()
    {
        version = 1;
        world = "";
        saved_at_utc = "";
        global = new map<string, int>();
        players = new array<ref TR_PlayerCooldownRow>();
    }
}

class TR_NodeCooldownSystem
{
    protected static ref TR_NodeCooldownSystem s_Instance;
    static TR_NodeCooldownSystem Get()
    {
        if (!s_Instance)
            s_Instance = new TR_NodeCooldownSystem();
        return s_Instance;
    }

    protected ref map<string, int> m_Global;
    protected ref map<string, ref map<string, int>> m_Players;

    void TR_NodeCooldownSystem()
    {
        m_Global = new map<string, int>();
        m_Players = new map<string, ref map<string, int>>();

        s_Instance = this;
    }

    void Load()
    {
        LoadFromDisk();
        TR_Debug.Log("[Cooldowns] Loaded from file - OK!");
    }

    void Save()
    {
        SaveToDisk(true);
        TR_Debug.Log("[Cooldowns] Saved to file - OK!");
    }

    bool IsOnCooldown(PlayerBase player, string key, float durationSec, bool globalScope)
    {
        if (key == "") return false;
        int now = NowMs();

        if (globalScope)
        {
            if (m_Global.Contains(key))
            {
                int expiryG = m_Global.Get(key);
                if (expiryG > now) return true;
                m_Global.Remove(key);
            }
            return false;
        }

        // Per-player
        PlayerIdentity id = null;
        if (player)
            id = player.GetIdentity();
        if (!id) return false;

        string pid = id.GetId();
        if (!m_Players.Contains(pid)) return false;

        ref map<string, int> per = m_Players.Get(pid);
        if (!per) return false;

        if (per.Contains(key))
        {
            int expiryP = per.Get(key);
            if (expiryP > now) return true;
            per.Remove(key);
        }
        return false;
    }

    void StartCooldown(PlayerBase player, string key, float durationSec, bool globalScope)
    {
        if (key == "") return;

        int now = NowMs();
        int expiry = now + (int)(durationSec * 1000.0);

        if (globalScope)
        {
            m_Global.Set(key, expiry);
        }
        else
        {
            // Per-player
            PlayerIdentity id = null;
            if (player)
                id = player.GetIdentity();
            if (!id) return;

            string pid = id.GetId();
            ref map<string, int> per;
            if (m_Players.Contains(pid))
            {
                per = m_Players.Get(pid);
            }
            else
            {
                per = new map<string, int>();
                m_Players.Set(pid, per);
            }
            per.Set(key, expiry);
        }

        SaveToDisk(false);
    }

    int Remaining(PlayerBase player, string key, float durationSec, bool globalScope)
    {
        if (key == "") return 0;
        int now = NowMs();

        if (globalScope)
        {
            if (m_Global.Contains(key))
            {
                int expiryG = m_Global.Get(key);
                int rem = expiryG - now;
                if (rem < 0) rem = 0;
                return rem / 1000;
            }
            return 0;
        }

        PlayerIdentity id = null;
        if (player)
            id = player.GetIdentity();
        if (!id) return 0;

        string pid = id.GetId();
        if (!m_Players.Contains(pid)) return 0;

        ref map<string, int> per = m_Players.Get(pid);
        if (!per) return 0;

        if (per.Contains(key))
        {
            int expiryP = per.Get(key);
            int rem2 = expiryP - now;
            if (rem2 < 0) rem2 = 0;
            return rem2 / 1000;
        }
        return 0;
    }

    void LoadFromDisk()
    {
        string path = GetFilePath();
        m_Global.Clear();
        m_Players.Clear();

        if (!FileExist(path))
        {
            TR_Debug.Log("[Cooldowns] No file on disk; starting empty.");
            return;
        }

        TR_CooldownsSave data = new TR_CooldownsSave();
        JsonFileLoader<TR_CooldownsSave>.JsonLoadFile(path, data);

        string world = GetGame().GetWorldName();
        if (data.version != 1)
        {
            TR_Debug.Log("[Cooldowns] File version mismatch; ignoring file.");
            return;
        }
        if (data.world != "" && data.world != world)
        {
            TR_Debug.Log("[Cooldowns] World mismatch; ignoring file. fileWorld=" + data.world + " curWorld=" + world);
            return;
        }

        int now = NowMs();

        if (data.global)
        {
            for (int gi = 0; gi < data.global.Count(); gi++)
            {
                string gkey = data.global.GetKey(gi);
                int remaining = data.global.GetElement(gi);
                if (remaining > 0)
                {
                    int expiry = now + remaining;
                    m_Global.Set(gkey, expiry);
                }
            }
        }

        if (data.players)
        {
            for (int pi = 0; pi < data.players.Count(); pi++)
            {
                TR_PlayerCooldownRow row = data.players.Get(pi);
                if (!row || row.id == "" || !row.keys) continue;

                ref map<string, int> per = new map<string, int>();
                bool hasAny = false;

                for (int ki = 0; ki < row.keys.Count(); ki++)
                {
                    string pkey = row.keys.GetKey(ki);
                    int prem = row.keys.GetElement(ki);
                    if (prem > 0)
                    {
                        int pexp = now + prem;
                        per.Set(pkey, pexp);
                        hasAny = true;
                    }
                }

                if (hasAny)
                    m_Players.Set(row.id, per);
            }
        }

        int gCount = m_Global.Count();
        int pPlayers = m_Players.Count();
        TR_Debug.Log("[Cooldowns] Counted from file: global='" + gCount.ToString() + "' playerBuckets='" + pPlayers.ToString() + "'");
    }

    void SaveToDisk(bool force)
    {
        string path = GetFilePath();
        TR_CooldownsSave data = new TR_CooldownsSave();
        data.version = 1;
        data.world = GetGame().GetWorldName();
        data.saved_at_utc = "";

        int now = NowMs();

        for (int gi = 0; gi < m_Global.Count(); gi++)
        {
            string gkey = m_Global.GetKey(gi);
            int gexp = m_Global.GetElement(gi);
            int grem = gexp - now;
            if (grem > 0)
                data.global.Set(gkey, grem);
        }

        for (int pi = 0; pi < m_Players.Count(); pi++)
        {
            string pid = m_Players.GetKey(pi);
            ref map<string, int> per = m_Players.GetElement(pi);
            if (!per || per.Count() == 0) continue;

            TR_PlayerCooldownRow row = new TR_PlayerCooldownRow();
            row.id = pid;

            for (int ki = 0; ki < per.Count(); ki++)
            {
                string pkey = per.GetKey(ki);
                int pexp = per.GetElement(ki);
                int prem = pexp - now;
                if (prem > 0)
                    row.keys.Set(pkey, prem);
            }

            if (row.keys.Count() > 0)
                data.players.Insert(row);
        }

        JsonFileLoader<TR_CooldownsSave>.JsonSaveFile(path, data);

        int gW = data.global.Count();
        int rows = data.players.Count();
        TR_Debug.Log("[Cooldowns] Counted from server memory: global='" + gW.ToString() + "', playerRows='" + rows.ToString() + "'");
    }

    protected string GetFilePath()
    {
        TR_Constants.EnsureProfile();
        return "$profile:TenaciousRummaging/Cooldowns.json";
    }

    protected int NowMs()
    {
        return GetGame().GetTime();
    }
}
