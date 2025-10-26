// MissionServer

modded class MissionServer
{
    ref TR_NodeCooldownSystem TR_Cooldowns;

    override void OnInit()
    {
        super.OnInit();
        Print(" ---------------- [Tenacious Rummaging] STARTUP STATUS: OnInit reached ---------------- ");

        TR_Constants.EnsureProfile();
        Print(" ---------------- [Tenacious Rummaging] STARTUP STATUS: Profile directory OK ---------------- ");

        TR_InitLootSystem();
        TR_Debug.Log("[Mission] Loot system loaded");

        TR_LootGroups.Load();
        TR_Debug.Log("[Mission] Loot groups loaded");

        TR_SearchNodesDb.Init();
        TR_Debug.Log("[Mission] Loot nodes loaded (server ready to sync to clients)");

        TR_Cooldowns = TR_NodeCooldownSystem.Get();
        if (TR_Cooldowns) TR_Cooldowns.Load();
        TR_Debug.Log("[Mission] Stored cooldowns loaded from file");

        TR_Debug.Log("[Mission] MissionServer initialization completed (Tenacious Rummaging ready)");

        Print(" ---------------- [Tenacious Rummaging] STARTUP STATUS: OnInit completed, mod loaded ---------------- ");
    }

    void TR_InitLootSystem()
    {
        TR_Debug.Log("[Mission] STARTUP STATUS: Initializing loot settings...");
        TR_LootSettingsManager.Load();

        TR_Debug.SetEnabled(TR_LootSettingsManager.IsDebugEnabled());

        if (TR_LootSettingsManager.IsDebugEnabled())
        {
            TR_Debug.Log("[Mission] Debug logging ENABLED via LootSettings.json - Let's have a look!");
        }
        else
        {
            Print(" ---------------- [Tenacious Rummaging] - Debug mode is DISABLED (set DebugMode to 1 in LootSettings.json to enable) ---------------- ");
        }
    }

    override void OnMissionFinish()
    {
        if (TR_Cooldowns) TR_Cooldowns.Save();
        Print(" ---------------- [Tenacious Rummaging] SHUTDOWN STATUS: MissionServer finish (Tenacious Rummaging saved state) ---------------- ");
        super.OnMissionFinish();

        #ifdef SERVER
        TR_SearchNodesDb.Save();
        #endif
    }

    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);

        #ifdef SERVER
        if (player && identity)
        {
            TR_SearchNodesDb.SyncAllToClient(player, identity);
            string nm = identity.GetName();
            string pid = identity.GetId();
            TR_Debug.Log("[MissionServer][PlayerConnect] Syncing '" + TR_SearchNodesDb.Count().ToString() + "' search nodes to player '" + nm + "' (" + pid + ")");
        }
        #endif
    }
}