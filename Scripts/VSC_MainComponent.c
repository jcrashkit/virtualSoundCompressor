//------------------------------------------------------------------------------------------------
// Virtual Sound Compressor - Main Component
// Main entry point for VSC mod
// Note: VSC_HeadgearManagerComponent should be attached separately to enable automatic headgear detection
// Author: jcrashkit
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(description: "Main component for Virtual Sound Compressor")]
class VSC_MainComponentClass : ScriptedGameComponentClass
{
}

class VSC_MainComponent : ScriptedGameComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		Print("[VSC] Virtual Sound Compressor initialized", LogLevel.NORMAL);
		Print("[VSC] Note: Attach VSC_HeadgearManagerComponent to enable automatic headgear detection", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		Print("[VSC] Virtual Sound Compressor cleanup", LogLevel.NORMAL);
	}
}
