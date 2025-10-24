// Rummage events

class TR_RummageEvent_ZombieSpawn
{
    float Chance;
    int   CountMin;
    int   CountMax;
    float SpawnRadius;
    ref array<string> ZombieTypes;
    float AlertNoiseStrength;
    float AlertNoiseRange;
    string FlavorText;

    void TR_RummageEvent_ZombieSpawn()
    {
        Chance = 0.0;
        CountMin = 1;
        CountMax = 1;
        SpawnRadius = 8.0;
        ZombieTypes = new array<string>();

        AlertNoiseStrength = 50.0;
        AlertNoiseRange = 30.0;

        FlavorText = "Your rummaging makes a lot of noise and draws immediate attention from nearby infected!";
    }
}

class TR_RummageEvent_Smoke
{
    float Chance;
    string Classname;
    float FuseSeconds;
    string FlavorText;

    void TR_RummageEvent_Smoke()
    {
        Chance = 0.0;
        Classname = "M18SmokeGrenade_Red";
        FuseSeconds = 1.0;

        FlavorText = "You jostle the stash and a smoke grenade hisses, flooding the area.";
    }
}

class TR_RummageEvent_Gas
{
    float Chance;
    string Classname;
    float FuseSeconds;
    string FlavorText;

    void TR_RummageEvent_Gas()
    {
        Chance = 0.0;
        Classname = "Grenade_ChemGas";
        FuseSeconds = 1.0;

        FlavorText = "You disturb the cache and a toxic canister pops, releasing choking gas.";
    }
}

class TR_RummageEvent_KnockOut
{
    float Chance;
    float DurationSeconds;
    float HealthDamage;
    string FlavorText;

    void TR_RummageEvent_KnockOut()
    {
        Chance = 0.0;
        DurationSeconds = 10.0;
        HealthDamage = 25.0;

        FlavorText = "A heavy hit staggers you and you collapse unconscious.";
    }
}

class TR_RummageEvent_Shock
{
    float Chance;
    float ShockAmount;
    float HealthDamage;
    string FlavorText;

    void TR_RummageEvent_Shock()
    {
        Chance = 0.0;
        ShockAmount = 80.0;
        HealthDamage = 25.0;

        FlavorText = "A surge of electricity tears through you. Your muscles seize!";
    }
}

// Siren alarm event
class TR_RummageEvent_SirenAlarm
{
    float Chance;
    string FlavorText;

    void TR_RummageEvent_SirenAlarm()
    {
        Chance = 0.0;

        FlavorText = "An air-raid siren blares across the area!";
    }
}

class TR_RummageEventConfig
{
    ref TR_RummageEvent_ZombieSpawn ZombieSpawn;
    ref TR_RummageEvent_Smoke       Smoke;
    ref TR_RummageEvent_Gas         Gas;
    ref TR_RummageEvent_KnockOut    KnockOut;
    ref TR_RummageEvent_Shock       Shock;
    ref TR_RummageEvent_SirenAlarm  SirenAlarm;

    void TR_RummageEventConfig()
    {
        ZombieSpawn = new TR_RummageEvent_ZombieSpawn();
        Smoke = new TR_RummageEvent_Smoke();
        Gas = new TR_RummageEvent_Gas();
        KnockOut = new TR_RummageEvent_KnockOut();
        Shock = new TR_RummageEvent_Shock();
        SirenAlarm = new TR_RummageEvent_SirenAlarm();
    }
}

class TR_RummageEventCategoryConfig
{
    ref TR_RummageEventConfig Events;

    void TR_RummageEventCategoryConfig()
    {
        Events = new TR_RummageEventConfig();
    }
}

class TR_RummageEventRoot
{
    ref map<string, ref TR_RummageEventCategoryConfig> Categories;

    void TR_RummageEventRoot()
    {
        Categories = new map<string, ref TR_RummageEventCategoryConfig>();
    }
}

class TR_RummageEventManager
{
    protected static ref TR_RummageEventRoot s_Cache;

    protected static void EnsureLoaded()
    {
        if (!GetGame().IsServer())
            return;

        if (!s_Cache)
            s_Cache = new TR_RummageEventRoot();

        string path = "$profile:TenaciousRummaging/LootSettings.json";
        if (!FileExist(path))
        {
            TR_Debug.Log("[RummageEventManager] LootSettings.json not found; events disabled.");
            return;
        }

        TR_RummageEventRoot tmp = new TR_RummageEventRoot();
        JsonFileLoader<TR_RummageEventRoot>.JsonLoadFile(path, tmp);

        if (!tmp || !tmp.Categories)
        {
            TR_Debug.Log("[RummageEventManager] LootSettings.json has no Categories; events disabled.");
            return;
        }

        s_Cache = tmp;
    }

