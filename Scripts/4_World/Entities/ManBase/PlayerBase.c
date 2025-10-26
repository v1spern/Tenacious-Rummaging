// Playerbase

modded class PlayerBase extends ManBase
{
    void PlayerBase()
    {
        TR_Debug.Log("[PlayerBase] ctor");
    }

    override void EEInit()
    {
        super.EEInit();

        if (GetGame().IsClient() && GetGame().IsMultiplayer())
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Send(this, TR_SearchNodesDb.TRPC_REQUEST_SYNC, true, null);
            TR_Debug.Log("[PlayerBase] Sent TRPC_REQUEST_SYNC");
            ScriptRPC rpc2 = new ScriptRPC();
            rpc2.Send(this, TR_SearchNodesDb.TRPC_REQUEST_PROMPTS, true, null);
            TR_Debug.Log("[PlayerBase] Sent TRPC_REQUEST_PROMPTS");
        }
    }

    override void SetActions(out TInputActionMap InputActionMap)
    {
        super.SetActions(InputActionMap);

        AddAction(ActionSearchNode, InputActionMap);
        AddAction(ActionAddClassSearchNode, InputActionMap);
        AddAction(ActionAddStaticSearchNode, InputActionMap);
        AddAction(ActionAddInteriorPieceSearchNode, InputActionMap);
    }

    override void MessageAction(string text)
    {
        Message(text, "colorGreen");
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type == TR_SearchNodesDb.TRPC_SYNC_SEARCHNODES || rpc_type == TR_SearchNodesDb.TRPC_CLEAR_SEARCHNODES || rpc_type == TR_SearchNodesDb.TRPC_SYNC_PROMPTS)
        {
            TR_SearchNodesDb.OnRpc_Receive(rpc_type, ctx);
            return;
        }

        if (GetGame().IsServer() && rpc_type == TR_SearchNodesDb.TRPC_REQUEST_SYNC)
        {
            TR_SearchNodesDb.SyncAllToClient(this, sender);
            return;
        }

        if (!GetGame().IsServer() && rpc_type == TR_Constants.RPC_ID_TR_AUDIO)
        {
            int   kind = -1;
            vector vpos = GetPosition();

            Param2<int, vector> p2;
            bool ok2 = ctx.Read(p2);
            if (ok2)
            {
                kind = p2.param1;
                vpos = p2.param2;
            }
            else
            {
                Param1<int> p1;
                bool ok1 = ctx.Read(p1);
                if (ok1)
                {
                    kind = p1.param1;
                    vector tmp;
                    bool hasV = ctx.Read(tmp);
                    if (hasV) vpos = tmp;
                }
            }

            TR_Debug.Log("[PlayerBase] OnRPC TR_AUDIO kind=" + kind.ToString() + " pos=" + vpos.ToString());

            if (kind == TR_Constants.TR_AUDIO_KIND_FOUND)
            {
                TR_Audio.PlayRandomLootFound(this);
            }
            else if (kind == TR_Constants.TR_AUDIO_KIND_SEARCH)
            {
                TR_Audio.PlayRandomSearch(this);
            }
            else if (kind == TR_Constants.TR_AUDIO_KIND_ZOMBIE)
            {
                TR_Audio.PlayRandomFallingObjectAt(vpos, this);
            }
            else if (kind == TR_Constants.TR_AUDIO_KIND_SHOCK)
            {
                TR_Audio.PlayRandomShockOnPlayer(this);
                TR_Debug.Log("[PlayerBase] Shock SFX played on player");
            }
            else if (kind == TR_Constants.TR_AUDIO_KIND_KNOCKOUT)
            {
                TR_Audio.PlayRandomKnockOutOnPlayer(this);
                TR_Debug.Log("[PlayerBase] KnockOut SFX played on player");
            }
            else if (kind == TR_Constants.TR_AUDIO_KIND_SIREN)
            {
                TR_Audio.PlaySirenAt(vpos);
                TR_Debug.Log("[PlayerBase] Siren SFX played at world position");
            }
            return;
        }

        super.OnRPC(sender, rpc_type, ctx);
    }
}
