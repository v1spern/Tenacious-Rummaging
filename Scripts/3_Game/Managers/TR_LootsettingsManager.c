// Loot Settings Manager

static const string TR_FLAVOR_UNSET = "#__TR_UNSET__#";

class TR_LSM_Notifications
{
    bool   Enabled = true;
    float  DurationSeconds = 5.0;
    int    EnableToastIcon = 1;
}

class TR_LSM_ZombieSpawnEvent
{
    float Chance = 0.0;
    int   CountMin = 1;
    int   CountMax = 1;
    float SpawnRadius = 8.0;
    ref array<string> ZombieTypes;
    float AlertNoiseStrength = 50.0;
    float AlertNoiseRange = 30.0;
    string FlavorText;

    void TR_LSM_ZombieSpawnEvent()
    {
        ZombieTypes = new array<string>();
        FlavorText = TR_FLAVOR_UNSET;
    }
}

class TR_LSM_SmokeEvent
{
    float Chance = 0.0;
    string Classname = "M18SmokeGrenade_Red";
    float FuseSeconds = 1.0;
    string FlavorText;

    void TR_LSM_SmokeEvent()
    {
        FlavorText = TR_FLAVOR_UNSET;
    }
}

class TR_LSM_GasEvent
{
    float Chance = 0.0;
    string Classname = "Grenade_ChemGas";
    float FuseSeconds = 1.0;
    string FlavorText;

    void TR_LSM_GasEvent()
    {
        FlavorText = TR_FLAVOR_UNSET;
    }
}

class TR_LSM_KnockOutEvent
{
    float Chance = 0.0;
    float DurationSeconds = 10.0;
    float HealthDamage = 25.0;
    string FlavorText;

    void TR_LSM_KnockOutEvent()
    {
        FlavorText = TR_FLAVOR_UNSET;
    }
}

class TR_LSM_ShockEvent
{
    float Chance = 0.0;
    float ShockAmount = 80.0;
    float HealthDamage = 25.0;
    string FlavorText;

    void TR_LSM_ShockEvent()
    {
    FlavorText = TR_FLAVOR_UNSET;
    }
}

class TR_LSM_SirenAlarmEvent
{
    float Chance = 0.0;
    string FlavorText;

    void TR_LSM_SirenAlarmEvent()
    {
        FlavorText = TR_FLAVOR_UNSET;
    }
}

class TR_LSM_Events
{
    ref TR_LSM_ZombieSpawnEvent ZombieSpawn;
    ref TR_LSM_SmokeEvent       Smoke;
    ref TR_LSM_GasEvent         Gas;
    ref TR_LSM_KnockOutEvent    KnockOut;
    ref TR_LSM_ShockEvent       Shock;
    ref TR_LSM_SirenAlarmEvent  SirenAlarm;

    void TR_LSM_Events()
    {
        ZombieSpawn = new TR_LSM_ZombieSpawnEvent();
        Smoke       = new TR_LSM_SmokeEvent();
        Gas         = new TR_LSM_GasEvent();
        KnockOut    = new TR_LSM_KnockOutEvent();
        Shock       = new TR_LSM_ShockEvent();
        SirenAlarm  = new TR_LSM_SirenAlarmEvent();
    }
}

class TR_CategorySettings
{
    float CooldownTime = 1800;
    float HazardChance = 0.0;

    int GloveDamageMin = 5;
    int GloveDamageMax = 15;

    ref TR_LSM_Events Events;
}

class TR_LootSettingsData
{
    ref map<string, ref TR_CategorySettings> Categories;

    float  DefaultCooldownTime = 1800;
    string CooldownScope = "Global";
    string DefaultGroup = "";
    string DefaultPromptText = "";
    string NothingFoundText = "";

    bool DebugMode = false;
    bool EnableCsvLogging = false;

    ref TR_LSM_Notifications Notifications;

    void TR_LootSettingsData()
    {
        Categories    = new map<string, ref TR_CategorySettings>();
        Notifications = new TR_LSM_Notifications();
    }
}

class TR_LootSettingsManager
{
    static private ref TR_LootSettingsData s_Data;

