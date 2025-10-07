// Interior search helpers (shared client/server)

class TR_Interior
{
	// Grid resolution
	static const int   INTERIOR_GRID_XZ_CM = 50;
	static const int   INTERIOR_GRID_Y_CM  = 100;
	static const float INTERIOR_ROUND_EPS  = 0.02;

	// ===== Admin Add Path (centralized) =====

	// Resolve the structure from a cursor target:
	// prefer the actual hit object first; only then try its parent.
	static Object ResolveStructureFromTarget(ActionTarget t)
	{
		if (!t) return null;

		Object o = t.GetObject();
		if (o) return o;

		Object p = t.GetParent();
		if (p) return p;

		return null;
	}

	// Lower-cased class key used for DB
	static string GetClassLower(Object o)
	{
		if (!o) return "";
		string t = o.GetType();
		t.ToLower();
		return t;
	}

	// Very conservative structural test; used only for label normalization (never a gate)
	static bool IsStructuralSelection(string s)
	{
		if (!s || s == "") return true;
		string x = s;
		x.ToLower();
		if (x.Contains("wall"))     return true;
		if (x.Contains("ceiling"))  return true;
		if (x.Contains("floor"))    return true;
		if (x.Contains("roof"))     return true;
		if (x.Contains("stairs"))   return true;
		if (x.Contains("pillar"))   return true;
		if (x.Contains("window"))   return true;
		if (x.Contains("frame"))    return true;
		if (x.Contains("door"))     return true;
		if (x.Contains("arch"))     return true;
		if (x.Contains("molding"))  return true;
		if (x.Contains("trim"))     return true;
		if (x.Contains("beam"))     return true;
		if (x.Contains("ledge"))    return true;
		if (x.Contains("sill"))     return true;
		return false;
	}

	// Best-effort selection label; component is optional; falls back to "interior"
	static string ExtractSelectionLabel(ActionTarget target, Object structure, int compIdx)
	{
		string s = "";
		if (compIdx >= 0)
		{
			Object o = target.GetObject();
			if (o)
			{
				string so;
				o.GetActionComponentName(compIdx, so);
				if (so && so != "") s = so;
			}

			if (s == "" && structure)
			{
				string sb;
				structure.GetActionComponentName(compIdx, sb);
				if (sb && sb != "") s = sb;
			}
		}

		if (s == "") return "interior";
		if (IsStructuralSelection(s)) return "interior";
		s.ToLower();
		s.Trim();
		return s;
	}

	// Admin add gating: accept any resolvable structure object from the cursor
	static bool CanAdminAdd(PlayerBase player, ActionTarget target)
	{
		if (!player || !target) return false;

		Object o = ResolveStructureFromTarget(target);
		if (!o) return false;

		// No distance-to-origin gate; large statics (e.g., ATC) may have far origins.

		return true;
	}

	// One-shot admin add from cursor; returns a user-facing status line
	static string AdminAddFromCursor(PlayerBase player, ActionTarget target, string lootGroup)
	{
		if (!player) return "[AddInterior] No player.";
		Object o = ResolveStructureFromTarget(target);
		if (!o) return "[AddInterior] Not a structure.";

		string key = GetClassLower(o);

		int compIdx = target.GetComponentIndex(); // may be -1
		string label = ExtractSelectionLabel(target, o, compIdx);

		// Token from player → structure local (grid quantized)
		string token = ServerComputeFurnitureToken(player, o);

		// Persist
		if (lootGroup == "" || !lootGroup) lootGroup = "general";
		TR_SearchNodesDb.AddInteriorPieceNode(key, token, lootGroup, label);

		return "[AddInterior] Added: " + key + " @ " + token + " [" + label + "] (group: " + lootGroup + ")";
	}

	// ===== Shared math & search helpers (existing paths) =====

	// simple building detection for action gating (used by search UX, not the admin add)
	static bool IsHouseTarget(Object tgt)
	{
	    if (!tgt) return false;

	    // Only care about real Building/House types for the *search* path
	    if (!Building.Cast(tgt) && !House.Cast(tgt)) return false;

	    // Exclusions: allow wrecks, boats and other mod types
	    string t = tgt.GetType();
	    t.ToLower();
		if (t.Contains("staticobj"))   return false;
		if (t.Contains("wreck"))       return false;
		if (t.Contains("blrd"))        return false;
		if (t.Contains("expansion"))   return false;
		if (t.Contains("boat_small"))  return false;
		if (t.Contains("boatsmall"))   return false;
		if (t.Contains("lifeboat"))    return false;
		if (t.Contains("misc"))        return false;

	    return true;
	}

