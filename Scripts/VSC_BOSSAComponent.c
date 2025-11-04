//------------------------------------------------------------------------------------------------
// Virtual Sound Compressor - BOSSA (Biologically Oriented Sound Segregation Algorithm) Component
// Brain-inspired algorithm that mimics how the human brain decodes sound using spatial cues
// and inhibitory filtering to enhance desired sounds in noisy environments.
// Author: jcrashkit
// Based on research by Kamal Sen, Alexander D. Boyd, and Virginia Best at Boston University
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(category: "GameScripted/Audio", description: "BOSSA algorithm: Brain-inspired sound segregation using spatial cues and inhibitory filtering to enhance speech and important sounds in noisy environments.")]
class VSC_BOSSAComponentClass : ScriptComponentClass
{
}

class VSC_BOSSAComponent : ScriptComponent
{
	// --- Spatial Filtering Parameters ---
	[Attribute(defvalue: "45", uiwidget: UIWidgets.EditBox, desc: "Field of attention cone angle in degrees. Sounds within this cone are enhanced.")]
	protected float m_fAttentionConeAngle;
	
	[Attribute(defvalue: "2.0", uiwidget: UIWidgets.Slider, desc: "Enhancement multiplier for sounds in the attention cone (front direction).", params: "1.0 5.0 0.1")]
	protected float m_fFrontEnhancementMultiplier;
	
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Suppression multiplier for sounds outside the attention cone (background noise).", params: "0.1 1.0 0.05")]
	protected float m_fBackgroundSuppressionMultiplier;
	
	// --- Selective Attention Parameters ---
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable enhanced detection of voice/communication sounds.")]
	protected bool m_bEnhanceVoices;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable enhanced detection of footsteps and movement sounds.")]
	protected bool m_bEnhanceMovement;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Enable enhanced detection of enemy combat sounds.")]
	protected bool m_bEnhanceCombat;
	
	[Attribute(defvalue: "1.5", uiwidget: UIWidgets.Slider, desc: "Multiplier for important sound categories (voices, movement, combat).", params: "1.0 3.0 0.1")]
	protected float m_fImportantSoundMultiplier;
	