    static void EnsureLoaded()
    {
        if (!s_Data)
            Load();
    }

    static float NotifDurationSeconds()
    {
        EnsureLoaded();
        if (!s_Data || !s_Data.Notifications) return 5.0;
        float v = s_Data.Notifications.DurationSeconds;
        if (v < 0.5) v = 0.5;
        if (v > 15.0) v = 15.0;
        return v;
    }

    static bool EnableToastIcon()
    {
        EnsureLoaded();
        if (!s_Data || !s_Data.Notifications) return true;
        return s_Data.Notifications.EnableToastIcon != 0;
    }

    static bool NotifEnabled()
    {
        EnsureLoaded();
        if (!s_Data || !s_Data.Notifications) return true;
        return s_Data.Notifications.Enabled;
    }

    static void Load()
    {
        s_Data = new TR_LootSettingsData();
        string path = TR_Constants.Path("LootSettings.json");

        bool mutated = false;

        if (FileExist(path))
        {
            JsonFileLoader<TR_LootSettingsData>.JsonLoadFile(path, s_Data);
            if (s_Data.NothingFoundText == "") { s_Data.NothingFoundText = "You found nothing."; mutated = true; }

            if (!s_Data.Notifications) { s_Data.Notifications = new TR_LSM_Notifications(); mutated = true; }
            else
            {
                float nd = s_Data.Notifications.DurationSeconds;
                if (nd < 0.5) { s_Data.Notifications.DurationSeconds = 0.5; mutated = true; }
                if (nd > 15.0) { s_Data.Notifications.DurationSeconds = 15.0; mutated = true; }
            }

            if (s_Data && s_Data.Categories)
            {
                foreach (string k, TR_CategorySettings cs : s_Data.Categories)
                {
                    if (!cs.Events) { cs.Events = CreateDefaultEvents(); mutated = true; }

                    if (cs.CooldownTime <= 0) { cs.CooldownTime = s_Data.DefaultCooldownTime; mutated = true; }
                    if (cs.GloveDamageMin < 0) { cs.GloveDamageMin = 0; mutated = true; }
                    if (cs.GloveDamageMax < 0) { cs.GloveDamageMax = 0; mutated = true; }
                    if (cs.GloveDamageMax < cs.GloveDamageMin) { cs.GloveDamageMax = cs.GloveDamageMin; mutated = true; }
                    if (cs.HazardChance < 0.0) { cs.HazardChance = 0.0; mutated = true; }
                    if (cs.HazardChance > 1.0) { cs.HazardChance = 1.0; mutated = true; }

                    mutated = EnsureCategoryBackfilled(cs) || mutated;
                }
            }

            if (mutated)
            {
                JsonFileLoader<TR_LootSettingsData>.JsonSaveFile(path, s_Data);
                TR_Debug.Warn("[LootSettings] Backfill detected; wrote updated LootSettings.json");
            }
        }
        else
        {
            SaveManual(s_Data);
        }

        ValidateGroups();
    }

    static void SaveManual(TR_LootSettingsData data)
    {
        string path = TR_Constants.Path("LootSettings.json");

        if (!data.Categories || data.Categories.Count() == 0)
        {
            data.Categories = new map<string, ref TR_CategorySettings>();

            TR_CategorySettings gen = new TR_CategorySettings();
            gen.CooldownTime = 1800;
            gen.GloveDamageMin = 5;
            gen.GloveDamageMax = 15;
            gen.Events = CreateDefaultEvents();
            data.Categories.Set("general", gen);

            TR_CategorySettings misc = new TR_CategorySettings();
            misc.CooldownTime = 1800;
            misc.GloveDamageMin = 5;
            misc.GloveDamageMax = 15;
            misc.Events = CreateDefaultEvents();
            data.Categories.Set("misc", misc);

            data.DefaultCooldownTime = 1800;
            data.DefaultPromptText = "Rummage here for loot";
            data.NothingFoundText = "You found nothing.";
            data.CooldownScope = "Global";
            data.DefaultGroup  = "general";
            data.DebugMode = false;
            data.EnableCsvLogging = false;
        }

        if (!data.Notifications)
        {
            data.Notifications = new TR_LSM_Notifications();
        }
        data.Notifications.Enabled = true;
        data.Notifications.EnableToastIcon = 1;
        if (data.Notifications.DurationSeconds < 0.5) data.Notifications.DurationSeconds = 5.0;

        foreach (string ck, TR_CategorySettings cs2 : data.Categories)
        {
            EnsureCategoryBackfilled(cs2);
        }

        if (data.NothingFoundText == "") data.NothingFoundText = "You found nothing.";

        JsonFileLoader<TR_LootSettingsData>.JsonSaveFile(path, data);
    }

