// Debug utilities

class TR_Debug
{
	protected static const string PREFIX = "[Tenacious Rummaging - DEBUGGER] ";
	protected static bool g_Enabled = false;

	static void SetEnabled(bool e) { g_Enabled = e; }
	static bool Enabled()          { return g_Enabled; }

	static void Log(string msg)   { if (g_Enabled) Print(PREFIX + "[INFO] " + msg); }
	static void Warn(string msg)  { if (g_Enabled) Print(PREFIX + "[WARN] " + msg); }
	static void Error(string msg) { Print(PREFIX + "[ERROR] " + msg); }
}
