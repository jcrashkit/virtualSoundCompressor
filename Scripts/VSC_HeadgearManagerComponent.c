//------------------------------------------------------------------------------------------------
// Virtual Sound Compressor - Headgear Manager Component
// Automatically attaches hearing protection component to headgear items when equipped
// Optimized for server-side operation with up to 128 concurrent users
// Author: jcrashkit
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(category: "GameScripted/Audio", description: "Automatically attaches VSC hearing protection and BOSSA to headgear items. Optimized for 128 concurrent users.")]
class VSC_HeadgearManagerComponentClass : ScriptedGameComponentClass
{
}

class VSC_HeadgearManagerComponent : ScriptedGameComponent
{
	// Performance optimization: Track which items already have components to avoid duplicate work
	private ref map<string, bool> m_mProcessedItems;
	
	// Performance: Cache component class to avoid repeated lookups
	private VSC_ActiveHearingProtectionComponentClass m_ComponentClass;
	private VSC_BOSSAComponentClass m_BOSSAClass;

	// Consolidated setup: single toggle controls auto-attach of both protection and BOSSA
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Auto-attach VSC components (Protection + BOSSA) to headgear items (server-side).")]
	protected bool m_bAutoAttach;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_mProcessedItems = new map<string, bool>();
		// Component classes will be created when needed
		m_ComponentClass = null;
		m_BOSSAClass = null;
		
		// Start monitoring for headgear equipment changes
		// Use staggered polling to distribute load across frames (performance optimization)
		GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).CallLater(MonitorHeadgearEquipments, 200, true);
		
		Print("[VSC Manager] Headgear manager initialized - monitoring for headgear items", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	// Monitor for headgear being equipped and attach component automatically
	// Optimized: Only runs on server, processes in batches to avoid frame spikes
	//------------------------------------------------------------------------------------------------
	protected void MonitorHeadgearEquipments()
	{
		// Only run on server side
		if (!GetGame().IsServer())
			return;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		// Performance optimization: Process characters in batches to avoid frame drops
		// Check all characters but only process a few per frame
		array<Managed> allCharacters = {};
		array<Class> excludeClasses = {};
		array<Object> objects = {};
		
		// Search for all characters in the world (optimized search)
		world.FindEntitiesAround("0 0 0", 50000.0, excludeClasses, allCharacters, objects);
		
		int processedThisFrame = 0;
		const int MAX_PROCESS_PER_FRAME = 8; // Process 8 characters per frame to avoid lag
		
		foreach (Managed obj : allCharacters)
		{
			IEntity entity = IEntity.Cast(obj);
			if (!entity)
				continue;
			
			ChimeraCharacter character = ChimeraCharacter.Cast(entity);
			if (!character)
				continue;
			
			// Limit processing per frame for performance
			if (processedThisFrame >= MAX_PROCESS_PER_FRAME)
				break;
			
			// Check if character has headgear equipped
			ProcessCharacterHeadgear(character);
			processedThisFrame++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Process a character's headgear and attach component if needed
	//------------------------------------------------------------------------------------------------
	protected void ProcessCharacterHeadgear(ChimeraCharacter character)
	{
		if (!character)
			return;
		
		// Get character's inventory
		InventoryComponent inventory = InventoryComponent.Cast(character.FindComponent(InventoryComponent));
		if (!inventory)
			return;
		
		// Get headgear - try multiple methods to find it
		IEntity headgear = null;
		
		// Try common headgear slots first (InventorySlots.HEADGEAR varies per project)
		headgear = inventory.GetItemInSlot(1);
		if (!headgear)
			headgear = inventory.GetItemInSlot(2);
		
		// Fallback: search inventory for headgear items by name
		if (!headgear)
		{
			headgear = FindHeadgearInInventory(inventory);
		}
		
		if (!headgear)
			return;
		
		// Create unique ID for this item
		string itemID = headgear.GetID().ToString();
		
		// Performance: Skip if already processed
		if (m_mProcessedItems.Contains(itemID) && m_mProcessedItems.Get(itemID))
			return;
		
		if (!m_bAutoAttach)
			return;
		
		// Attach both components to headgear
		bool attachedAny = false;
		attachedAny |= AttachProtection(headgear);
		attachedAny |= AttachBOSSA(headgear);
		
		if (attachedAny)
		{
			m_mProcessedItems.Set(itemID, true);
			Print("[VSC Manager] Attached components to headgear: " + headgear.GetName(), LogLevel.NORMAL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Find headgear item in character inventory
	//------------------------------------------------------------------------------------------------
	protected IEntity FindHeadgearInInventory(InventoryComponent inventory)
	{
		if (!inventory)
			return null;
		
		// Try to find headgear by checking common inventory slots
		// Most Arma Reforger headgear is in slot 1 or 2
		array<int> headgearSlots = {1, 2, 3}; // Common headgear slots
		
		foreach (int slot : headgearSlots)
		{
			IEntity item = inventory.GetItemInSlot(slot);
			if (item)
			{
				// Check if item name suggests it's headgear (heuristic)
				string itemName = item.GetName().ToLower();
				if (itemName.Contains("helmet") || itemName.Contains("cap") || 
				    itemName.Contains("hat") || itemName.Contains("head") ||
				    itemName.Contains("headphone") || itemName.Contains("ear") ||
				    itemName.Contains("headset") || itemName.Contains("comms"))
				{
					return item;
				}
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	// Attach Active Hearing Protection to an item entity
	//------------------------------------------------------------------------------------------------
	protected bool AttachProtection(IEntity item)
	{
		if (!item)
			return false;
		
		VSC_ActiveHearingProtectionComponent existing = VSC_ActiveHearingProtectionComponent.Cast(item.FindComponent(VSC_ActiveHearingProtectionComponent));
		if (existing)
			return true; // Already attached
		
		if (!m_ComponentClass)
		{
			m_ComponentClass = new VSC_ActiveHearingProtectionComponentClass();
		}
		
		ScriptComponent component = ScriptComponent.Cast(item.CreateComponent(m_ComponentClass));
		return component != null;
	}
	
	//------------------------------------------------------------------------------------------------
	// Attach BOSSA component to an item entity
	//------------------------------------------------------------------------------------------------
	protected bool AttachBOSSA(IEntity item)
	{
		if (!item)
			return false;
		
		VSC_BOSSAComponent existing = VSC_BOSSAComponent.Cast(item.FindComponent(VSC_BOSSAComponent));
		if (existing)
			return true; // Already attached
		
		if (!m_BOSSAClass)
		{
			m_BOSSAClass = new VSC_BOSSAComponentClass();
		}
		
		ScriptComponent component = ScriptComponent.Cast(item.CreateComponent(m_BOSSAClass));
		return component != null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_mProcessedItems)
		{
			m_mProcessedItems.Clear();
		}
		
		GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).Remove(MonitorHeadgearEquipments);
		
		super.OnDelete(owner);
	}
}