    static TR_LSM_Events CreateDefaultEvents()
    {
        TR_LSM_Events ev = new TR_LSM_Events();

        ev.ZombieSpawn.Chance = 0.0;
        ev.ZombieSpawn.CountMin = 1;
        ev.ZombieSpawn.CountMax = 2;
        ev.ZombieSpawn.SpawnRadius = 12.0;
        ev.ZombieSpawn.AlertNoiseStrength = 50.0;
        ev.ZombieSpawn.AlertNoiseRange = 30.0;
        ev.ZombieSpawn.FlavorText = "Your rummaging makes a lot of noise and draws immediate attention from nearby infected!";

        ev.Smoke.Chance = 0.0;
        ev.Smoke.Classname = "M18SmokeGrenade_Red";
        ev.Smoke.FuseSeconds = 1.0;
        ev.Smoke.FlavorText = "You jostle the stash and a smoke grenade hisses, flooding the area!";

        ev.Gas.Chance = 0.0;
        ev.Gas.Classname = "Grenade_ChemGas";
        ev.Gas.FuseSeconds = 1.0;
        ev.Gas.FlavorText = "You disturb the cache and a toxic canister pops, releasing choking gas!";

        ev.KnockOut.Chance = 0.0;
        ev.KnockOut.DurationSeconds = 12.0;
        ev.KnockOut.HealthDamage = 25.0;
        ev.KnockOut.FlavorText = "A heavy hit staggers you and you collapse unconscious..";

        ev.Shock.Chance = 0.0;
        ev.Shock.ShockAmount = 50.0;
        ev.Shock.HealthDamage = 25.0;
        ev.Shock.FlavorText = "A surge of electricity tears through you. Your muscles seize!";

        ev.SirenAlarm.Chance = 0.0;
        ev.SirenAlarm.FlavorText = "An air-raid siren blares across the area!";

        return ev;
    }

    static bool EnsureCategoryBackfilled(TR_CategorySettings cs)
    {
        bool mutated = false;

        if (!cs.Events)
        {
            cs.Events = CreateDefaultEvents();
            mutated = true;
        }
        else
        {
            mutated = BackfillEvents(cs.Events) || mutated;
        }

        return mutated;
    }

