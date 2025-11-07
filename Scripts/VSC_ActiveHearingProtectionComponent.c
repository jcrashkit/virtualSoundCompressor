//------------------------------------------------------------------------------------------------
// Virtual Sound Compressor - Active Hearing Protection Component
// Simulates active hearing protection. Boosts quiet sounds and dampens loud ones.
// Author: jcrashkit
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(category: "GameScripted/Audio", description: "Simulates active hearing protection. Boosts quiet sounds and dampens loud ones.")]
class VSC_ActiveHearingProtectionComponentClass : ScriptComponentClass
{
}

class VSC_ActiveHearingProtectionComponent : ScriptComponent
{
	[Attribute(defvalue: "1.75", uiwidget: UIWidgets.Slider, desc: "Auditory range multiplier for quiet sounds.", params: "1.0 5.0 0.1")]
	protected float m_fBoostMultiplier;

	[Attribute(defvalue: "0.25", uiwidget: UIWidgets.Slider, desc: "Auditory range multiplier when dampening loud sounds.", params: "0.1 1.0 0.05")]
	protected float m_fDampenMultiplier;

	[Attribute(defvalue: "25", uiwidget: UIWidgets.EditBox, desc: "The maximum distance (meters) from an explosion to trigger the dampening effect.")]
	protected float m_fDampenTriggerRange;