	static string QuantizeLocalXYZToken(vector localPos)
	{
		int mulXZ = 100 / INTERIOR_GRID_XZ_CM; if (mulXZ < 1) mulXZ = 1;
		int mulY  = 100 / INTERIOR_GRID_Y_CM;  if (mulY  < 1) mulY  = 1;

		float xf = localPos[0] * mulXZ;
		float yf = localPos[1] * mulY;
		float zf = localPos[2] * mulXZ;

		if (xf >= 0) xf += INTERIOR_ROUND_EPS; else xf -= INTERIOR_ROUND_EPS;
		if (yf >= 0) yf += INTERIOR_ROUND_EPS; else yf -= INTERIOR_ROUND_EPS;
		if (zf >= 0) zf += INTERIOR_ROUND_EPS; else zf -= INTERIOR_ROUND_EPS;

		int xi = (int)(xf + 0.5);
		int yi = (int)(yf + 0.5);
		int zi = (int)(zf + 0.5);

		return xi.ToString() + "|" + yi.ToString() + "|" + zi.ToString();
	}

	// Token TO local position (approximate cell center)
	static void TokenToLocalXYZ(int xi, int yi, int zi, out float lx, out float ly, out float lz)
	{
		int mulXZ = 100 / INTERIOR_GRID_XZ_CM; if (mulXZ < 1) mulXZ = 1;
		int mulY  = 100 / INTERIOR_GRID_Y_CM;  if (mulY  < 1) mulY  = 1;

		lx = xi / (float)mulXZ;
		ly = yi / (float)mulY;
		lz = zi / (float)mulXZ;
	}

	static void TokenToWorldXYZ(Object o, int xi, int yi, int zi, out float wx, out float wy, out float wz)
	{
		float lx, ly, lz;
		TokenToLocalXYZ(xi, yi, zi, lx, ly, lz);
		// Avoid vector locals: compute from inline ModelToWorld call
		wx = o.ModelToWorld(Vector(lx, ly, lz))[0];
		wy = o.ModelToWorld(Vector(lx, ly, lz))[1];
		wz = o.ModelToWorld(Vector(lx, ly, lz))[2];
	}

	// Token math using generic Object (works for Building/House/HND/statics)
	static string ComputeFurnitureToken(PlayerBase player, Object o)
	{
		if (!player || !o) return "0|0|0";
		// Avoid vector locals by passing inline
		return QuantizeLocalXYZToken(o.WorldToModel(player.GetPosition()));
	}

	static string ServerComputeFurnitureToken(PlayerBase player, Object o)
	{
		return ComputeFurnitureToken(player, o);
	}

	// House-only helper (search/cooldown path)
	static string GetHouseClassLower(House h)
	{
		if (!h) return "";
		string t = h.GetType();
		t.ToLower();
		return t;
	}

	static string BuildInteriorCooldownKey(House h, string furnitureLocalToken)
	{
		string hLow = GetHouseClassLower(h);
		return TR_SearchNodesDb.BuildInteriorCooldownKey(h, hLow, furnitureLocalToken);
	}

	// Prompt for searching (player experience) — unchanged; accepts any Building (covers House + HouseNoDestruct)
	static bool ClientCanPrompt(PlayerBase player, Object candidate)
	{
		if (!player || !candidate) return false;

		Building b = Building.Cast(candidate);
		if (!b) return false;

		string hLow = b.GetType();
		hLow.ToLower();
		if (!TR_SearchNodesDb.HasInteriorForHouse(hLow)) return false;

		// distance check (squared, <= 8 m) — avoid vector locals
		float dx = player.GetPosition()[0] - b.GetPosition()[0];
		float dy = player.GetPosition()[1] - b.GetPosition()[1];
		float dz = player.GetPosition()[2] - b.GetPosition()[2];
		float d2 = dx*dx + dy*dy + dz*dz;
		if (d2 > 64.0) return false; // 8 m
		return true;
	}
}