    static bool BackfillEvents(TR_LSM_Events ev)
    {
        if (!ev) return false;

        bool mutated = false;

        bool createdZ = false;
        if (!ev.ZombieSpawn) { ev.ZombieSpawn = new TR_LSM_ZombieSpawnEvent(); createdZ = true; mutated = true; }
        if (createdZ)
        {
            ev.ZombieSpawn.CountMin = 1;
            ev.ZombieSpawn.CountMax = 2;
            ev.ZombieSpawn.SpawnRadius = 12.0;
            ev.ZombieSpawn.AlertNoiseStrength = 50.0;
            ev.ZombieSpawn.AlertNoiseRange = 30.0;
            ev.ZombieSpawn.FlavorText = "Your rummaging makes a lot of noise and draws immediate attention from nearby infected!";
        }
        else
        {
            if (ev.ZombieSpawn.FlavorText == TR_FLAVOR_UNSET || ev.ZombieSpawn.FlavorText == "")
            {
                ev.ZombieSpawn.FlavorText = "Your rummaging makes a lot of noise and draws immediate attention from nearby infected!";
                mutated = true;
            }
        }

        bool createdS = false;
        if (!ev.Smoke) { ev.Smoke = new TR_LSM_SmokeEvent(); createdS = true; mutated = true; }
        if (createdS)
        {
            ev.Smoke.Classname = "M18SmokeGrenade_Red";
            ev.Smoke.FuseSeconds = 1.0;
            ev.Smoke.FlavorText = "You jostle the stash and a smoke grenade hisses, flooding the area!";
        }
        else
        {
            if (ev.Smoke.FlavorText == TR_FLAVOR_UNSET || ev.Smoke.FlavorText == "")
            {
                ev.Smoke.FlavorText = "You jostle the stash and a smoke grenade hisses, flooding the area!";
                mutated = true;
            }
        }

        bool createdG = false;
        if (!ev.Gas) { ev.Gas = new TR_LSM_GasEvent(); createdG = true; mutated = true; }
        if (createdG)
        {
            ev.Gas.Classname = "Grenade_ChemGas";
            ev.Gas.FuseSeconds = 1.0;
            ev.Gas.FlavorText = "You disturb the cache and a toxic canister pops, releasing choking gas!";
        }
        else
        {
            if (ev.Gas.FlavorText == TR_FLAVOR_UNSET || ev.Gas.FlavorText == "")
            {
                ev.Gas.FlavorText = "You disturb the cache and a toxic canister pops, releasing choking gas!";
                mutated = true;
            }
        }

        bool createdKO = false;
        if (!ev.KnockOut) { ev.KnockOut = new TR_LSM_KnockOutEvent(); createdKO = true; mutated = true; }
        if (createdKO)
        {
            ev.KnockOut.DurationSeconds = 12.0;
            ev.KnockOut.HealthDamage = 25.0;
            ev.KnockOut.FlavorText = "A heavy hit staggers you and you collapse unconscious..";
        }
        else
        {
            if (ev.KnockOut.FlavorText == TR_FLAVOR_UNSET || ev.KnockOut.FlavorText == "")
            {
                ev.KnockOut.FlavorText = "A heavy hit staggers you and you collapse unconscious..";
                mutated = true;
            }
        }

        bool createdSh = false;
        if (!ev.Shock) { ev.Shock = new TR_LSM_ShockEvent(); createdSh = true; mutated = true; }
        if (createdSh)
        {
            ev.Shock.ShockAmount = 50.0;
            ev.Shock.HealthDamage = 25.0;
            ev.Shock.FlavorText = "A surge of electricity tears through you. Your muscles seize!";
        }
        else
        {
            if (ev.Shock.FlavorText == TR_FLAVOR_UNSET || ev.Shock.FlavorText == "")
            {
                ev.Shock.FlavorText = "A surge of electricity tears through you. Your muscles seize!";
                mutated = true;
            }
        }

        bool createdSi = false;
        if (!ev.SirenAlarm) { ev.SirenAlarm = new TR_LSM_SirenAlarmEvent(); createdSi = true; mutated = true; }
        if (createdSi)
        {
            ev.SirenAlarm.FlavorText = "An air-raid siren blares across the area!";
        }
        else
        {
            if (ev.SirenAlarm.FlavorText == TR_FLAVOR_UNSET || ev.SirenAlarm.FlavorText == "")
            {
                ev.SirenAlarm.FlavorText = "An air-raid siren blares across the area!";
                mutated = true;
            }
        }

        return mutated;
    }

    static void ValidateGroups()
    {
        if (!s_Data) return;

        array<string> groups = TR_LootGroups.GetAllGroupNames();
        foreach (string g : groups)
        {
            if (!s_Data.Categories || !s_Data.Categories.Contains(g))
            {
                TR_Debug.Warn("[LootSettings] LootSettings.json missing settings for group '" + g + "'. Using defaults.");
            }
        }

        if (s_Data.Categories)
        {
            foreach (string key, TR_CategorySettings settings : s_Data.Categories)
            {
                if (!groups.Find(key))
                {
                    TR_Debug.Warn("[LootSettings] LootSettings.json defines category '" + key + "' which has no matching loot group in LootGroups.json.");
                }
            }
        }
    }

