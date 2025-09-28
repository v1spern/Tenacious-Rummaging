// Centralized audio utilities

class TR_Audio
{
	// Common helpers

	static void PlayExactSetOnPlayer(PlayerBase player, string setName)
	{
		if (!player)
		{
			TR_Debug.Log("[Audio] PlayExactSetOnPlayer FAIL: player null set=" + setName);
			return;
		}

		bool exists = GetGame().ConfigIsExisting("CfgSoundSets " + setName);
		TR_Debug.Log("[Audio] Config check CfgSoundSets '" + setName + "' exists=" + exists.ToString());

		EffectSound s = null;

		if (exists)
		{
			s = SEffectManager.PlaySoundOnObject(setName, player);
		}

		if (!s)
		{
			s = SEffectManager.PlaySound(setName, player.GetPosition());
			if (s)
			{
				s.SetParent(player);
			}
		}

		if (s)
		{
			s.SetSoundAutodestroy(true);
			s.SetSoundVolume(1.0);
			s.SoundPlay();
			TR_Debug.Log("[Audio] PlayExactSetOnPlayer OK set=" + setName);
		}
		else
		{
			TR_Debug.Log("[Audio] PlayExactSetOnPlayer FAIL create set=" + setName);
		}
	}

	static void PlayRandomSearch(PlayerBase player)
	{
		if (!player) return;

		array<string> choices = new array<string>;
		choices.Insert("trash_search_1_SoundSet");
		choices.Insert("trash_search_2_SoundSet");
		choices.Insert("trash_search_3_SoundSet");
		choices.Insert("trash_search_4_SoundSet");
		choices.Insert("trash_search_5_SoundSet");

		int idx = Math.RandomInt(0, choices.Count());
		string setName = choices.Get(idx);

		TR_Debug.Log("[Audio] PlayRandomSearch choose " + setName);
		PlayExactSetOnPlayer(player, setName);
	}

	static void PlayRandomLootFound(PlayerBase player)
	{
		if (!player) return;

		array<string> choices = new array<string>;
		choices.Insert("loot_found_1_SoundSet");
		choices.Insert("loot_found_2_SoundSet");

		int idx = Math.RandomInt(0, choices.Count());
		string setName = choices.Get(idx);

		TR_Debug.Log("[Audio] PlayRandomLootFound choose " + setName);
		PlayExactSetOnPlayer(player, setName);
	}

	static void PlayRandomFallingObjectAt(vector pos)
	{
		PlayRandomFallingObjectAtInternal(pos, null);
	}

	static void PlayRandomFallingObjectAt(vector pos, PlayerBase attachTo)
	{
		PlayRandomFallingObjectAtInternal(pos, attachTo);
	}

	protected static void PlayRandomFallingObjectAtInternal(vector pos, PlayerBase attachTo)
	{
		array<string> choices = new array<string>;
		choices.Insert("falling_object_1_SoundSet");
		choices.Insert("falling_object_2_SoundSet");
		choices.Insert("falling_object_3_SoundSet");
		choices.Insert("falling_object_4_SoundSet");

		string setName = choices.Get(Math.RandomInt(0, choices.Count()));
		if (!GetGame().ConfigIsExisting("CfgSoundSets " + setName))
		{
			string found = "";
			for (int i = 0; i < choices.Count(); i++)
			{
				if (GetGame().ConfigIsExisting("CfgSoundSets " + choices.Get(i)))
				{
					found = choices.Get(i);
					break;
				}
			}
			if (found == "")
			{
				found = "trash_search_1_SoundSet";
			}
			setName = found;
		}

		string posInfo = "(" + pos[0].ToString() + "," + pos[1].ToString() + "," + pos[2].ToString() + ")";
		TR_Debug.Log("[Audio] PlayRandomFallingObjectAt choose " + setName + " pos=" + posInfo);

		if (attachTo)
		{
			PlayExactSetOnPlayer(attachTo, setName);
			return;
		}

		EffectSound s = SEffectManager.PlaySound(setName, pos);
		if (s)
		{
			s.SetSoundAutodestroy(true);
			s.SetSoundVolume(1.0);
			s.SoundPlay();
			TR_Debug.Log("[Audio] PlayRandomFallingObjectAt OK " + setName);
		}
		else
		{
			TR_Debug.Log("[Audio] PlayRandomFallingObjectAt FAIL " + setName);
		}
	}

	// Shock sting 
	static void PlayRandomShockOnPlayer(PlayerBase player)
	{
		if (!player) return;

		array<string> choices = new array<string>;
		choices.Insert("shock_1_SoundSet");
		choices.Insert("shock_2_SoundSet");
		choices.Insert("shock_3_SoundSet");

		int idx = Math.RandomInt(0, choices.Count());
		string setName = choices.Get(idx);

		TR_Debug.Log("[Audio] PlayRandomShockOnPlayer choose " + setName);
		PlayExactSetOnPlayer(player, setName);
	}

	// KnockOut sting
	static void PlayRandomKnockOutOnPlayer(PlayerBase player)
	{
		if (!player) return;

		array<string> choices = new array<string>;
		choices.Insert("punch_hit_1_SoundSet");
		choices.Insert("punch_hit_2_SoundSet");
		choices.Insert("punch_hit_3_SoundSet");
		choices.Insert("punch_hit_4_SoundSet");

		string setName = choices.Get(Math.RandomInt(0, choices.Count()));
		if (!GetGame().ConfigIsExisting("CfgSoundSets " + setName))
		{
			string found = "";
			for (int i = 0; i < choices.Count(); i++)
			{
				if (GetGame().ConfigIsExisting("CfgSoundSets " + choices.Get(i)))
				{
					found = choices.Get(i);
					break;
				}
			}
			if (found == "")
			{
				found = "loot_found_1_SoundSet";
			}
			setName = found;
		}

		TR_Debug.Log("[Audio] PlayRandomKnockOutOnPlayer choose " + setName);
		PlayExactSetOnPlayer(player, setName);
	}

	// Siren alarm
	static void PlaySirenAt(vector pos)
	{
		PlaySirenAtInternal(pos, null);
	}

	static void PlaySirenAt(vector pos, PlayerBase attachTo)
	{
		PlaySirenAtInternal(pos, attachTo);
	}

	protected static void PlaySirenAtInternal(vector pos, PlayerBase attachTo)
	{
		string setName = "air_raid_alarm_1_SoundSet";
		if (!GetGame().ConfigIsExisting("CfgSoundSets " + setName))
		{
			// Fallback
			setName = "TR_Silence_SoundSet";
		}

		if (attachTo)
		{
			TR_Debug.Log("[Audio] PlaySirenAt attachTo player set=" + setName);
			PlayExactSetOnPlayer(attachTo, setName);
			return;
		}

		EffectSound s = SEffectManager.PlaySound(setName, pos);
		if (s)
		{
			s.SetSoundAutodestroy(true);
			s.SetSoundVolume(1.0);
			s.SoundPlay();
			TR_Debug.Log("[Audio] PlaySirenAt OK " + setName);
		}
		else
		{
			TR_Debug.Log("[Audio] PlaySirenAt FAIL " + setName);
		}
	}
}
