// Core constants & helpers

class TR_Constants
{
	static const string MOD_NAME   = "Tenacious Rummaging";
	static const string MOD_AUTHOR = "v1sper";

	static const float MATCH_RADIUS = 1.5;

	// Paths
	static const string PROFILE = "$profile:TenaciousRummaging/";

	static const string FILE_LOOT_CONFIG   = "LootConfig.json";
	static const string FILE_LOOT_GROUPS   = "LootGroups.json";
	static const string FILE_SETTINGS	   = "LootSettings.json";
	static const string FILE_SEARCH_NODES  = "SearchableNodes.json";
	static const string FILE_COOLDOWNS     = "Cooldowns.json";

	// Interior stuff
	static const string MODELKEY_INTERIOR        = "__interior__";
	static const string MODELKEY_INTERIOR_TOKEN  = "__interior_tok__";

	static string Path(string filename)
	{
		return PROFILE + filename;
	}

	static void EnsureProfile()
	{
		if (!FileExist(PROFILE))
			MakeDirectory(PROFILE);
	}

	static const int   RPC_SEARCH_LOG     = 77001;
	static const int   RPC_ADMIN_ADD_NODE = 77002;
	static const int   RPC_DEBUG_MSG      = 77003;

	// Audio stuff

	static const int   RPC_ID_TR_AUDIO         = 0x054A0D10;
	static const int   TR_AUDIO_KIND_SEARCH    = 1;
	static const int   TR_AUDIO_KIND_FOUND     = 2;
	static const int   TR_AUDIO_KIND_ZOMBIE    = 3;
	static const int   TR_AUDIO_KIND_SHOCK     = 4;
	static const int   TR_AUDIO_KIND_KNOCKOUT  = 5;
	static const int   TR_AUDIO_KIND_SIREN     = 6;

    // Audio radii
    static const float TR_SIREN_RADIUS_M = 1500.0;
}
