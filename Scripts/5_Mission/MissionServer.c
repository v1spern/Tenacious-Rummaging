// Missionserver

modded class MissionServer
{
    ref TR_NodeCooldownSystem TR_Cooldowns;

    override void OnInit()
    {
        super.OnInit();
        TR_Debug.Log("STARTUP STATUS: OnInit reached");

        // Profile directory
        TR_Constants.EnsureProfile();
        TR_Debug.Log("STARTUP STATUS: Profile directory validated");

        // Load loot system
        TR_InitLootSystem();
        TR_Debug.Log("STARTUP STATUS: Loot system loaded");

        // Groups DB
        TR_LootGroups.Load();
        TR_Debug.Log("STARTUP STATUS: Loot groups loaded");

        // Searchable nodes DB
        TR_SearchNodesDb.Init();
        TR_Debug.Log("Loot nodes loaded (server ready to sync to clients)");

        // Per-node cooldown system
        TR_NodeCooldownSystem.Get().Load();
        TR_Debug.Log("Saved Cooldowns loaded from file");

        TR_Debug.Log("MissionServer init complete (Tenacious Rummaging Mod ready).");
        Print(" ----- [Tenacious Rummaging] - OnInit completed, mod loaded ----- ");
    }

    void TR_InitLootSystem()
    {
        TR_Debug.Log("STARTUP STATUS: Initializing loot settings...");
        TR_LootSettingsManager.Load();

        // Enable/disable debug logging based on LootSettings.json
        TR_Debug.SetEnabled(TR_LootSettingsManager.IsDebugEnabled());

        if (TR_LootSettingsManager.IsDebugEnabled())
        {
            TR_Debug.Log("Debug logging ENABLED via LootSettings.json");
        }
        else
        {
            Print(" ----- [Tenacious Rummaging] - Debug mode is DISABLED (set DebugMode to 1 in LootSettings.json to enable) ----- ");
        }
    }

    override void OnMissionFinish()
    {
        if (TR_Cooldowns) TR_Cooldowns.Save();
        TR_Debug.Log("SHUTDOWN STATUS: MissionServer finish (Tenacious Rummaging saved state).");
        super.OnMissionFinish();
    }

    // Send nodes to clients when they connect
    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);

        #ifdef SERVER
        if (player && identity)
        {
            TR_SearchNodesDb.SyncAllToClient(player, identity);
            TR_Debug.Log("[MissionServer] Sent " + TR_SearchNodesDb.Count().ToString() + " search nodes to " + identity.GetName());
        }
        #endif
    }
}
