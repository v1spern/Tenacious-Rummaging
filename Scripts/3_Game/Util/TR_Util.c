// Utils for misc.

class TR_Util
{

    static string GetObjectModel(Object obj)
    {
        if (!obj) return "";

        string modelName = obj.GetModelName();
        if (modelName != "UNKNOWN_P3D_FILE") {
            modelName.ToLower();
            return modelName;
        }

        string dbgName = obj.GetDebugNameNative();
        TStringArray parts = {};
        dbgName.Split(":", parts);
        if (parts.Count() > 1) {
            string trimmed = parts[1].Trim();
            trimmed.ToLower();
            if (trimmed.Length() > 4)
                return trimmed.Substring(0, trimmed.Length() - 4);
            return trimmed;
        }

        return string.Empty;
    }

	static string GenerateGUID()
	{
		ref TIntArray a = new TIntArray;
		for (int i = 0; i < 4; i++)
			a.Insert(Math.RandomInt(0, 0x7FFFFFFF));

		return string.Format("%1-%2-%3-%4", a[0], a[1], a[2], a[3]);
	}

	static string FormatHMS(int secs)
	{
		if (secs < 0) secs = 0;
		int h = secs / 3600;
		int m = (secs % 3600) / 60;
		int s = secs % 60;

		if (h > 0)  return string.Format("%1h %2m %3s", h, m, s);
		if (m > 0)  return string.Format("%1m %2s", m, s);
		return string.Format("%1s", s);
	}

	static string VecKey(vector p, int decimals = 2)
	{
		float factor = 1.0;
		for (int i = 0; i < decimals; i++) factor *= 10.0;

		float x = Math.Round(p[0] * factor) / factor;
		float y = Math.Round(p[1] * factor) / factor;
		float z = Math.Round(p[2] * factor) / factor;

		return string.Format("%1,%2,%3", x, y, z);
	}

	static vector KeyToVec(string key)
	{
		string spaced = key;
		spaced.Replace(",", " ");
		return spaced.ToVector();
	}
}