	[Attribute(defvalue: "400", uiwidget: UIWidgets.EditBox, desc: "How long the dampening effect lasts in milliseconds (ms).")]
	protected int m_iDampenDurationMs;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable dampening for weapon sounds (gunshots).")]
	protected bool m_bDetectWeaponSounds;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.EditBox, desc: "The maximum distance (meters) from weapon fire to trigger dampening.")]
	protected float m_fWeaponSoundTriggerRange;

	[Attribute(defvalue: "200", uiwidget: UIWidgets.EditBox, desc: "How long weapon sound dampening lasts in milliseconds (ms).")]
	protected int m_iWeaponSoundDurationMs;

	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Minimum time between dampening triggers (seconds) to prevent rapid toggling.", params: "0.1 2.0 0.1")]
	protected float m_fDampeningCooldown;

	// --- Private Member Variables ---
	private ChimeraCharacter m_PlayerCharacter;
	private PerceptionComponent m_PlayerPerception;
	private float m_fOriginalAuditoryRange;
	private bool m_bIsActive = false;
	private bool m_bIsDampened = false;
	private float m_fLastDampeningTime = 0.0;
	private ref map<string, float> m_mRecentWeaponFire; // Track recent weapon fire events

	//------------------------------------------------------------------------------------------------
	// Called when the component is attached and initialized (e.g., when equipped)
	//------------------------------------------------------------------------------------------------
	override void EOnPostInit(IEntity owner)
	{
		super.EOnPostInit(owner);
		
		// A small delay ensures the player character and its components are fully ready
		GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).CallLater(InitializeProtection, 100, false);
	}

	//------------------------------------------------------------------------------------------------
	// Initialize the hearing protection system
	//------------------------------------------------------------------------------------------------
	protected void InitializeProtection()
	{
		// Get the player character this item is attached to
		// For equipment items, the owner is the item itself, parent should be the character
		IEntity parent = GetOwner().GetParent();
		if (!parent)
			return;
			
		m_PlayerCharacter = ChimeraCharacter.Cast(parent);
		if (!m_PlayerCharacter) 
		{
			// Last attempt: check if owner itself is the character
			m_PlayerCharacter = ChimeraCharacter.Cast(GetOwner());
			if (!m_PlayerCharacter)
				return;
		}

		// Only run this logic for the local player
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
			
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (controlledEntity != m_PlayerCharacter)
			return;
		
		m_PlayerPerception = PerceptionComponent.Cast(m_PlayerCharacter.FindComponent(PerceptionComponent));
		if (!m_PlayerPerception)
			return;
		
		// Store original hearing range for clean restoration
		m_fOriginalAuditoryRange = m_PlayerPerception.GetAuditoryRange();

		// Initialize weapon fire tracking
		m_mRecentWeaponFire = new map<string, float>();

		// Apply the initial boost
		m_PlayerPerception.SetAuditoryRange(m_fOriginalAuditoryRange * m_fBoostMultiplier);
		m_bIsActive = true;

		// Subscribe to the global explosion event
		// Performance: Event-driven, no polling overhead - scales to 128+ users
		BaseWorld world = GetGame().GetWorld();
		if (world)
		{
			world.GetOnExplosion().Insert(this.OnExplosion);
		}
		
		// Start monitoring for weapon sounds if enabled
		// Using 50ms polling for lower latency (was 100ms)
		if (m_bDetectWeaponSounds)
		{
			GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).CallLater(MonitorWeaponSounds, 50, true);
			// Clean up old weapon fire tracking entries periodically
			GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).CallLater(CleanupWeaponFireTracking, 1000, true);
		}
		
		Print("[VSC] Active Hearing Protection Activated. Boost Applied.", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// This method is called by the game engine whenever ANY explosion happens in the world
	//------------------------------------------------------------------------------------------------
	protected void OnExplosion(IEntity explosionEntity, IEntity source, vector position, float rawDamage, float range, EExplosionType type)
	{
		// If the protection isn't active, or we can't find the player, do nothing.
		if (!m_bIsActive || !m_PlayerCharacter || !m_PlayerPerception)
			return;

		// Check cooldown to prevent rapid toggling
		float currentTime = GetGame().GetWorld().GetWorldTime();
		if (currentTime - m_fLastDampeningTime < m_fDampeningCooldown)
			return;

		// Calculate distance from the explosion to the player
		float distance = vector.Distance(m_PlayerCharacter.GetOrigin(), position);
		
		// If the explosion is within our trigger range, apply the dampening effect
		if (distance <= m_fDampenTriggerRange)
		{
			ApplyDampening(m_iDampenDurationMs);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Monitor for weapon sounds (gunshots) in the vicinity
	// Detects nearby characters firing weapons by checking weapon state
	//------------------------------------------------------------------------------------------------
	protected void MonitorWeaponSounds()
	{
		if (!m_bIsActive || !m_bDetectWeaponSounds || !m_PlayerCharacter || !m_PlayerPerception)
			return;

		// Check cooldown to prevent rapid toggling
		float currentTime = GetGame().GetWorld().GetWorldTime();
		if (currentTime - m_fLastDampeningTime < m_fDampeningCooldown)
			return;

		vector playerPos = m_PlayerCharacter.GetOrigin();
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		// Performance optimization for 128 users: Only search within trigger range
		// Limit search radius to weapon sound trigger range to avoid checking distant players
		array<Managed> found = {};
		array<Class> excludeClasses = {};
		array<Object> objects = {};
		
		// Optimized: Use smaller search radius to reduce entity count
		float searchRange = m_fWeaponSoundTriggerRange;
		world.FindEntitiesAround(playerPos, searchRange, excludeClasses, found, objects);
		
		// Performance: Limit maximum entities checked per frame (for 128 player scenarios)
		int maxChecks = 16; // Only check up to 16 nearby entities per frame
		int checked = 0;
		
		foreach (Managed obj : found)
		{
			// Performance optimization: Limit checks per frame
			if (checked >= maxChecks)
				break;
			
			IEntity entity = IEntity.Cast(obj);
			if (!entity || entity == m_PlayerCharacter)
				continue;

			// Check if entity is a character with a weapon
			ChimeraCharacter character = ChimeraCharacter.Cast(entity);
			if (!character)
				continue;
			
			checked++;

			// Get the character's weapon manager
			WeaponManagerComponent weaponManager = WeaponManagerComponent.Cast(character.FindComponent(WeaponManagerComponent));
			if (!weaponManager)
				continue;

			// Get the currently equipped weapon
			BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
			if (!weapon)
				continue;

			// Create unique key for this weapon entity
			string weaponKey = weapon.GetOwner().GetID().ToString();
			float currentTime = GetGame().GetWorld().GetWorldTime();
			
			// Check if this weapon has fired recently (within last 0.3 seconds for faster detection)
			if (m_mRecentWeaponFire.Contains(weaponKey))
			{
				float fireTime = m_mRecentWeaponFire.Get(weaponKey);
				if (currentTime - fireTime < 0.3)
				{
					// Weapon fired recently, apply dampening immediately
					ApplyDampening(m_iWeaponSoundDurationMs);
					break;
				}
			}
			
			// Optimized: Use lightweight projectile detection for immediate response
			// Check for projectiles in a small radius around weapon position (faster detection)
			IEntity weaponEntity = weapon.GetOwner();
			if (weaponEntity)
			{
				vector weaponPos = weaponEntity.GetOrigin();
				float distToPlayer = vector.Distance(playerPos, weaponPos);
				
				// Only check if weapon is within trigger range
				if (distToPlayer < m_fWeaponSoundTriggerRange)
				{
					// Check for projectiles in small radius (optimized search - 2m radius)
					array<Managed> projectiles = {};
					array<Class> excludeClasses2 = {};
					array<Object> objects2 = {};
					world.FindEntitiesAround(weaponPos, 2.0, excludeClasses2, projectiles, objects2);
					
					foreach (Managed proj : projectiles)
					{
						IEntity projectile = IEntity.Cast(proj);
						if (projectile)
						{
							// Found projectile near weapon - mark as recently fired and trigger immediately
							m_mRecentWeaponFire.Set(weaponKey, currentTime);
							ApplyDampening(m_iWeaponSoundDurationMs);
							break;
						}
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Apply audio dampening effect
	//------------------------------------------------------------------------------------------------
	protected void ApplyDampening(int durationMs = -1)
	{
		if (!m_PlayerPerception) 
			return;

		// Use provided duration or default
		if (durationMs < 0)
			durationMs = m_iDampenDurationMs;

		m_bIsDampened = true;
		m_fLastDampeningTime = GetGame().GetWorld().GetWorldTime();
		m_PlayerPerception.SetAuditoryRange(m_fOriginalAuditoryRange * m_fDampenMultiplier);
		Print("[VSC] LOUD NOISE DETECTED! Dampening audio.", LogLevel.WARNING);

		// Schedule the effect to be removed after the specified duration
		GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).CallLater(RestoreBoost, durationMs);
	}

	//------------------------------------------------------------------------------------------------
	// Clean up old weapon fire tracking entries
	//------------------------------------------------------------------------------------------------
	protected void CleanupWeaponFireTracking()
	{
		if (!m_mRecentWeaponFire || !m_bIsActive)
			return;
			
		float currentTime = GetGame().GetWorld().GetWorldTime();
		array<string> keysToRemove = {};
		
		// Remove entries older than 2 seconds
		for (int i = 0; i < m_mRecentWeaponFire.Count(); i++)
		{
			string key = m_mRecentWeaponFire.GetKey(i);
			float fireTime = m_mRecentWeaponFire.Get(key);
			if (currentTime - fireTime > 2.0)
			{
				keysToRemove.Insert(key);
			}
		}
		
		foreach (string key : keysToRemove)
		{
			m_mRecentWeaponFire.Remove(key);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Restore the boost after dampening period
	//------------------------------------------------------------------------------------------------
	protected void RestoreBoost()
	{
		// If protection was turned off while dampened, don't do anything
		if (!m_bIsActive || !m_PlayerPerception) 
			return;

		m_bIsDampened = false;
		m_PlayerPerception.SetAuditoryRange(m_fOriginalAuditoryRange * m_fBoostMultiplier);
		Print("[VSC] Dampening finished. Boost restored.", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// Called when the component is de-initialized (e.g., when unequipped)
	//------------------------------------------------------------------------------------------------
	override void EOnDeinit(IEntity owner)
	{
		// Unsubscribe from the global event to prevent memory leaks and errors
		BaseWorld world = GetGame().GetWorld();
		if (world)
		{
			world.GetOnExplosion().Remove(this.OnExplosion);
		}

		// Stop weapon sound monitoring
		if (m_bDetectWeaponSounds)
		{
			GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).Remove(MonitorWeaponSounds);
			GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).Remove(CleanupWeaponFireTracking);
		}
		
		// Clear weapon fire tracking
		if (m_mRecentWeaponFire)
		{
			m_mRecentWeaponFire.Clear();
		}

		// Restore the player's hearing to its original state
		if (m_bIsActive && m_PlayerPerception)
		{
			m_PlayerPerception.SetAuditoryRange(m_fOriginalAuditoryRange);
		}
		
		m_bIsActive = false;
		Print("[VSC] Active Hearing Protection Deactivated. Hearing restored to normal.", LogLevel.NORMAL);
		
		super.EOnDeinit(owner);
	}
}
