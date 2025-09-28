// Searchable Nodes Database - also known as the script from hell.

class TR_SearchNode
{
    bool   IsClassWide;
    string ClassName;
    string ModelName;
    string Position;
    string LootGroup;
    string Comment;

    void TR_SearchNode()
    {
        IsClassWide = false;
        ClassName   = "";
        ModelName   = "";
        Position    = "0 0 0";
        LootGroup   = "";
        Comment     = "";
    }

    static TR_SearchNode Create(bool isClassWide, string className, string modelName, vector pos, string lootGroup, string comment = "")
    {
        TR_SearchNode node = new TR_SearchNode();
        node.IsClassWide = isClassWide;
        node.ClassName   = className;
        node.ModelName   = modelName;
        node.Position    = pos.ToString(); // "x y z"
        node.LootGroup   = lootGroup;
        node.Comment     = comment;
        return node;
    }

    vector GetPos()
    {
        return TR_SearchNodesDb.ParseVectorFlexible(Position);
    }
}

class TR_SearchNodesDb
{
    // RPC types
    static const int TRPC_SYNC_SEARCHNODES  = 98765;
    static const int TRPC_CLEAR_SEARCHNODES = 98766;
    static const int TRPC_REQUEST_SYNC      = 98767;

    // Tunables
    static const float TR_STATIC_MATCH_RADIUS = 1.0;
    static const int   TR_COOLDOWN_GRID_CM    = 50;

    // Data
    static ref array<ref TR_SearchNode> s_SearchNodes;
    static ref map<string, ref array<ref TR_SearchNode> > s_SearchNodesCache;

    // InteriorClass index: houseClass(lower) -> TR_SearchNode (ModelName="__interior__")
    static ref map<string, ref TR_SearchNode> s_InteriorByHouseClass;

    // Per-furniture overrides:
    // key = houseClass(lower) + "#" + token("xi|yi|zi") -> TR_SearchNode (ModelName="__interior_tok__")
    static ref map<string, ref TR_SearchNode> s_InteriorPieceByHouseAndTok;

    // Local model key for per-furniture overrides
    static const string MODELKEY_INTERIOR_TOKEN = "__interior_tok__";

    // Utilities

    static vector ParseVectorFlexible(string s)
    {
        if (s == "") return "0 0 0".ToVector();
        string t = s;
        t.Replace("<", "");
        t.Replace(">", "");
        t.Replace(",", " ");
        return t.ToVector();
    }

    static string NormalizeModelKey(string name)
    {
        if (name == "") return "";
        string key = name;
        key.ToLower();
        key.Replace("\\", "/");

        TStringArray pathParts = new TStringArray();
        key.Split("/", pathParts);
        if (pathParts && pathParts.Count() > 0)
            key = pathParts.Get(pathParts.Count() - 1);

        TStringArray extParts = new TStringArray();
        key.Split(".", extParts);
        if (extParts && extParts.Count() > 0)
            key = extParts.Get(0);

        return key;
    }

    // Admin/code path to add/update a per-furniture override. furnitureToken: "xi|yi|zi"
    static void AddInteriorPieceNode(string houseClass, string furnitureToken, string lootGroup, string nodeLabelStr)
    {
        if (!s_SearchNodes) Init();
    
        string hLow = ToLowerSafe(houseClass);
    
        // Parse "xi|yi|zi"
        int xi = 0;
        int yi = 0;
        int zi = 0;
        TStringArray parts = new TStringArray();
        furnitureToken.Split("|", parts);
        if (parts && parts.Count() == 3)
        {
            xi = parts.Get(0).ToInt();
            yi = parts.Get(1).ToInt();
            zi = parts.Get(2).ToInt();
        }
    
        if (!s_InteriorPieceByHouseAndTok)
            s_InteriorPieceByHouseAndTok = new map<string, ref TR_SearchNode>();
    
        string hTokKey = hLow + "#" + furnitureToken;
    
        // Update if the piece already exists
        if (s_InteriorPieceByHouseAndTok.Contains(hTokKey))
        {
            TR_SearchNode exist = s_InteriorPieceByHouseAndTok.Get(hTokKey);
            if (exist)
            {
                exist.LootGroup = lootGroup;
                exist.Comment   = nodeLabelStr;
                Save();
                if (GetGame().IsServer()) SendNodeToClient(exist, null, null);
            }
            return;
        }
    
        // Create new row (no vector locals)
        TR_SearchNode row = new TR_SearchNode();
        row.IsClassWide = true;
        row.ClassName   = hLow;
        row.ModelName   = MODELKEY_INTERIOR_TOKEN;
        row.Position    = xi.ToString() + " " + yi.ToString() + " " + zi.ToString();
        row.LootGroup   = lootGroup;
        row.Comment     = nodeLabelStr;
    
        s_SearchNodes.Insert(row);
        Save();
    
        s_InteriorPieceByHouseAndTok.Set(hTokKey, row);
        if (GetGame().IsServer()) SendNodeToClient(row, null, null);
    }
    