    protected static float Rand01()
    {
        return Math.RandomFloatInclusive(0.0, 1.0);
    }

    protected static string GetPlayerNameSafe(PlayerBase player)
    {
        if (!player) return "unknown";
        PlayerIdentity id = player.GetIdentity();
        if (id) return id.GetName();
        return "unknown";
    }

    protected static string VecToShortStr(vector v)
    {
        return "(" + v[0].ToString() + "," + v[1].ToString() + "," + v[2].ToString() + ")";
    }

    protected static vector RandomAround(vector center, float radius)
    {
        float a = Math.RandomFloat(0.0, Math.PI2);
        float r = Math.RandomFloat(0.0, radius);
        vector off = Vector(Math.Cos(a) * r, 0, Math.Sin(a) * r);
        vector pos = center + off;
        pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
        return pos;
    }

    protected static string PickZombieType(array<string> types)
    {
        if (types && types.Count() > 0)
        {
            int idx = Math.RandomInt(0, types.Count());
            return types.Get(idx);
        }
        array<string> fallback = {
            "ZmbM_CitizenASkinny_Brown",
            "ZmbM_CitizenBFat_Blue",
            "ZmbF_CitizenANormal_Beige",
            "ZmbF_BlueCollarFat_White"
        };
        int j = Math.RandomInt(0, fallback.Count());
        return fallback.Get(j);
    }

    // Immediate aggro helpers

    protected static void ForceImmediateAggro(DayZInfected infected, PlayerBase player)
    {
        if (!infected || !player) return;

        int damageType = 1;
        string ammoType = "MeleeDamage";
        vector mdlPos = "0 0 0";

        infected.ProcessDirectDamage(damageType, player, "", ammoType, mdlPos, 1.0);

        vector dir = player.GetPosition() - infected.GetPosition();
        float yaw = Math.Atan2(dir[0], dir[2]) * Math.RAD2DEG;
        vector o = infected.GetOrientation();
        o[1] = yaw;
        infected.SetOrientation(o);
    }

    protected static void AggroExistingInfectedNear(PlayerBase player, vector atPos, float range)
    {
        if (!player) return;
        if (range <= 0) range = 25.0;

        array<Object> objs = new array<Object>;
        array<CargoBase> c = new array<CargoBase>;
        GetGame().GetObjectsAtPosition(atPos, range, objs, c);

        int affected = 0;
        for (int i = 0; i < objs.Count(); i++)
        {
            DayZInfected z = DayZInfected.Cast(objs[i]);
            if (z)
            {
                ForceImmediateAggro(z, player);
                affected++;
            }
        }

        TR_Debug.Log("[RummageEvent] AggroExistingInfectedNear affected=" + affected.ToString() + " range=" + range.ToString() + " pos=" + VecToShortStr(atPos));
    }

    protected static void PlayZombieAlertSound(PlayerBase player, vector soundPos)
    {
        if (GetGame().IsMultiplayer() && GetGame().IsServer())
        {
            if (player)
            {
                PlayerIdentity id = player.GetIdentity();
                if (id)
                {
                    Param2<int, vector> p = new Param2<int, vector>(TR_Constants.TR_AUDIO_KIND_ZOMBIE, soundPos);
                    player.RPCSingleParam(TR_Constants.RPC_ID_TR_AUDIO, p, true, id);
                    return;
                }
                TR_Debug.Log("[RummageEvent] ZombieSpawn sound fallback: missing identity for player");
            }
            else
            {
                TR_Debug.Log("[RummageEvent] ZombieSpawn sound fallback: player is null");
            }
        }

        if (!GetGame().IsMultiplayer() || GetGame().IsClient())
        {
            TR_Audio.PlayRandomFallingObjectAt(soundPos, player);
        }
    }

    // Shock SFX
    protected static void PlayShockSound(PlayerBase player, vector soundPos)
    {
        if (GetGame().IsMultiplayer() && GetGame().IsServer())
        {
            if (player)
            {
                PlayerIdentity id = player.GetIdentity();
                if (id)
                {
                    Param2<int, vector> p = new Param2<int, vector>(TR_Constants.TR_AUDIO_KIND_SHOCK, soundPos);
                    player.RPCSingleParam(TR_Constants.RPC_ID_TR_AUDIO, p, true, id);
                    return;
                }
                TR_Debug.Log("[RummageEvent] Shock sound fallback: missing identity for player");
            }
            else
            {
                TR_Debug.Log("[RummageEvent] Shock sound fallback: player is null");
            }
        }

        if (!GetGame().IsMultiplayer() || GetGame().IsClient())
        {
            TR_Audio.PlayRandomShockOnPlayer(player);
        }
    }

