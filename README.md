# Tenacious Rummaging
Welcome to the Github repo for DayZ mod "Tenacious Rummaging"! You can find the mod on Steam here: https://steamcommunity.com/sharedfiles/filedetails/?id=3576065083

#### Try the mod on our server
The Tenacious DayZ community has its own server with this mod on it. Everyone are welcome to come try it! Server details can be found over at our discord, here: http://discord.gg/WaCXkWSrwe

#### Support
First and foremost, check out the [Wiki here on Github](https://github.com/v1spern/Tenacious-Rummaging/wiki/LootSettings.json) for mod guidance.

You can also find support-channels on our discord (link above). Go look for the "mod-support" channel in the "Tenacious Mods" channel category.

#### Config examples
Look at this repos Wiki for guides on how to set up the JSON structures. Also check out the examples in the "Example configs" folder.

## Mod description

Tenacious Rummaging adds a fully server-authoritative “rummage/search” gameplay loop to DayZ by letting admins mark world objects—and even specific pieces of interior furniture—as searchable nodes.

Players who look at a whitelisted node gets a prompt to search that plays rummaging audio, checks against a cooldowns table, rolls against configurable loot odds, and either spawns weighted loot or fires “failure” events (e.g., zombie aggro, smoke/gas, shock/KO, siren alarm, or even custom events - to a certain degree!).

All configuration lives under $profile:TenaciousRummaging/ in standard JSON files:
* SearchableNodes.json (what can be searched)
* LootGroups.json (what can drop, with weights and item health/quantity overrides)
* LootSettings.json (per-category cooldown, hazard, and event knobs)
* Cooldowns.json (persisted per-player/global cooldowns)

Searchable nodes come in three flavors: static (specific object instance at a quantized position), class-wide (every instance of a given class), and interior-piece (fine-grained furniture “tokens” inside houses, computed from the player’s local position and selection, not just the house itself).

The mod ships client-side prompt gating and tight server validation: clients only see the action where a synced node exists, while the server enforces cooldowns, loot rolls, hazards, and events.

Admin tooling is in-game (AdminStone in hands) and writes changes immediately, then RPC-syncs to clients. The result is a configurable, low-friction rummaging system that’s extensible without repacks.

Feature list

* Three node types: Static, Class-Wide, Interior-Piece (fine furniture tokens).
* Custom audio cues
* Hazard chance when looting
* Flavor text for each action, configurable in the config with fallback texts
* Triggerable event chance when looting (zombies, gas, smoke, shock, knock-out and siren-alarm)
* Client-side prompt whitelist (no spammy actions; interior prompts only in valid contexts).
* Admin-item gated in-game authoring actions for all node types.
* Robust and detailed JSON driven config of all the loot-related functions in the mod.
* Server-authoritative match & spawn with position quantization and radius fallback for statics.
* Search logger: logs each attempt in either predetermined .log, or additionally in a CSV document with CSV-formatted lines (player, node key, roll/chance, outcome). Toggleable in the config.
* Central debug helper with consistent prefixes and gating. Toggleable in the config.

## About

#### Legal stuff
This mod is licensed under GNU GPL 3.0, which means you can share and change all versions of this mod as long as it remains free for all its users.

**I, the author, do not allow this mod to be monetized of repacked in any way or form. This is non-negotiable, and will not be met with a repsonse.**

#### Methods
This mod was created with heavy use of AI guidance, specifically:
* OpenAI GPT-5 High
* OpenAI GPT-5 Codex
* Gemini Pro 2.5
* Claude Sonnet

I am a novice coder and could not have done this without the help of AI.

## Credits

* A huge thank you to the Tenacious DayZ discord community for shits'n'giggles
* Shout out to UsuallyAmie, RubixxLOL and Jordan for testing the mod, giving me lots of ideas for features, inputs and general good shit all around. You guys rock!

###### Also...
I got a lot of inspiration from other mods and modauthors on Steam, especially the "Search for Loot" mod by HunterZ, and the "DayZStory Loot Wreck System" mod by JOKER and Uncle T. Thank you for all the work you've done. This mod would not have been possible without the previous work of the DayZ modder community.

Lastly, a shoutout to the DayZ Modders discord! The best modding resource on the entire interwebs, hands down.