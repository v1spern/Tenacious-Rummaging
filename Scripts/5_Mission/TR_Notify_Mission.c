// Mission helper for the notification system

modded class MissionBase
{
    void TR_ShowToast(float lifeTime, string text, string iconPath)
    {
        TR_Debug.Log("[NotifyHUD-Helper] Show Notification: display-duration='" + lifeTime + "', text='" + text + "'");
        NotificationSystem.AddNotificationExtended(lifeTime, "", text, iconPath);
    }
}