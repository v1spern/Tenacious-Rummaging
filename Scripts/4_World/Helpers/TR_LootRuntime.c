// Runtime loot helpers

class TR_LootRuntime
{
	static void ApplyOverrides(ItemBase ib, TR_LootEntry e)
	{
		if (!ib || !e) return;

		// Health
		if (e.MinHealth >= 0.0 || e.MaxHealth >= 0.0)
		{
			float hmin = e.MinHealth;
			float hmax = e.MaxHealth;
			if (hmin < 0.0 && hmax >= 0.0) hmin = hmax;
			if (hmax < 0.0 && hmin >= 0.0) hmax = hmin;
			if (hmin < 0.0) hmin = 0.0;
			if (hmax < 0.0) hmax = 0.0;
			if (hmin > 1.0) hmin = 1.0;
			if (hmax > 1.0) hmax = 1.0;
			if (hmin > hmax) { float th = hmin; hmin = hmax; hmax = th; }

			float h = Math.RandomFloat(hmin, hmax);
			ib.SetHealth01("", "", h);
		}

		// Quantity
		bool anyQ = (e.MinQuantity >= 0) || (e.MaxQuantity >= 0);
		if (anyQ)
		{
			if (!ib.HasQuantity())
			{
				TR_Debug.Warn("[LootRuntime] Quantity override ignored for non-quantity item '" + ib.GetType() + "'");
				return;
			}

			int qmin = e.MinQuantity;
			int qmax = e.MaxQuantity;
			if (qmin < 0 && qmax >= 0) qmin = qmax;
			if (qmax < 0 && qmin >= 0) qmax = qmin;
			if (qmin < 1) qmin = 1;
			if (qmax < 1) qmax = 1;
			if (qmin > qmax) { int tq = qmin; qmin = qmax; qmax = tq; }

			int q = Math.RandomIntInclusive(qmin, qmax);
			int maxItem = ib.GetQuantityMax();
			if (maxItem > 0 && q > maxItem) q = maxItem;

			ib.SetQuantity(q);
		}
	}
}