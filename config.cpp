// Tenacious Rummaging root config

class CfgPatches
{
    class TenaciousRummaging
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.0;
        requiredAddons[] = {"DZ_Data","DZ_Scripts","DZ_Characters","DZ_Sounds_Effects"};
        author = "v1sper";
        name = "Tenacious Rummaging";
    };
};

class CfgMods
{
    class TenaciousRummaging
    {
        dir = "TenaciousRummaging";

        name = "Tenacious Rummaging";
        author = "v1sper";
        authorID = "76561197992870768";
        version = "1.0.4";
        type = "mod";

        dependencies[] = {"Game","World","Mission"};

        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {
                    "TenaciousRummaging/Scripts/3_Game"
                };
            };

            class worldScriptModule
            {
                value = "";
                files[] = {
                    "TenaciousRummaging/Scripts/4_World"
                };
            };

            class missionScriptModule
            {
                value = "";
                files[] = {
                    "TenaciousRummaging/Scripts/5_Mission"
                };
            };
        };
    };
};

// ----- Vehicles / AnimEvents -----

class CfgVehicles
{
    class Inventory_Base;
    class Man;
    class AdminStone: Inventory_Base
    {
        scope = 2;
        displayName = "Admin Stone";
        descriptionShort = "A mystical stone that grants administrative powers to add nodes to the rummaging-system in Tenacious Rummaging";
        model = "\dz\gear\consumables\Stone.p3d";
        weight = 100;
        itemSize[] = {1,1};
    };
    class AdminPebble: Inventory_Base
    {
        scope = 2;
        displayName = "Admin Pebble";
        descriptionShort = "An administrative placement tool for adding virtual interior rummage nodes in Tenacious Rummaging";
        model = "\dz\gear\consumables\SmallStone.p3d";
        weight = 50;
        itemSize[] = {1,1};
    };

    class SurvivorBase: Man
    {
        class AnimEvents
        {
            class Sounds
            {
                class interactSilent
                {
                    soundSet = "TR_Silence_SoundSet"; id = 41;
                };
            };
        };
    };
};

// ----- Audio -----

class CfgSoundShaders
{
    // Rummage one-shots
    class trash_search_1_SoundShader { samples[] = { {"TenaciousRummaging\Sounds\trash-search-1.ogg",1} }; volume=1.0; range=30; };
    class trash_search_2_SoundShader { samples[] = { {"TenaciousRummaging\Sounds\trash-search-2.ogg",1} }; volume=1.0; range=30; };
    class trash_search_3_SoundShader { samples[] = { {"TenaciousRummaging\Sounds\trash-search-3.ogg",1} }; volume=1.0; range=30; };
    class trash_search_4_SoundShader { samples[] = { {"TenaciousRummaging\Sounds\trash-search-4.ogg",1} }; volume=1.0; range=30; };
    class trash_search_5_SoundShader { samples[] = { {"TenaciousRummaging\Sounds\trash-search-5.ogg",1} }; volume=1.0; range=30; };

    // Loot found stingers
    class loot_found_1_SoundShader   { samples[] = { {"TenaciousRummaging\Sounds\loot-found-1.ogg",1} }; volume=1.0; range=40; };
    class loot_found_2_SoundShader   { samples[] = { {"TenaciousRummaging\Sounds\loot-found-2.ogg",1} }; volume=1.0; range=40; };

    // Falling Objects
    class falling_object_1_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\falling_object-1.ogg",1} }; volume=1.0; range=40; };
    class falling_object_2_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\falling_object-2.ogg",1} }; volume=1.0; range=40; };
    class falling_object_3_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\falling_object-3.ogg",1} }; volume=1.0; range=40; };
    class falling_object_4_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\falling_object-4.ogg",1} }; volume=1.0; range=40; };

    // Punch hits
    class punch_hit_1_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\punch_hit-1.ogg",1} }; volume=1.0; range=40; };
    class punch_hit_2_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\punch_hit-2.ogg",1} }; volume=1.0; range=40; };
    class punch_hit_3_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\punch_hit-3.ogg",1} }; volume=1.0; range=40; };

    // Shocks
    class shock_1_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\shock-1.ogg",1} }; volume=1.0; range=40; };
    class shock_2_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\shock-2.ogg",1} }; volume=1.0; range=40; };
    class shock_3_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\shock-3.ogg",1} }; volume=1.0; range=40; };

    // Air Raid Alarm (1500m range!)
    class air_raid_alarm_1_SoundShader  { samples[] = { {"TenaciousRummaging\Sounds\air-raid-alarm-1.ogg",1} }; volume=1.0; range=1500; };

    // Silence shader
    class TR_Silence_SoundShader    { samples[] = { {"TenaciousRummaging\Sounds\silence.ogg",1} }; volume = 0; range = 0; };
};

class CfgSoundSets
{
    // Rummage one-shot sets
    class trash_search_1_SoundSet { soundShaders[] = {"trash_search_1_SoundShader"}; };
    class trash_search_2_SoundSet { soundShaders[] = {"trash_search_2_SoundShader"}; };
    class trash_search_3_SoundSet { soundShaders[] = {"trash_search_3_SoundShader"}; };
    class trash_search_4_SoundSet { soundShaders[] = {"trash_search_4_SoundShader"}; };
    class trash_search_5_SoundSet { soundShaders[] = {"trash_search_5_SoundShader"}; };

    // Loot found sets
    class loot_found_1_SoundSet   { soundShaders[] = {"loot_found_1_SoundShader"}; };
    class loot_found_2_SoundSet   { soundShaders[] = {"loot_found_2_SoundShader"}; };

    // Falling objects sets
    class falling_object_1_SoundSet   { soundShaders[] = {"falling_object_1_SoundShader"}; };
    class falling_object_2_SoundSet   { soundShaders[] = {"falling_object_2_SoundShader"}; };
    class falling_object_3_SoundSet   { soundShaders[] = {"falling_object_3_SoundShader"}; };
    class falling_object_4_SoundSet   { soundShaders[] = {"falling_object_4_SoundShader"}; };

    // Punch hits sets
    class punch_hit_1_SoundSet   { soundShaders[] = {"punch_hit_1_SoundShader"}; };
    class punch_hit_2_SoundSet   { soundShaders[] = {"punch_hit_2_SoundShader"}; };
    class punch_hit_3_SoundSet   { soundShaders[] = {"punch_hit_3_SoundShader"}; };

    // Shocks sets
    class shock_1_SoundSet   { soundShaders[] = {"shock_1_SoundShader"}; };
    class shock_2_SoundSet   { soundShaders[] = {"shock_2_SoundShader"}; };
    class shock_3_SoundSet   { soundShaders[] = {"shock_3_SoundShader"}; };

    // Air Raid Alarm set
    class air_raid_alarm_1_SoundSet   { soundShaders[] = {"air_raid_alarm_1_SoundShader"}; };

    // Silence set
    class TR_Silence_SoundSet    { soundShaders[] = {"TR_Silence_SoundShader"}; volumeFactor = 0; spatial = 0; doppler = 0; loop = 0; };
};