    // KnockOut SFX
    protected static void PlayKnockOutSound(PlayerBase player, vector soundPos)
    {
        if (GetGame().IsMultiplayer() && GetGame().IsServer())
        {
            if (player)
            {
                PlayerIdentity id = player.GetIdentity();
                if (id)
                {
                    Param2<int, vector> p = new Param2<int, vector>(TR_Constants.TR_AUDIO_KIND_KNOCKOUT, soundPos);
                    player.RPCSingleParam(TR_Constants.RPC_ID_TR_AUDIO, p, true, id);
                    return;
                }
                TR_Debug.Log("[RummageEvent] KnockOut sound fallback: missing identity for player");
            }
            else
            {
                TR_Debug.Log("[RummageEvent] KnockOut sound fallback: player is null");
            }
        }

        if (!GetGame().IsMultiplayer() || GetGame().IsClient())
        {
            TR_Audio.PlayRandomKnockOutOnPlayer(player);
        }
    }

    // Broadcast positional audio to players in range
    protected static void BroadcastAudioInRange(int kind, vector soundPos, float radiusM)
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);

        int sent = 0;
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase pb = PlayerBase.Cast(players[i]);
            if (!pb) continue;

            vector pp = pb.GetPosition();
            float dist = vector.Distance(pp, soundPos);
            if (dist <= radiusM)
            {
                PlayerIdentity id = pb.GetIdentity();
                if (id)
                {
                    Param2<int, vector> msg = new Param2<int, vector>(kind, soundPos);
                    pb.RPCSingleParam(TR_Constants.RPC_ID_TR_AUDIO, msg, true, id);
                    sent++;
                }
            }
        }

        TR_Debug.Log("[RummageEvent] BroadcastAudioInRange kind=" + kind.ToString() + " radius=" + radiusM.ToString() + " sent=" + sent.ToString());
    }

    // Siren SFX
    protected static void PlaySirenSound(vector soundPos)
    {
        if (GetGame().IsMultiplayer() && GetGame().IsServer())
        {
            BroadcastAudioInRange(TR_Constants.TR_AUDIO_KIND_SIREN, soundPos, TR_Constants.TR_SIREN_RADIUS_M);
            return;
        }

        if (!GetGame().IsMultiplayer())
        {
            TR_Audio.PlaySirenAt(soundPos);
        }
    }

    protected static void TryZombieSpawn(PlayerBase player, vector pos, TR_RummageEvent_ZombieSpawn cfg)
    {
        if (cfg.Chance <= 0.0) return;
        float roll = Rand01();
        if (roll > cfg.Chance) return;

        int nmin = cfg.CountMin;
        int nmax = cfg.CountMax;
        if (nmin < 1) nmin = 1;
        if (nmax < nmin) nmax = nmin;

        float radius = cfg.SpawnRadius;
        if (radius <= 0.0) radius = 1.0;

        int targetCount = Math.RandomIntInclusive(nmin, nmax);
        int spawned = 0;

        array<DayZInfected> spawnedList = new array<DayZInfected>();

        for (int i = 0; i < targetCount; i++)
        {
            string ztype = PickZombieType(cfg.ZombieTypes);
            float angle = Math.RandomFloatInclusive(0.0, Math.PI2);
            vector sp = pos;
            sp[0] = pos[0] + Math.Cos(angle) * radius;
            sp[2] = pos[2] + Math.Sin(angle) * radius;
            sp[1] = GetGame().SurfaceY(sp[0], sp[2]) + 0.3;

            Object obj = GetGame().CreateObjectEx(ztype, sp, ECE_KEEPHEIGHT | ECE_UPDATEPATHGRAPH | ECE_CREATEPHYSICS | ECE_INITAI);
            DayZInfected infected = DayZInfected.Cast(obj);
            EntityAI eai = EntityAI.Cast(obj);

            if (!infected)
            {
                if (obj)
                {
                    GetGame().ObjectDelete(obj);
                }
                TR_Debug.Log("[RummageEvent] ZombieSpawn FAIL type=" + ztype + " pos=" + VecToShortStr(sp));
                continue;
            }

            if (eai)
            {
                eai.SetAffectPathgraph(true, false);
            }

            infected.SetPosition(sp);

            vector orient = infected.GetOrientation();
            orient[1] = Math.RandomFloatInclusive(0.0, 360.0);
            infected.SetOrientation(orient);

            spawnedList.Insert(infected);
            spawned++;
        }

        if (spawned <= 0)
        {
            TR_Debug.Log("[RummageEvent] ZombieSpawn: no infected created pos=" + VecToShortStr(pos));
            return;
        }

        for (int j = 0; j < spawnedList.Count(); j++)
        {
            ForceImmediateAggro(spawnedList.Get(j), player);
        }

        float r = cfg.AlertNoiseRange;
        if (r <= 0) r = 30.0;
        AggroExistingInfectedNear(player, pos, r);

        vector soundPos = pos;
        soundPos[1] = GetGame().SurfaceY(soundPos[0], soundPos[2]) + 0.3;
        PlayZombieAlertSound(player, soundPos);

        if (player && cfg.FlavorText != "")
        {
            TR_Notify.Send(player, cfg.FlavorText);
        }

        TR_Debug.Log("[RummageEvent] TRIGGER ZombieSpawn count=" + spawned.ToString() + " radius=" + cfg.SpawnRadius.ToString() + " chance=" + cfg.Chance.ToString() + " roll=" + roll.ToString() + " pos=" + VecToShortStr(pos));
    }

    protected static void TrySmoke(PlayerBase player, vector pos, TR_RummageEvent_Smoke cfg)
    {
        if (cfg.Chance <= 0.0) return;
        float roll = Rand01();
        if (roll > cfg.Chance) return;
        if (cfg.Classname == "") return;

        vector sp = pos;
        sp[1] = GetGame().SurfaceY(sp[0], sp[2]);
        EntityAI eai = EntityAI.Cast(GetGame().CreateObjectEx(cfg.Classname, sp, ECE_PLACE_ON_SURFACE));
        if (eai)
        {
            Grenade_Base gb = Grenade_Base.Cast(eai);
            if (gb)
            {
                gb.SetFuseDelay(cfg.FuseSeconds);
                gb.Unpin();
                if (player && cfg.FlavorText != "")
                {
                    TR_Notify.Send(player, cfg.FlavorText);
                }
                TR_Debug.Log("[RummageEvent] TRIGGER Smoke class=" + cfg.Classname + " fuse_s=" + cfg.FuseSeconds.ToString() + " chance=" + cfg.Chance.ToString() + " roll=" + roll.ToString() + " pos=" + VecToShortStr(sp));
            }
        }
    }

    protected static void TryGas(PlayerBase player, vector pos, TR_RummageEvent_Gas cfg)
    {
        if (cfg.Chance <= 0.0) return;
        float roll = Rand01();
        if (roll > cfg.Chance) return;
        if (cfg.Classname == "") return;

        vector sp = pos;
        sp[1] = GetGame().SurfaceY(sp[0], sp[2]);
        EntityAI eai = EntityAI.Cast(GetGame().CreateObjectEx(cfg.Classname, sp, ECE_PLACE_ON_SURFACE));
        if (eai)
        {
            Grenade_Base gb = Grenade_Base.Cast(eai);
            if (gb)
            {
                gb.SetFuseDelay(cfg.FuseSeconds);
            }
            eai.SetHealth("", "", 0.0);
            if (player && cfg.FlavorText != "")
            {
                TR_Notify.Send(player, cfg.FlavorText);
            }
            TR_Debug.Log("[RummageEvent] TRIGGER Gas(RUIN) class=" + cfg.Classname + " fuse_s=" + cfg.FuseSeconds.ToString() + " chance=" + cfg.Chance.ToString() + " roll=" + roll.ToString() + " pos=" + VecToShortStr(sp));
        }
    }

    protected static void RestoreShock(PlayerBase p, float amount)
    {
        if (!p) return;
        p.AddHealth("", "Shock", amount);
        TR_Debug.Log("[RummageEvent] KnockOut RECOVER amount=" + amount.ToString() + " player=" + GetPlayerNameSafe(p) + " pos=" + VecToShortStr(p.GetPosition()));
    }

    protected static void TryKnockOut(PlayerBase player, TR_RummageEvent_KnockOut cfg)
    {
        if (cfg.Chance <= 0.0) return;
        float roll = Rand01();
        if (roll > cfg.Chance) return;
        if (!player) return;

        float curShock = player.GetHealth("", "Shock");
        float drop = curShock + 5.0;
        player.AddHealth("", "Shock", -drop);

        if (cfg.FlavorText != "")
        {
            TR_Notify.Send(player, cfg.FlavorText);
        }

        float hpDmg = cfg.HealthDamage;
        if (hpDmg > 0.0)
        {
            player.AddHealth("", "", -hpDmg);
        }

        vector soundPos = player.GetPosition();
        soundPos[1] = GetGame().SurfaceY(soundPos[0], soundPos[2]) + 0.3;
        PlayKnockOutSound(player, soundPos);

        int ms = Math.Round(cfg.DurationSeconds * 1000.0);
        if (ms < 1000) ms = 1000;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(RestoreShock, ms, false, player, drop);

        TR_Debug.Log("[RummageEvent] TRIGGER KnockOut player=" + GetPlayerNameSafe(player) + " KO_drop=" + drop.ToString() + " hp_dmg=" + hpDmg.ToString() + " duration_s=" + cfg.DurationSeconds.ToString() + " chance=" + cfg.Chance.ToString() + " roll=" + roll.ToString() + " pos=" + VecToShortStr(player.GetPosition()));
    }

    // Shock
    protected static void TryShock(PlayerBase player, TR_RummageEvent_Shock cfg)
    {
        if (cfg.Chance <= 0.0) return;
        float roll = Rand01();
        if (roll > cfg.Chance) return;
        if (!player) return;

        float shockAmt = cfg.ShockAmount;
        if (shockAmt < 0.0) shockAmt = -shockAmt;
        player.AddHealth("", "Shock", -shockAmt);

        float hpDmg = cfg.HealthDamage;
        if (hpDmg > 0.0)
        {
            player.AddHealth("", "", -hpDmg);
        }

        vector soundPos = player.GetPosition();
        soundPos[1] = GetGame().SurfaceY(soundPos[0], soundPos[2]) + 0.3;
        PlayShockSound(player, soundPos);

        if (cfg.FlavorText != "")
        {
            TR_Notify.Send(player, cfg.FlavorText);
        }

        TR_Debug.Log("[RummageEvent] TRIGGER Shock player=" + GetPlayerNameSafe(player) + " shock=" + shockAmt.ToString() + " hp_dmg=" + hpDmg.ToString() + " chance=" + cfg.Chance.ToString() + " roll=" + roll.ToString() + " pos=" + VecToShortStr(player.GetPosition()));
    }

    // Siren
    protected static void TrySirenAlarm(PlayerBase player, vector pos, TR_RummageEvent_SirenAlarm cfg)
    {
        if (cfg.Chance <= 0.0) return;
        float roll = Rand01();
        if (roll > cfg.Chance) return;

        vector soundPos = pos;
        soundPos[1] = GetGame().SurfaceY(soundPos[0], soundPos[2]) + 0.3;
        PlaySirenSound(soundPos);

        if (player && cfg.FlavorText != "")
        {
            TR_Notify.Send(player, cfg.FlavorText);
        }

        TR_Debug.Log("[RummageEvent] TRIGGER SirenAlarm chance=" + cfg.Chance.ToString() + " roll=" + roll.ToString() + " pos=" + VecToShortStr(soundPos));
    }

    protected static TR_RummageEventConfig GetEventsForCategory(string category)
    {
        if (!s_Cache || !s_Cache.Categories) return null;
        TR_RummageEventCategoryConfig cc = s_Cache.Categories.Get(category);
        if (!cc) return null;
        return cc.Events;
    }

    static void TriggerOnFailedRummage(PlayerBase player, string category, vector atPos)
    {
        if (!GetGame().IsServer()) return;
        EnsureLoaded();
        if (!player || category == "") return;

        TR_RummageEventConfig ev = GetEventsForCategory(category);
        if (!ev) return;

        string pname = GetPlayerNameSafe(player);
        TR_Debug.Log("[RummageEvent] EVALUATE player=" + pname + " category=" + category + " pos=" + VecToShortStr(atPos));

        if (ev.ZombieSpawn) TryZombieSpawn(player, atPos, ev.ZombieSpawn);
        if (ev.Smoke)       TrySmoke(player, atPos, ev.Smoke);
        if (ev.Gas)         TryGas(player, atPos, ev.Gas);
        if (ev.KnockOut)    TryKnockOut(player, ev.KnockOut);
        if (ev.Shock)       TryShock(player, ev.Shock);
        if (ev.SirenAlarm)  TrySirenAlarm(player, atPos, ev.SirenAlarm);
    }
}
