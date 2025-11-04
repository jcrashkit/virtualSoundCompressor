//------------------------------------------------------------------------------------------------
// Virtual Sound Compressor - Main Component
// Author: jcrashkit
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(description: "Main component for Virtual Sound Compressor")]
class VSC_MainComponentClass : ScriptedGameComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		Print("[VSC] Virtual Sound Compressor initialized", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		Print("[VSC] Virtual Sound Compressor cleanup", LogLevel.NORMAL);
	}
}