    static string WorldNameLower()
    {
        string w = GetGame().GetWorldName();
        w.ToLower();
        return w;
    }

    static string ToLowerSafe(string s)
    {
        string t = s;
        t.ToLower();
        return t;
    }

    static string QuantizeXZToken(vector pos)
    {
        int div = TR_COOLDOWN_GRID_CM;
        if (div < 1) div = 1;
        int mul = 100 / div;
        if (mul < 1) mul = 1;

        float xf = pos[0] * mul;
        float zf = pos[2] * mul;

        int xi = (int)(xf + 0.5);
        int zi = (int)(zf + 0.5);

        return xi.ToString() + "|" + zi.ToString();
    }

    // O:<world_lower>:<model_lower>:<xi|zi>
    static string BuildCanonicalPerObjectKey(Object obj, string modelKey)
    {
        string world = WorldNameLower();
        vector pos = obj.GetPosition();
        string tok = QuantizeXZToken(pos);
        string key = "O:" + world + ":" + modelKey + ":" + tok;
        return key;
    }

    static string BuildCanonicalKeyForNode(TR_SearchNode node)
    {
        if (!node) return "";
        string world = WorldNameLower();
        string modelKey = NormalizeModelKey(node.ModelName);
        vector pos = ParseVectorFlexible(node.Position);
        string tok = QuantizeXZToken(pos);
        string key = "O:" + world + ":" + modelKey + ":" + tok;
        return key;
    }

    // Interior cooldown key
    // I:<world>:<houseClassLower>:<houseXi|Zi>:<furnXi|Zi>
    static string BuildInteriorCooldownKey(House h, string houseClassLower, string furnitureLocalToken)
    {
        if (!h) return "";
        string world = WorldNameLower();
        vector hp = h.GetPosition();
        string houseTok = QuantizeXZToken(hp);
        return "I:" + world + ":" + houseClassLower + ":" + houseTok + ":" + furnitureLocalToken;
    }

    // Lifecycle

    static void Init()
    {
        if (!s_SearchNodes)
            Load();

        if (GetGame().IsServer() && s_SearchNodes)
        {
            for (int i = 0; i < s_SearchNodes.Count(); i++)
                SendNodeToClient(s_SearchNodes[i], null, null);
            TR_Debug.Log("[NodesDB] Sent " + s_SearchNodes.Count().ToString() + " nodes.");
        }
    }

    protected static void RebuildInteriorIndex()
    {
        s_InteriorByHouseClass       = new map<string, ref TR_SearchNode>();
        s_InteriorPieceByHouseAndTok = new map<string, ref TR_SearchNode>();
        if (!s_SearchNodes) return;

        for (int i = 0; i < s_SearchNodes.Count(); i++)
        {
            TR_SearchNode n = s_SearchNodes.Get(i);
            if (!n) continue;
            if (!n.IsClassWide) continue;

            string mkey = NormalizeModelKey(n.ModelName);

            if (mkey == TR_Constants.MODELKEY_INTERIOR)
            {
                string hLow = ToLowerSafe(n.ClassName);
                s_InteriorByHouseClass.Set(hLow, n);
            }
            else if (mkey == MODELKEY_INTERIOR_TOKEN)
            {
                string t = n.Position;
                t.Replace("<", "");
                t.Replace(">", "");
                t.Replace(",", " ");
                TStringArray parts = new TStringArray();
                t.Split(" ", parts);

                int xi = 0; int yi = 0; int zi = 0;
                if (parts && parts.Count() >= 3)
                {
                    xi = parts.Get(0).ToInt();
                    yi = parts.Get(1).ToInt();
                    zi = parts.Get(2).ToInt();
                }

                string tok   = xi.ToString() + "|" + yi.ToString() + "|" + zi.ToString();
                string hTokKey = ToLowerSafe(n.ClassName) + "#" + tok;

                s_InteriorPieceByHouseAndTok.Set(hTokKey, n);
            }
        }
    }