	// --- Inhibitory Filtering Parameters (Brain-Inspired Noise Cancellation) ---
	[Attribute(defvalue: "0.7", uiwidget: UIWidgets.Slider, desc: "Inhibitory strength for competing sounds (0.0 = no inhibition, 1.0 = maximum).", params: "0.0 1.0 0.05")]
	protected float m_fInhibitoryStrength;
	
	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, desc: "Maximum number of concurrent sound sources to process for spatial filtering.")]
	protected int m_iMaxTrackedSources;
	
	// --- Temporal Processing (Brain's timing-based filtering) ---
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, desc: "Time window for sound analysis in seconds (brain's temporal processing window).", params: "0.05 0.5 0.01")]
	protected float m_fTemporalWindow;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Use temporal coherence to enhance sounds that persist over time (like voices).")]
	protected bool m_bUseTemporalCoherence;
	
	// --- Advanced Features ---
	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Enable adaptive learning - adjusts filtering based on player behavior.")]
	protected bool m_bAdaptiveLearning;
	
	[Attribute(defvalue: "0.3", uiwidget: UIWidgets.Slider, desc: "Adaptation rate for learning player preferences (0.0 = no adaptation, 1.0 = instant).", params: "0.0 1.0 0.05")]
	protected float m_fAdaptationRate;
	
	// --- Private Member Variables ---
	private ChimeraCharacter m_PlayerCharacter;
	private PerceptionComponent m_PlayerPerception;
	private float m_fOriginalAuditoryRange;
	private bool m_bIsActive = false;
	
	// Spatial sound tracking
	private ref array<ref VSC_TrackedSoundSource> m_aTrackedSources;
	private float m_fLastUpdateTime = 0.0;
	
	// Performance optimization - cached values
	private vector m_vCachedPlayerDir;
	private float m_fLastDirUpdateTime = 0.0;
	private int m_iUpdateCounter = 0; // For staggered updates
	private const int STAGGERED_UPDATE_INTERVAL = 3; // Update every Nth frame
	
	// Adaptive learning state
	private float m_fLearnedAttentionAngle = 45.0;
	private float m_fLearnedEnhancementLevel = 1.0;
	
	//------------------------------------------------------------------------------------------------
	// Initialize BOSSA algorithm
	//------------------------------------------------------------------------------------------------
	override void EOnPostInit(IEntity owner)
	{
		super.EOnPostInit(owner);
		
		m_aTrackedSources = new array<ref VSC_TrackedSoundSource>();
		
		// Initialize after a short delay to ensure player is ready
		GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).CallLater(InitializeBOSSA, 100, false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Initialize the BOSSA system
	//------------------------------------------------------------------------------------------------
	protected void InitializeBOSSA()
	{
		// Get the player character
		IEntity parent = GetOwner().GetParent();
		if (!parent)
			return;
			
		m_PlayerCharacter = ChimeraCharacter.Cast(parent);
		if (!m_PlayerCharacter) 
		{
			m_PlayerCharacter = ChimeraCharacter.Cast(GetOwner().FindComponent(ChimeraCharacter));
			if (!m_PlayerCharacter)
			{
				m_PlayerCharacter = ChimeraCharacter.Cast(GetOwner());
				if (!m_PlayerCharacter)
					return;
			}
		}

		// Only run for local player
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
			
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (controlledEntity != m_PlayerCharacter)
			return;
		
		m_PlayerPerception = PerceptionComponent.Cast(m_PlayerCharacter.FindComponent(PerceptionComponent));
		if (!m_PlayerPerception)
			return;
		
		// Store original hearing range
		m_fOriginalAuditoryRange = m_PlayerPerception.GetAuditoryRange();
		
		// Initialize learned parameters
		m_fLearnedAttentionAngle = m_fAttentionConeAngle;
		m_fLearnedEnhancementLevel = m_fFrontEnhancementMultiplier;
		
		// Start the BOSSA processing loop
		m_bIsActive = true;
		GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).CallLater(ProcessBOSSA, 50, true);
		
		Print("[VSC BOSSA] Biologically Oriented Sound Segregation Algorithm initialized", LogLevel.NORMAL);
		Print("[VSC BOSSA] Spatial filtering active - Attention cone: " + m_fAttentionConeAngle + " degrees", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	// Main BOSSA processing loop - mimics brain's continuous sound processing
	//------------------------------------------------------------------------------------------------
	protected void ProcessBOSSA()
	{
		if (!m_bIsActive || !m_PlayerCharacter || !m_PlayerPerception)
			return;
		
		float currentTime = GetGame().GetWorld().GetWorldTime();
		float deltaTime = currentTime - m_fLastUpdateTime;
		m_fLastUpdateTime = currentTime;
		
		// Update cached player direction (less frequently for performance)
		if (currentTime - m_fLastDirUpdateTime > 0.1) // Update every 100ms
		{
			m_vCachedPlayerDir = GetPlayerForwardDirection();
			m_fLastDirUpdateTime = currentTime;
		}
		
		// Staggered updates: Only update sound sources every Nth frame for performance
		m_iUpdateCounter++;
		if (m_iUpdateCounter >= STAGGERED_UPDATE_INTERVAL)
		{
			m_iUpdateCounter = 0;
			UpdateTrackedSources();
		}
		
		// Always apply filtering (lightweight operations)
		ApplySpatialFiltering();
		ApplyInhibitoryFiltering();
		ApplySelectiveAttention();
		
		// Adaptive learning (if enabled) - less frequent
		if (m_bAdaptiveLearning && m_iUpdateCounter == 0)
		{
			UpdateAdaptiveLearning();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Update the list of tracked sound sources in the environment
	//------------------------------------------------------------------------------------------------
	protected void UpdateTrackedSources()
	{
		float currentTime = GetGame().GetWorld().GetWorldTime();
		
		// Clear old sources outside temporal window (optimized: only check when needed)
		if (m_aTrackedSources.Count() > 0)
		{
			for (int i = m_aTrackedSources.Count() - 1; i >= 0; i--)
			{
				VSC_TrackedSoundSource source = m_aTrackedSources[i];
				if (currentTime - source.m_fLastUpdateTime > m_fTemporalWindow * 2.0)
				{
					m_aTrackedSources.Remove(i);
				}
			}
		}
		
		// Early exit if we're at max capacity and all sources are recent
		if (m_aTrackedSources.Count() >= m_iMaxTrackedSources)
		{
			// Only update existing tracked sources, don't add new ones
			UpdateExistingSources();
			return;
		}
		
		// Find nearby sound sources
		vector playerPos = m_PlayerCharacter.GetOrigin();
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		// Search for entities within auditory range (optimize range)
		array<Managed> found = {};
		array<Class> excludeClasses = {};
		array<Object> objects = {};
		
		// Use more conservative search range for performance
		float searchRange = m_fOriginalAuditoryRange * 1.5; // Reduced from full enhancement multiplier
		world.FindEntitiesAround(playerPos, searchRange, excludeClasses, found, objects);
		
		// Process found entities (prioritize characters for footstep detection)
		foreach (Managed obj : found)
		{
			IEntity entity = IEntity.Cast(obj);
			if (!entity || entity == m_PlayerCharacter)
				continue;
			
			// Check if entity produces sounds (with early exit)
			if (HasSoundComponent(entity))
			{
				TrackSoundSource(entity, playerPos, m_vCachedPlayerDir);
				
				// Early exit if we've reached max sources
				if (m_aTrackedSources.Count() >= m_iMaxTrackedSources)
					break;
			}
		}
		
		// Update existing sources that weren't found in this search
		UpdateExistingSources();
	}
	
	//------------------------------------------------------------------------------------------------
	// Update existing tracked sources (performance optimization)
	//------------------------------------------------------------------------------------------------
	protected void UpdateExistingSources()
	{
		vector playerPos = m_PlayerCharacter.GetOrigin();
		
		foreach (VSC_TrackedSoundSource source : m_aTrackedSources)
		{
			if (!source.m_Entity)
				continue;
			
			// Quick update for existing sources
			vector sourcePos = source.m_Entity.GetOrigin();
			float distance = vector.Distance(playerPos, sourcePos);
			
			// Update only if significant change
			if (Math.Abs(distance - source.m_fDistance) > 2.0)
			{
				TrackSoundSource(source.m_Entity, playerPos, m_vCachedPlayerDir);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if an entity has sound-producing components
	// Optimized with early exits for performance
	//------------------------------------------------------------------------------------------------
	protected bool HasSoundComponent(IEntity entity)
	{
		if (!entity)
			return false;
		
		// Check for weapon sounds (combat sounds)
		if (WeaponSoundComponent.Cast(entity.FindComponent(WeaponSoundComponent)))
			return true;
		
		// Check for character movement (footsteps, voices)
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character)
		{
			// Always track characters - they produce footsteps, voices, and combat sounds
			// Check for movement component specifically for footsteps
			CharacterMovementComponent movementComp = CharacterMovementComponent.Cast(character.FindComponent(CharacterMovementComponent));
			if (movementComp && m_bEnhanceMovement)
			{
				// Character has movement component - definitely produces footstep sounds
				return true;
			}
			
			// Even without explicit movement component, characters make sounds
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get player's current forward direction
	//------------------------------------------------------------------------------------------------
	protected vector GetPlayerForwardDirection()
	{
		if (!m_PlayerCharacter)
			return "0 0 1"; // Default forward
		
		Transform playerTransform = m_PlayerCharacter.GetTransform();
		return playerTransform[2]; // Forward vector from transform matrix
	}
	
	//------------------------------------------------------------------------------------------------
	// Track a sound source and calculate spatial properties
	//------------------------------------------------------------------------------------------------
	protected void TrackSoundSource(IEntity entity, vector playerPos, vector playerDir)
	{
		vector sourcePos = entity.GetOrigin();
		vector toSource = sourcePos - playerPos;
		float distance = vector.Distance(playerPos, sourcePos);
		
		// Normalize direction vector
		float toSourceLen = toSource.Length();
		float playerDirLen = playerDir.Length();
		if (toSourceLen > 0.001)
			toSource = toSource / toSourceLen;
		if (playerDirLen > 0.001)
			playerDir = playerDir / playerDirLen;
		
		// Calculate angle from player's forward direction
		float dotProduct = vector.Dot(toSource, playerDir);
		// Clamp dot product to valid range for acos
		dotProduct = Math.Clamp(dotProduct, -1.0, 1.0);
		// Use approximate acos calculation (acos in radians, convert to degrees)
		float angleRad = Math.Acos(dotProduct);
		float angle = angleRad * 57.295779513; // RAD2DEG constant
		
		// Find or create tracked source
		VSC_TrackedSoundSource trackedSource = null;
		foreach (VSC_TrackedSoundSource source : m_aTrackedSources)
		{
			if (source.m_Entity == entity)
			{
				trackedSource = source;
				break;
			}
		}
		
		if (!trackedSource)
		{
			if (m_aTrackedSources.Count() >= m_iMaxTrackedSources)
				return; // Too many sources
			
			trackedSource = new VSC_TrackedSoundSource();
			trackedSource.m_Entity = entity;
			m_aTrackedSources.Insert(trackedSource);
		}
		
		// Update source properties
		trackedSource.m_fDistance = distance;
		trackedSource.m_fAngle = angle;
		trackedSource.m_fLastUpdateTime = GetGame().GetWorld().GetWorldTime();
		trackedSource.m_bIsImportant = IsImportantSound(entity);
		trackedSource.m_bIsFootstep = IsFootstepSound(entity);
		
		// Calculate intensity (simplified - would use actual audio levels in real implementation)
		trackedSource.m_fIntensity = CalculateSoundIntensity(entity, distance);
	}
	
	//------------------------------------------------------------------------------------------------
	// Determine if a sound source is "important" (voice, movement, combat)
	// Enhanced for footstep detection
	//------------------------------------------------------------------------------------------------
	protected bool IsImportantSound(IEntity entity)
	{
		if (!entity)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character)
		{
			// Check for movement component - prioritize footsteps when movement enhancement is on
			if (m_bEnhanceMovement)
			{
				CharacterMovementComponent movementComp = CharacterMovementComponent.Cast(character.FindComponent(CharacterMovementComponent));
				if (movementComp)
				{
					// This character can produce footsteps - always important
					return true;
				}
			}
			
			// Characters can produce voices, footsteps, combat sounds
			if (m_bEnhanceVoices || m_bEnhanceMovement || m_bEnhanceCombat)
				return true;
		}
		
		WeaponSoundComponent weapon = WeaponSoundComponent.Cast(entity.FindComponent(WeaponSoundComponent));
		if (weapon && m_bEnhanceCombat)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Check if sound source is specifically a footstep/movement sound
	//------------------------------------------------------------------------------------------------
	protected bool IsFootstepSound(IEntity entity)
	{
		if (!m_bEnhanceMovement)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return false;
		
		CharacterMovementComponent movementComp = CharacterMovementComponent.Cast(character.FindComponent(CharacterMovementComponent));
		return movementComp != null;
	}
	
	//------------------------------------------------------------------------------------------------
	// Calculate sound intensity (simplified model)
	// Enhanced for footsteps - they need to be clearly audible
	//------------------------------------------------------------------------------------------------
	protected float CalculateSoundIntensity(IEntity entity, float distance)
	{
		// Base intensity decreases with distance (inverse square law simplified)
		float baseIntensity = 1.0 / (1.0 + distance * 0.1);
		
		// Special handling for footsteps - they're quieter but need to be heard
		if (IsFootstepSound(entity))
		{
			// Footsteps get significant boost to ensure they're audible
			// Compensate for their naturally lower volume
			baseIntensity *= 2.5; // Strong boost for footsteps
			
			// Further boost for close footsteps (within 30m)
			if (distance < 30.0)
				baseIntensity *= 1.3;
		}
		// Boost for other important sounds
		else if (IsImportantSound(entity))
		{
			baseIntensity *= 1.5;
		}
		
		return baseIntensity;
	}
	
	//------------------------------------------------------------------------------------------------
	// Apply spatial filtering - enhance sounds in attention cone, suppress background
	// This mimics the brain's directional processing
	//------------------------------------------------------------------------------------------------
	protected void ApplySpatialFiltering()
	{
		if (!m_PlayerPerception)
			return;
		
		float effectiveRange = m_fOriginalAuditoryRange;
		float frontEnhancement = 0.0;
		float backgroundSuppression = 0.0;
		int frontCount = 0;
		int backCount = 0;
		
		// Use learned angle if adaptive learning is enabled
		float attentionAngle = m_bAdaptiveLearning ? m_fLearnedAttentionAngle : m_fAttentionConeAngle;
		float halfAngle = attentionAngle / 2.0;
		
		foreach (VSC_TrackedSoundSource source : m_aTrackedSources)
		{
			if (source.m_fAngle <= halfAngle)
			{
				// Sound is in attention cone - enhance
				frontEnhancement += source.m_fIntensity;
				frontCount++;
			}
			else
			{
				// Sound is background noise - suppress
				backgroundSuppression += source.m_fIntensity;
				backCount++;
			}
		}
		
		// Calculate dynamic range adjustment
		float enhancementFactor = 1.0;
		if (frontCount > 0)
		{
			// Enhance front sounds
			float learnedMultiplier = m_bAdaptiveLearning ? m_fLearnedEnhancementLevel : m_fFrontEnhancementMultiplier;
			enhancementFactor = learnedMultiplier;
		}
		
		if (backCount > 0 && m_fInhibitoryStrength > 0.0)
		{
			// Suppress background using inhibitory filtering
			float suppression = 1.0 - (backgroundSuppression / (backCount + 1.0)) * m_fInhibitoryStrength * m_fBackgroundSuppressionMultiplier;
			enhancementFactor *= Math.Clamp(suppression, m_fBackgroundSuppressionMultiplier, 1.0);
		}
		
		// Apply the calculated range
		effectiveRange = m_fOriginalAuditoryRange * enhancementFactor;
		m_PlayerPerception.SetAuditoryRange(effectiveRange);
	}
	
	//------------------------------------------------------------------------------------------------
	// Apply inhibitory filtering - brain-inspired noise cancellation
	// Uses competitive inhibition to suppress competing sounds
	//------------------------------------------------------------------------------------------------
	protected void ApplyInhibitoryFiltering()
	{
		if (m_fInhibitoryStrength <= 0.0)
			return;
		
		// Find the strongest sound source (most important)
		VSC_TrackedSoundSource strongestSource = null;
		float maxImportance = 0.0;
		
		foreach (VSC_TrackedSoundSource source : m_aTrackedSources)
		{
			float importance = source.m_fIntensity;
			
			// Footsteps get highest priority (they're quiet but critical)
			if (source.m_bIsFootstep)
				importance *= 3.0; // Strong boost for footsteps
			else if (source.m_bIsImportant)
				importance *= 2.0; // Boost important sounds
			
			// Prefer sounds in attention cone
			float halfAngle = m_fAttentionConeAngle / 2.0;
			if (source.m_fAngle <= halfAngle)
				importance *= 1.5;
			
			if (importance > maxImportance)
			{
				maxImportance = importance;
				strongestSource = source;
			}
		}
		
		// If we have a strong source, suppress competing sounds
		if (strongestSource)
		{
			// In a real implementation, this would adjust individual sound volumes
			// Here we apply it through the perception range system
			// The spatial filtering already handles this conceptually
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Apply selective attention - enhance important sound categories
	// Enhanced to prioritize footsteps
	//------------------------------------------------------------------------------------------------
	protected void ApplySelectiveAttention()
	{
		if (!m_PlayerPerception)
			return;
		
		// Count important sounds and footsteps separately
		int importantCount = 0;
		int footstepCount = 0;
		
		foreach (VSC_TrackedSoundSource source : m_aTrackedSources)
		{
			float halfAngle = m_fAttentionConeAngle / 2.0;
			bool inAttentionCone = source.m_fAngle <= halfAngle;
			
			if (inAttentionCone)
			{
				if (source.m_bIsFootstep)
				{
					footstepCount++;
				}
				else if (source.m_bIsImportant)
				{
					importantCount++;
				}
			}
		}
		
		// Apply boosting - footsteps get extra boost
		float boostMultiplier = 1.0;
		
		if (footstepCount > 0)
		{
			// Footsteps get additional boost to ensure audibility
			boostMultiplier = m_fImportantSoundMultiplier * 1.4; // Extra boost for footsteps
		}
		else if (importantCount > 0)
		{
			boostMultiplier = m_fImportantSoundMultiplier;
		}
		
		// Apply the boost
		if (boostMultiplier > 1.0)
		{
			float currentRange = m_PlayerPerception.GetAuditoryRange();
			float boostedRange = currentRange * boostMultiplier;
			m_PlayerPerception.SetAuditoryRange(boostedRange);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Update adaptive learning based on player behavior
	//------------------------------------------------------------------------------------------------
	protected void UpdateAdaptiveLearning()
	{
		// Simple adaptive learning: adjust attention angle based on where important sounds are
		float avgImportantAngle = 0.0;
		int importantCount = 0;
		
		foreach (VSC_TrackedSoundSource source : m_aTrackedSources)
		{
			if (source.m_bIsImportant)
			{
				avgImportantAngle += source.m_fAngle;
				importantCount++;
			}
		}
		
		if (importantCount > 0)
		{
			avgImportantAngle /= importantCount;
			
			// Gradually adjust learned angle toward where important sounds are
			float targetAngle = avgImportantAngle * 2.0; // Expand to cover important sounds
			// Manual lerp: lerp(a, b, t) = a + (b - a) * t
			float lerpFactor = m_fAdaptationRate * 0.1;
			m_fLearnedAttentionAngle = m_fLearnedAttentionAngle + (targetAngle - m_fLearnedAttentionAngle) * lerpFactor;
			m_fLearnedAttentionAngle = Math.Clamp(m_fLearnedAttentionAngle, 30.0, 90.0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Cleanup
	//------------------------------------------------------------------------------------------------
	override void EOnDeinit(IEntity owner)
	{
		if (m_bIsActive && m_PlayerPerception)
		{
			m_PlayerPerception.SetAuditoryRange(m_fOriginalAuditoryRange);
		}
		
		m_bIsActive = false;
		GetGame().GetCallqueue(CALL_CATEGORY_GAMEPLAY).Remove(ProcessBOSSA);
		
		Print("[VSC BOSSA] Algorithm deactivated. Hearing restored to normal.", LogLevel.NORMAL);
		
		super.EOnDeinit(owner);
	}
}

//------------------------------------------------------------------------------------------------
// Data structure for tracking sound sources
//------------------------------------------------------------------------------------------------
class VSC_TrackedSoundSource
{
	IEntity m_Entity;
	float m_fDistance;
	float m_fAngle; // Angle from player's forward direction in degrees
	float m_fIntensity;
	bool m_bIsImportant;
	bool m_bIsFootstep; // Specifically tracks if this is a footstep sound
	float m_fLastUpdateTime;
	
	void VSC_TrackedSoundSource()
	{
		m_fDistance = 0.0;
		m_fAngle = 0.0;
		m_fIntensity = 0.0;
		m_bIsImportant = false;
		m_bIsFootstep = false;
		m_fLastUpdateTime = 0.0;
	}
}

