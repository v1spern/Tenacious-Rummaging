// Interior search helpers (shared client/server)

class TR_Interior
{
	// Grid resolution
	static const int   INTERIOR_GRID_XZ_CM = 50;
	static const int   INTERIOR_GRID_Y_CM  = 100;
	static const float INTERIOR_ROUND_EPS  = 0.02;

	// simple building detection for action gating
	static bool IsHouseTarget(Object tgt)
	{
		if (!tgt) return false;
		// Hide static/class actions whenever the target is any building/house
		if (House.Cast(tgt))    return true;
		if (Building.Cast(tgt)) return true;
		return false;
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

	// Token TO local position (approximatelyish cell center)
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
		vector loc = Vector(lx, ly, lz);
		vector w = b.ModelToWorld(loc);
		wx = w[0]; wy = w[1]; wz = w[2];
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
		House h = House.Cast(candidate);
		if (!h) return false;

		string hLow = GetHouseClassLower(h);
		if (!TR_SearchNodesDb.HasInteriorForHouse(hLow)) return false;

		vector pp = player.GetPosition();
		vector hp = h.GetPosition();
		float dx = pp[0] - hp[0];
		float dy = pp[1] - hp[1];
		float dz = pp[2] - hp[2];
		float d2 = dx*dx + dy*dy + dz*dz;
		if (d2 > 64.0) return false; // 8 m
		return true;
	}

	static string ComputeFurnitureToken(PlayerBase player, Building b)
	{
		if (!player || !b) return "0|0|0";
		vector ppos = player.GetPosition();
		vector loc  = b.WorldToModel(ppos);
		return QuantizeLocalXYZToken(loc);
	}

	static string ServerComputeFurnitureToken(PlayerBase player, Building b)
	{
		return ComputeFurnitureToken(player, b);
	}

	static string BuildInteriorCooldownKey(House h, string furnitureLocalToken)
	{
		string hLow = GetHouseClassLower(h);
		return TR_SearchNodesDb.BuildInteriorCooldownKey(h, hLow, furnitureLocalToken);
	}
}