    static void Save()
    {
        if (!s_SearchNodes) s_SearchNodes = new array<ref TR_SearchNode>;
        TR_Constants.EnsureProfile();
        string path = TR_Constants.Path(TR_Constants.FILE_SEARCH_NODES);
        JsonFileLoader<array<ref TR_SearchNode>>.JsonSaveFile(path, s_SearchNodes);
    }

    static void Load()
    {
        s_SearchNodes = new array<ref TR_SearchNode>;
        TR_Constants.EnsureProfile();
        string path = TR_Constants.Path(TR_Constants.FILE_SEARCH_NODES);

        if (FileExist(path))
        {
            JsonFileLoader<array<ref TR_SearchNode>>.JsonLoadFile(path, s_SearchNodes);

            if (!s_SearchNodesCache) s_SearchNodesCache = new map<string, ref array<ref TR_SearchNode> >();
            s_SearchNodesCache.Clear();

            for (int i = 0; i < s_SearchNodes.Count(); i++)
            {
                TR_SearchNode node = s_SearchNodes.Get(i);
                if (!node) continue;

                string key = NormalizeModelKey(node.ModelName);
                if (!s_SearchNodesCache.Contains(key))
                    s_SearchNodesCache.Set(key, new array<ref TR_SearchNode>());
                s_SearchNodesCache.Get(key).Insert(node);
            }

            RebuildInteriorIndex();
        }
        else
        {
            Save();
            RebuildInteriorIndex();
        }
    }

    // Server -> Client sync

    static void SendNodeToClient(TR_SearchNode node, Object target = null, PlayerIdentity id = null)
    {
        if (!node) return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(node.IsClassWide);
        rpc.Write(node.ClassName);
        rpc.Write(node.ModelName);
        rpc.Write(node.Position);
        rpc.Write(node.LootGroup);
        rpc.Write(node.Comment);
        rpc.Send(target, TRPC_SYNC_SEARCHNODES, true, id);
    }

