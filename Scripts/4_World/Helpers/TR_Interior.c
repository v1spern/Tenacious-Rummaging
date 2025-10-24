// Interior search helpers (shared client/server)

class TR_Interior
{
	// Grid resolution
	static const int   INTERIOR_GRID_XZ_CM = 50;
	static const int   INTERIOR_GRID_Y_CM  = 100;
	static const float INTERIOR_ROUND_EPS  = 0.02;

	// ===== Proxy-aware helpers (admin add path) =====

	// Get the exact object hit by the cursor (CCTCursor supplies this)
	protected static Object ResolveHit(ActionTarget t)
	{
		if (!t) return null;
		Object o = t.GetObject();
		if (o) return o;
		return t.GetParent(); // last resort
	}

	// Walk up parents to find the first Building ancestor (covers House & HouseNoDestruct)
	protected static Building FindBuildingAncestor(Object start)
	{
		if (!start) return null;

		Object cur = start;
		int hops = 0;
		const int MAX_HOPS = 8;

		while (cur && hops < MAX_HOPS)
		{
			Building b;
			if (Class.CastTo(b, cur)) return b;
			cur = cur.GetParent();
			hops++;
		}
		return null;
	}

	// Lower-cased class key
	static string GetClassLower(Object o)
	{
		if (!o) return "";
		string t = o.GetType();
		t.ToLower();
		return t;
	}

	// Selection label best-effort; never gates
	protected static string ExtractSelectionLabel(Object onObject, int compIdx)
	{
		if (compIdx < 0 || !onObject) return "interior";
		string s;
		onObject.GetActionComponentName(compIdx, s);
		if (!s || s == "") return "interior";
		s.ToLower(); s.Trim();
		return s;
	}

	// === GATE: allow only when aiming a PROXY child that belongs to a Building ===
	static bool CanAdminAdd(PlayerBase player, ActionTarget target)
	{
		if (!player || !target) return false;

		Object   hit = ResolveHit(target);
		if (!hit) return false;

		Building parentB = FindBuildingAncestor(hit);
		if (!parentB) return false;

		// If the hit object IS the building mesh, deny (prevents walls/ceilings/doors/windows adds)
		if (hit == parentB) return false;

		// Otherwise it's a proxy under a building — allow
		return true;
	}

	// One-shot add: persist under the building ancestor, compute token in building space
	static string AdminAddFromCursor(PlayerBase player, ActionTarget target, string lootGroup)
	{
		if (!player || !target) return "[AddInterior] No player/target.";

		Object   hit = ResolveHit(target);
		Building b   = FindBuildingAncestor(hit);

		if (!hit || !b) return "[AddInterior] Not a valid building proxy.";

		// DB key from building parent
		string buildingKey = GetClassLower(b);

		// Label from HIT (proxy), fall back to "interior"
		int compIdx = target.GetComponentIndex(); // may be -1
		string label = ExtractSelectionLabel(hit, compIdx);

		// Token from player -> BUILDING local (grid quantized)
		string token = ComputeFurnitureToken(player, b);

		// Group default
		if (!lootGroup || lootGroup == "") lootGroup = "general";

		TR_SearchNodesDb.AddInteriorPieceNode(buildingKey, token, lootGroup, label);

		return "[AddInterior] Added: " + buildingKey + " @ " + token + " [" + label + "] (group: " + lootGroup + ")";
	}

	// ===== Shared math & search helpers (existing paths) =====

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

	// Token TO local position (approx cell center)
	static void TokenToLocalXYZ(int xi, int yi, int zi, out float lx, out float ly, out float lz)
	{
		int mulXZ = 100 / INTERIOR_GRID_XZ_CM; if (mulXZ < 1) mulXZ = 1;
		int mulY  = 100 / INTERIOR_GRID_Y_CM;  if (mulY  < 1) mulY  = 1;

		lx = xi / (float)mulXZ;
		ly = yi / (float)mulY;
		lz = zi / (float)mulXZ;
	}

	static void TokenToWorldXYZ(Building b, int xi, int yi, int zi, out float wx, out float wy, out float wz)
	{
		float lx, ly, lz;
		TokenToLocalXYZ(xi, yi, zi, lx, ly, lz);
		wx = b.ModelToWorld(Vector(lx, ly, lz))[0];
		wy = b.ModelToWorld(Vector(lx, ly, lz))[1];
		wz = b.ModelToWorld(Vector(lx, ly, lz))[2];
	}

	// SINGLE TOKEN FUNCTION (avoid overload ambiguity elsewhere)
	static string ComputeFurnitureToken(PlayerBase player, Building b)
	{
		if (!player || !b) return "0|0|0";
		return QuantizeLocalXYZToken(b.WorldToModel(player.GetPosition()));
	}

	static string ServerComputeFurnitureToken(PlayerBase player, Building b)
	{
		return ComputeFurnitureToken(player, b);
	}

	// Search UX
	static bool IsHouseTarget(Object tgt)
	{
		if (!tgt) return false;

		Building asB; House asH;
		bool isB = Class.CastTo(asB, tgt);
		bool isH = Class.CastTo(asH, tgt);
		if (!isB && !isH) return false;

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

	static string GetHouseClassLower(House h)
	{
		if (!h) return "";
		string t = h.GetType();
		t.ToLower();
		return t;
	}

	static bool ClientCanPrompt(PlayerBase player, Object candidate)
	{
		if (!player || !candidate) return false;
	
		Building b;
		if (!Class.CastTo(b, candidate)) return false;
	
		string hLow = b.GetType();
		hLow.ToLower();
		if (!TR_SearchNodesDb.HasInteriorForHouse(hLow)) return false;
	
		float dx = player.GetPosition()[0] - b.GetPosition()[0];
		float dy = player.GetPosition()[1] - b.GetPosition()[1];
		float dz = player.GetPosition()[2] - b.GetPosition()[2];
		float d2 = dx*dx + dy*dy + dz*dz;
		if (d2 > 64.0) return false;
		return true;
	}
	
	static string BuildInteriorCooldownKey(House h, string furnitureLocalToken)
	{
		string hLow = GetHouseClassLower(h);
		return TR_SearchNodesDb.BuildInteriorCooldownKey(h, hLow, furnitureLocalToken);
	}
}
