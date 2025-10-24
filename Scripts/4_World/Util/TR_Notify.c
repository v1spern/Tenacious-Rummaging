// Notifications utility system

class TR_Notify
{
    static const int RPC_TR_TOAST = 0x0599A110;

    static void Send(PlayerBase player, string text)
    {
        if (!player) return;
        if (text == "") return;

        if (!TR_LootSettingsManager.NotifEnabled())
        {
            player.MessageStatus(text);
            return;
        }

        string iconPath = "TenaciousRummaging/gui/images/TR_Notify_Blank.edds";
        if (TR_LootSettingsManager.EnableToastIcon())
        {
            iconPath = "TenaciousRummaging/gui/images/TR_Notify_Logo.edds";
        }

        float lifeTime = TR_LootSettingsManager.NotifDurationSeconds();

        if (GetGame() && GetGame().IsServer() && GetGame().IsMultiplayer())
        {
            PlayerIdentity id = player.GetIdentity();
            if (id)
            {
                TR_Debug.Log("[NotifyHUD] Toast send RPC: lifeTime='" + lifeTime + "', text='" + text + "'.");
                Param3<float,string,string> p = new Param3<float,string,string>(lifeTime, text, iconPath);
                GetGame().RPCSingleParam(player, RPC_TR_TOAST, p, true, id);
            }
            else
            {
                player.MessageStatus(text);
            }
            return;
        }

        TR_Debug.Log("[NotifyHUD] Toast local: lifeTime='" + lifeTime + "', text='" + text + "'.");
        CallMissionToast(lifeTime, text, iconPath);
    }

    static void CallMissionToast(float lifeTime, string text, string iconPath)
    {
        Mission m = GetGame().GetMission();
        if (!m) return;

        Param3<float,string,string> params = new Param3<float,string,string>(lifeTime, text, iconPath);
        GetGame().GameScript.CallFunctionParams(m, "TR_ShowToast", null, params);
    }
}

modded class PlayerBase
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type == TR_Notify.RPC_TR_TOAST)
        {
            if (GetGame() && GetGame().IsServer() && GetGame().IsMultiplayer()) return;

            Param3<float,string,string> p;
            if (!ctx.Read(p)) return;

            if (!TR_LootSettingsManager.NotifEnabled())
            {
                MessageStatus(p.param2);
                return;
            }

            TR_Debug.Log("[NotifyHUD] Toast receive RPC: lifeTime='" + p.param1 + "', text='" + p.param2 + "'.");
            TR_Notify.CallMissionToast(p.param1, p.param2, p.param3);
        }
    }
}