    static void SendClearToClients(Object target = null, PlayerIdentity id = null)
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send(target, TRPC_CLEAR_SEARCHNODES, true, id);
    }

    static void OnRpc_Receive(int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type == TRPC_CLEAR_SEARCHNODES)
        {
            s_SearchNodesCache = new map<string, ref array<ref TR_SearchNode> >();
            RebuildInteriorIndex();
            return;
        }

        if (rpc_type == TRPC_SYNC_SEARCHNODES)
        {
            bool   isClassWide;
            string className;
            string modelName;
            string posStr;
            string lootGroup;
            string comment;

            if (!ctx.Read(isClassWide)) return;
            if (!ctx.Read(className))   return;
            if (!ctx.Read(modelName))   return;
            if (!ctx.Read(posStr))      return;
            if (!ctx.Read(lootGroup))   return;
            if (!ctx.Read(comment))     return;

            TR_SearchNode node = new TR_SearchNode();
            node.IsClassWide = isClassWide;
            node.ClassName   = className;
            node.ModelName   = modelName;
            node.Position    = posStr;
            node.LootGroup   = lootGroup;
            node.Comment     = comment;

            if (!s_SearchNodesCache) s_SearchNodesCache = new map<string, ref array<ref TR_SearchNode> >();
            string modelNorm = NormalizeModelKey(node.ModelName);

            if (!s_SearchNodesCache.Contains(modelNorm))
                s_SearchNodesCache.Set(modelNorm, new array<ref TR_SearchNode>());
            s_SearchNodesCache.Get(modelNorm).Insert(node);

            // keep interior indices warm on clients
            string mkey2 = NormalizeModelKey(modelName);

            if (mkey2 == TR_Constants.MODELKEY_INTERIOR && isClassWide && className != "")
            {
                string hLow = ToLowerSafe(className);
                if (!s_InteriorByHouseClass) s_InteriorByHouseClass = new map<string, ref TR_SearchNode>();
                s_InteriorByHouseClass.Set(hLow, node);
            }

            if (mkey2 == MODELKEY_INTERIOR_TOKEN && isClassWide && className != "" && posStr != "")
            {
                string t = posStr;
                t.Replace("<", "");
                t.Replace(">", "");
                t.Replace(",", " ");
                TStringArray parts = new TStringArray();
                t.Split(" ", parts);

                int xi = 0; int yi = 0; int zi = 0;
                if (parts && parts.Count() >= 3)
                {
                    xi = parts.Get(0).ToInt();
                    yi = parts.Get(1).ToInt();
                    zi = parts.Get(2).ToInt();
                }

                string tok   = xi.ToString() + "|" + yi.ToString() + "|" + zi.ToString();
                string hTokK = ToLowerSafe(className) + "#" + tok;

                if (!s_InteriorPieceByHouseAndTok) s_InteriorPieceByHouseAndTok = new map<string, ref TR_SearchNode>();
                s_InteriorPieceByHouseAndTok.Set(hTokK, node);
            }
        }
    }

    // Matching

    static TR_SearchNode MatchEx(Object obj, bool verbose)
    {
        if (!obj) return null;

        string rawModel = TR_Util.GetObjectModel(obj);
        string modelKey = NormalizeModelKey(rawModel);
        vector pos = obj.GetPosition();

        array<ref TR_SearchNode> candidates;
        bool usingClientCache = false;

        if (s_SearchNodesCache && s_SearchNodesCache.Count() > 0)
        {
            usingClientCache = true;
            if (!s_SearchNodesCache.Find(modelKey, candidates))
                return null;
        }
        else
        {
            if (!s_SearchNodes) Init();

            candidates = new array<ref TR_SearchNode>();
            for (int i = 0; i < s_SearchNodes.Count(); i++)
            {
                TR_SearchNode n = s_SearchNodes.Get(i);
                if (!n) continue;
                string nKey = NormalizeModelKey(n.ModelName);
                if (nKey == modelKey)
                    candidates.Insert(n);
            }

            if (candidates.Count() == 0) return null;
        }

        string objCanon = BuildCanonicalPerObjectKey(obj, modelKey);

        // exact canonical match
        for (int s = 0; s < candidates.Count(); s++)
        {
            TR_SearchNode sn = candidates.Get(s);
            if (!sn || sn.IsClassWide) continue;

            string nodeCanon = BuildCanonicalKeyForNode(sn);
            if (nodeCanon == objCanon) return sn;
        }

        // fallback radius
        for (int s2 = 0; s2 < candidates.Count(); s2++)
        {
            TR_SearchNode sn2 = candidates.Get(s2);
            if (!sn2 || sn2.IsClassWide) continue;

            float dist = vector.Distance(sn2.GetPos(), pos);
            if (dist <= TR_STATIC_MATCH_RADIUS) return sn2;
        }

        // class/model-wide
        for (int c = 0; c < candidates.Count(); c++)
        {
            TR_SearchNode cw = candidates.Get(c);
            if (!cw) continue;
            if (cw.IsClassWide) return cw;
        }

        return null;
    }

    static TR_SearchNode Match(Object obj) { return MatchEx(obj, false); }

    static ref array<ref TR_SearchNode> GetAll() { if (!s_SearchNodes) Init(); return s_SearchNodes; }

    static bool ExistsClassWideNode(string className, string modelName)
    {
        if (!s_SearchNodes) Init();
        if (!s_SearchNodes) return false;

        string normModel = NormalizeModelKey(modelName);

        for (int i = 0; i < s_SearchNodes.Count(); i++)
        {
            TR_SearchNode ex = s_SearchNodes.Get(i);
            if (!ex) continue;
            if (!ex.IsClassWide) continue;
            if (ex.ClassName != className) continue;

            string exNorm = NormalizeModelKey(ex.ModelName);
            if (exNorm == normModel) return true;
        }
        return false;
    }

    static bool HasInteriorForHouse(string houseClassLower)
    {
        string hLow = ToLowerSafe(houseClassLower);

        if (!s_InteriorByHouseClass)
        {
            if (GetGame().IsServer()) RebuildInteriorIndex();
            else s_InteriorByHouseClass = new map<string, ref TR_SearchNode>();
        }
        return s_InteriorByHouseClass.Contains(hLow);
    }

    static TR_SearchNode GetInteriorDefForHouse(string houseClassLower)
    {
        string hLow = ToLowerSafe(houseClassLower);

        if (!s_InteriorByHouseClass)
        {
            if (GetGame().IsServer()) RebuildInteriorIndex();
            else s_InteriorByHouseClass = new map<string, ref TR_SearchNode>();
        }

        if (!s_InteriorByHouseClass.Contains(hLow)) return null;
        return s_InteriorByHouseClass.Get(hLow);
    }

    // Interior range-aware resolution

    // Gating check for prompt
    static bool HasInteriorPieceNear(House h, string furnitureToken, vector playerPos, float rangeM)
    {
        string dummyGroup;
        string dummyTok;
        return ResolveInteriorMatchNear(h, furnitureToken, playerPos, rangeM, dummyGroup, dummyTok);
    }
    
    // Returns loot group + matched token if a stored token is within <= rangeM of the player. Hopefully.
    static bool ResolveInteriorMatchNear(
        House h, string furnitureToken, vector playerPos, float rangeM,
        out string outGroup, out string outMatchedToken)
    {
        outGroup = "";
        outMatchedToken = "";
        if (!h) return false;

        string hLow = ToLowerSafe(h.GetType());

        if (!s_InteriorPieceByHouseAndTok)
        {
            if (GetGame().IsServer()) RebuildInteriorIndex();
            else s_InteriorPieceByHouseAndTok = new map<string, ref TR_SearchNode>();
        }
        if (!s_InteriorPieceByHouseAndTok || s_InteriorPieceByHouseAndTok.Count() == 0) return false;

        // Parse requested token "xi|yi|zi"
        int xi = 0;
        int yi = 0;
        int zi = 0;
        TStringArray parts = new TStringArray();
        furnitureToken.Split("|", parts);
        if (parts && parts.Count() >= 3)
        {
            xi = parts.Get(0).ToInt();
            yi = parts.Get(1).ToInt();
            zi = parts.Get(2).ToInt();
        }

        float r2 = rangeM * rangeM;

        // Exact first
        if (TryResolveTokenNear(h, hLow, xi, yi, zi, playerPos, r2, outGroup, outMatchedToken)) return true;

        // Immediate neighbors only (+/-1 on each axis). This handles rounding flips without opening the whole room.
        if (TryResolveTokenNear(h, hLow, xi + 1, yi,     zi,     playerPos, r2, outGroup, outMatchedToken)) return true;
        if (TryResolveTokenNear(h, hLow, xi - 1, yi,     zi,     playerPos, r2, outGroup, outMatchedToken)) return true;
        if (TryResolveTokenNear(h, hLow, xi,     yi + 1, zi,     playerPos, r2, outGroup, outMatchedToken)) return true;
        if (TryResolveTokenNear(h, hLow, xi,     yi - 1, zi,     playerPos, r2, outGroup, outMatchedToken)) return true;
        if (TryResolveTokenNear(h, hLow, xi,     yi,     zi + 1, playerPos, r2, outGroup, outMatchedToken)) return true;
        if (TryResolveTokenNear(h, hLow, xi,     yi,     zi - 1, playerPos, r2, outGroup, outMatchedToken)) return true;

        return false;
    }

    protected static bool TryResolveTokenNear(
        House h, string hLow, int xi, int yi, int zi,
        vector playerPos, float range2,
        out string outGroup, out string outMatchedToken)
    {
        outGroup = "";
        outMatchedToken = "";

        string tok = xi.ToString() + "|" + yi.ToString() + "|" + zi.ToString();
        string key = hLow + "#" + tok;

        if (!s_InteriorPieceByHouseAndTok.Contains(key)) return false;

        // Token to world position, then test distance to player
        float wx, wy, wz;
        TR_Interior.TokenToWorldXYZ(h, xi, yi, zi, wx, wy, wz);

        float dx = playerPos[0] - wx;
        float dy = playerPos[1] - wy;
        float dz = playerPos[2] - wz;
        float d2 = dx*dx + dy*dy + dz*dz;
        if (d2 > range2) return false;

        TR_SearchNode n = s_InteriorPieceByHouseAndTok.Get(key);
        if (!n) return false;

        outGroup = n.LootGroup;
        outMatchedToken = tok;
        return true;
    }

    // Convenience for building the per-object cooldown key
    static string GetCooldownKeyFor(Object obj, TR_SearchNode node)
    {
        if (!obj) return "";
        string raw = TR_Util.GetObjectModel(obj);
        string modelKey = NormalizeModelKey(raw);
        return BuildCanonicalPerObjectKey(obj, modelKey);
    }

    // Mutators
    static void AddStaticNode(string className, string modelName, vector pos)
    {
        if (!s_SearchNodes) Init();

        string normModel = NormalizeModelKey(modelName);
        string posStr = pos.ToString();
        string newCanon = "O:" + WorldNameLower() + ":" + normModel + ":" + QuantizeXZToken(pos);

        for (int i = 0; i < s_SearchNodes.Count(); i++)
        {
            TR_SearchNode ex = s_SearchNodes.Get(i);
            if (!ex) continue;
            if (ex.IsClassWide) continue;

            string exCanon = BuildCanonicalKeyForNode(ex);
            if (exCanon == newCanon)
            {
                TR_Debug.Log("[NodesDB] AddStaticNode dedup via canonical key: model=" + modelName + " pos=" + posStr);
                return;
            }
        }

        TR_SearchNode node = TR_SearchNode.Create(false, className, modelName, pos, TR_LootSettingsManager.GetDefaultGroup());
        s_SearchNodes.Insert(node);
        Save();

        if (GetGame().IsServer())
            SendNodeToClient(node, null, null);
    }

    static void AddClassWideNode(string className, string modelName)
    {
        if (!s_SearchNodes) Init();

        string normModel = NormalizeModelKey(modelName);
        for (int i = 0; i < s_SearchNodes.Count(); i++)
        {
            TR_SearchNode ex = s_SearchNodes.Get(i);
            if (!ex) continue;
            if (!ex.IsClassWide) continue;
            if (ex.ClassName != className) continue;

            string exNorm = NormalizeModelKey(ex.ModelName);
            if (exNorm == normModel)
            {
                TR_Debug.Log("[NodesDB] AddClassWideNode dedup: " + className + " / " + modelName + " already exists.");
                return;
            }
        }

        vector zero = "0 0 0".ToVector();
        TR_SearchNode node2 = TR_SearchNode.Create(true, className, modelName, zero, TR_LootSettingsManager.GetDefaultGroup());
        s_SearchNodes.Insert(node2);
        Save();

        if (GetGame().IsServer())
            SendNodeToClient(node2, null, null);
    }

    static void AddInteriorClassNode(string houseClass, string lootGroup, string label)
    {
        if (!s_SearchNodes) Init();

        string hLow = ToLowerSafe(houseClass);

        if (HasInteriorForHouse(hLow))
        {
            TR_Debug.Log("[NodesDB] AddInteriorClassNode dedup: " + hLow + " already interior-enabled.");
            return;
        }

        vector zero = "0 0 0".ToVector();
        TR_SearchNode row = TR_SearchNode.Create(true, hLow, TR_Constants.MODELKEY_INTERIOR, zero, lootGroup, label);
        s_SearchNodes.Insert(row);
        Save();

        if (!s_InteriorByHouseClass) s_InteriorByHouseClass = new map<string, ref TR_SearchNode>();
        s_InteriorByHouseClass.Set(hLow, row);

        if (GetGame().IsServer())
            SendNodeToClient(row, null, null);

        TR_Debug.Log("[NodesDB] AddInteriorClassNode OK: house=" + hLow + " group=" + lootGroup + " label=" + label);
    }

    static string GetKey(TR_SearchNode node)
    {
        if (node == null) return "";
        if (node.IsClassWide)
        {
            if (node.ClassName != "")
                return "C:" + node.ClassName + ":" + node.ModelName;
            return "M:" + node.ModelName;
        }
        else
        {
            return "S:" + node.Position;
        }
    }

    static int Count()
    {
        if (!s_SearchNodes) Init();
        return s_SearchNodes.Count();
    }

    static void SyncAllToClient(PlayerBase player, PlayerIdentity id)
    {
        if (!GetGame().IsServer() || !player || !id) return;
        if (!s_SearchNodes) Init();

        for (int i = 0; i < s_SearchNodes.Count(); i++)
            SendNodeToClient(s_SearchNodes[i], player, id);
    }
}