    static float GetCooldownForCategory(string groupName)
    {
        if (!s_Data) return 60;
        if (s_Data.Categories && s_Data.Categories.Contains(groupName))
        {
            return s_Data.Categories.Get(groupName).CooldownTime;
        }
        return s_Data.DefaultCooldownTime;
    }

    static bool IsCooldownGlobal()
    {
        if (!s_Data) return true;
        return s_Data.CooldownScope == "Global";
    }

    static string GetDefaultGroup()
    {
        if (!s_Data) return "";
        return s_Data.DefaultGroup;
    }

    static bool IsDebugEnabled()
    {
        if (!s_Data) return false;
        return s_Data.DebugMode;
    }

    static bool IsCsvLoggingEnabled()
    {
        if (!s_Data) return false;
        return s_Data.EnableCsvLogging;
    }

    static float GetHazardChanceForCategory(string groupName)
    {
        if (!s_Data) return 0.0;
        if (s_Data.Categories && s_Data.Categories.Contains(groupName))
        {
            float c = s_Data.Categories.Get(groupName).HazardChance;
            if (c < 0.0) return 0.0;
            if (c > 1.0) return 1.0;
            return c;
        }
        return 0.0;
    }

    static int GetRandomGloveDamageForCategory(string groupName)
    {
        int dmin = 5;
        int dmax = 15;

        if (s_Data && s_Data.Categories && s_Data.Categories.Contains(groupName))
        {
            TR_CategorySettings cs = s_Data.Categories.Get(groupName);
            dmin = cs.GloveDamageMin;
            dmax = cs.GloveDamageMax;
        }

        if (dmin < 0) dmin = 0;
        if (dmax < dmin) dmax = dmin;

        int span = dmax - dmin + 1;
        if (span <= 0) return dmin;

        return dmin + Math.RandomInt(0, span);
    }

    static string GetDefaultPromptText()
    {
        EnsureLoaded();
        if (GetGame() && (GetGame().IsClient() || !GetGame().IsMultiplayer()))
        {
            string cd = TR_GroupPromptsClient.GetDefault();
            if (cd != "") return cd;
        }
        if (!s_Data) return "";
        return s_Data.DefaultPromptText;
    }

    static string GetNothingFoundText()
    {
        EnsureLoaded();
        if (!s_Data) return "You found nothing.";
        string t = s_Data.NothingFoundText;
        if (t == "") return "You found nothing.";
        return t;
    }

    static string ResolvePromptText(string nodePrompt, string lootGroup, out string outSource)
    {
        string lootGroupNorm = lootGroup;
        if (lootGroupNorm != "")
        {
            lootGroupNorm.Trim();
        }

        EnsureLoaded();
        string txt = "";
        outSource = "Fallback";

        if (nodePrompt != "")
        {
            outSource = "Node";
            txt = nodePrompt;
        }
        else
        {
            string gp = "";
            if (lootGroupNorm != "")
            {
                if (GetGame() && (GetGame().IsClient() || !GetGame().IsMultiplayer()))
                {
                    gp = TR_GroupPromptsClient.GetGroupPrompt(lootGroupNorm);
                    if (gp == "")
                    {
                        gp = TR_LootGroups.GetPromptText(lootGroupNorm);
                    }
                }
                else
                {
                    gp = TR_LootGroups.GetPromptText(lootGroupNorm);
                }
            }

            if (gp != "")
            {
                outSource = "Group";
                txt = gp;
            }
            else
            {
                string dp = "";
                if (GetGame() && (GetGame().IsClient() || !GetGame().IsMultiplayer()))
                {
                    dp = TR_GroupPromptsClient.GetDefault();
                    if (dp == "" && s_Data)
                    {
                        dp = s_Data.DefaultPromptText;
                    }
                }
                else if (s_Data)
                {
                    dp = s_Data.DefaultPromptText;
                }

                if (dp != "")
                {
                    outSource = "Default";
                    txt = dp;
                }
                else
                {
                    outSource = "Fallback";
                    txt = TR_Constants.DEFAULT_RUMMAGE_PROMPT;
                }
            }
        }

        int maxLen = 56;
        if (txt.Length() > maxLen) txt = txt.Substring(0, maxLen);
        return txt;
    }
}
