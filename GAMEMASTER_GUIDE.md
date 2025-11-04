# Game Master Guide: Manual Component Attachment

This guide is for game masters and mission makers who want to manually attach Virtual Sound Compressor components to specific headgear items in the Arma Reforger Workbench.

---

## Quick Start

**For most users**: The automatic detection system (VSC_HeadgearManagerComponent) will handle everything. You only need this manual guide if:
- You want to attach to specific headgear items manually
- You're using custom headgear that isn't auto-detected
- You want to configure different settings per headgear type
- You prefer manual control over automatic detection

---

## Step-by-Step: Attaching Components to Headgear

### Step 1: Open Your Mission/World in Workbench

1. Launch **Arma Reforger Workbench**
2. Open your mission file or world configuration
3. Navigate to the entity hierarchy (usually in the left panel)

### Step 2: Locate Headgear Items

Headgear items can be found in several places:

**Option A: Pre-placed Items**
- Look in the entity hierarchy for items like:
  - `Helmet_*`
  - `Cap_*`
  - `Headphone_*`
  - `Headset_*`
  - Any item with "head", "helmet", "cap" in the name

**Option B: Item Templates/Archetypes**
- Check the **Archetypes** or **Templates** section
- Look for headgear item templates
- These are the "blueprints" that get instantiated when players spawn

**Option C: Character Templates**
- Find character templates/archetypes
- Expand to see their inventory/equipment slots
- Headgear is typically in slot 1 or the HEADGEAR slot

### Step 3: Select the Headgear Item

1. **Click on the headgear item** in the hierarchy
2. The item properties will appear in the right panel
3. Verify it's the correct item by checking the name

### Step 4: Add VSC_ActiveHearingProtectionComponent

1. In the **Properties Panel** (right side), scroll to the **Components** section
2. Click the **"+"** or **"Add Component"** button
3. In the component selection dialog:
   - Search for: `VSC_ActiveHearingProtectionComponent`
   - Or browse: `GameScripted > Audio > VSC_ActiveHearingProtectionComponent`
4. Select it and click **OK**

The component is now attached to the headgear item.

### Step 5: Configure Component Attributes (Optional)

With the component selected, you can configure its attributes in the Properties Panel:

#### Essential Attributes:

- **Boost Multiplier** (`1.75`): How much quiet sounds are amplified
  - Higher = louder quiet sounds
  - Range: 1.0 - 5.0
  - Recommended: 1.75 for balanced gameplay

- **Dampen Multiplier** (`0.25`): How much loud sounds are reduced
  - Lower = more protection from loud sounds
  - Range: 0.1 - 1.0
  - Recommended: 0.25 for good protection

- **Dampen Trigger Range** (`25` meters): Distance from explosions to trigger protection
  - Increase for larger blast radius detection
  - Range: 5-100 meters
  - Recommended: 25m for most scenarios

#### Advanced Attributes:

- **Dampen Duration** (`400` ms): How long protection lasts after loud sound
- **Detect Weapon Sounds** (`true`): Enable gunshot detection
- **Weapon Sound Trigger Range** (`15` meters): Distance for weapon detection
- **Weapon Sound Duration** (`200` ms): How long weapon protection lasts
- **Dampening Cooldown** (`0.5` seconds): Minimum time between triggers

### Step 6: Add VSC_BOSSAComponent (Optional)

For advanced spatial sound filtering:

1. While still on the headgear item, click **"Add Component"** again
2. Select: `VSC_BOSSAComponent`
3. Configure BOSSA attributes:
   - **Attention Cone Angle**: `45` degrees (sounds in front are enhanced)
   - **Front Enhancement Multiplier**: `2.0` (boost for forward sounds)
   - **Background Suppression**: `0.5` (reduce background noise)
   - **Enhance Voices**: `true` (boost voice/communication)
   - **Enhance Movement**: `true` (boost footsteps)
   - **Enhance Combat**: `true` (boost enemy combat sounds)

### Step 7: Save and Test

1. **Save** your mission/world file
2. **Test in-game**:
   - Equip the headgear item
   - Listen for boosted quiet sounds
   - Trigger an explosion nearby (within 25m)
   - Verify audio dampening occurs

---

## Examples: Common Headgear Types

### Example 1: Standard Combat Helmet

```
Entity: Helmet_Combat_01
Components:
  ├─ VSC_ActiveHearingProtectionComponent
  │   ├─ Boost Multiplier: 1.75
  │   ├─ Dampen Multiplier: 0.25
  │   ├─ Dampen Trigger Range: 25
  │   └─ Detect Weapon Sounds: true
  └─ VSC_BOSSAComponent (optional)
      ├─ Attention Cone Angle: 45
      └─ Enhance Movement: true
```

### Example 2: Communication Headset

```
Entity: Headset_Comms_01
Components:
  ├─ VSC_ActiveHearingProtectionComponent
  │   ├─ Boost Multiplier: 2.0 (higher for comms)
  │   ├─ Dampen Multiplier: 0.2 (more protection)
  │   └─ Detect Weapon Sounds: true
  └─ VSC_BOSSAComponent
      ├─ Attention Cone Angle: 60 (wider for comms)
      ├─ Enhance Voices: true
      └─ Enhance Movement: true
```

### Example 3: Lightweight Cap/Beanie

```
Entity: Cap_Baseball_01
Components:
  └─ VSC_ActiveHearingProtectionComponent
      ├─ Boost Multiplier: 1.5 (less aggressive)
      ├─ Dampen Multiplier: 0.3 (less protection)
      └─ Dampen Trigger Range: 20 (shorter range)
```

---

## Best Practices

### 1. **Consistent Configuration**
- Use the same settings across similar headgear types
- This ensures consistent gameplay experience

### 2. **Performance Tiers**
Consider creating different tiers:

- **Premium Headgear** (Comms, Advanced Helmets):
  - Full BOSSA component
  - Higher boost multipliers
  - Better protection

- **Standard Headgear** (Basic Helmets):
  - Active Hearing Protection only
  - Balanced settings

- **Lightweight Headgear** (Caps, Beanies):
  - Active Hearing Protection only
  - Lower multipliers (less aggressive)

### 3. **Testing**
- Test each headgear type in-game
- Verify explosion detection works
- Check weapon fire detection
- Ensure audio feels balanced

### 4. **Performance Considerations**
- **Don't attach BOSSAComponent** to every headgear if not needed
- BOSSA has higher CPU usage (processes every 50ms)
- Use it only for premium/comm headgear

### 5. **Documentation**
- Keep a list of which headgear has which components
- Note any custom configurations
- This helps with updates and troubleshooting

---

## Troubleshooting

### Component Not Appearing in List

**Problem**: Can't find `VSC_ActiveHearingProtectionComponent` in component list

**Solutions**:
1. Verify mod is loaded in Workbench
2. Check mod.json has the component in serverModules/clientModules
3. Restart Workbench
4. Verify script files are in correct location

### Component Attached But Not Working

**Problem**: Component is attached but audio doesn't change

**Solutions**:
1. **Check it's on the correct item**: Must be on the headgear item itself, not the character
2. **Verify PerceptionComponent**: The player character must have PerceptionComponent
3. **Test ranges**: Ensure explosions/weapons are within trigger ranges
4. **Check cooldown**: Wait 0.5 seconds between tests
5. **Verify local player**: Component only works for the local player (client-side)

### Performance Issues

**Problem**: Game becomes laggy after attaching components

**Solutions**:
1. **Remove BOSSAComponent**: If not needed, it uses more CPU
2. **Reduce polling**: Increase weapon detection interval (requires code change)
3. **Limit components**: Only attach to frequently-used headgear
4. **Use automatic detection**: Let VSC_HeadgearManagerComponent handle it

---

## Advanced: Per-Item Configuration

### Different Settings for Different Headgear

You can create different "tiers" of hearing protection:

**Tier 1 - Basic (Caps, Beanies)**:
```
Boost Multiplier: 1.5
Dampen Multiplier: 0.3
Dampen Trigger Range: 20
No BOSSA Component
```

**Tier 2 - Standard (Combat Helmets)**:
```
Boost Multiplier: 1.75
Dampen Multiplier: 0.25
Dampen Trigger Range: 25
No BOSSA Component
```

**Tier 3 - Advanced (Comms Headsets)**:
```
Boost Multiplier: 2.0
Dampen Multiplier: 0.2
Dampen Trigger Range: 30
BOSSA Component: Enabled
  - Attention Cone: 60 degrees
  - Enhance Voices: true
  - Enhance Movement: true
```

---

## Quick Reference: Component Checklist

When attaching components to headgear:

- [ ] Item selected in Workbench
- [ ] `VSC_ActiveHearingProtectionComponent` added
- [ ] Boost Multiplier configured (1.75 recommended)
- [ ] Dampen Multiplier configured (0.25 recommended)
- [ ] Dampen Trigger Range configured (25m recommended)
- [ ] Detect Weapon Sounds enabled (if desired)
- [ ] `VSC_BOSSAComponent` added (if advanced features needed)
- [ ] BOSSA attributes configured (if using)
- [ ] Mission saved
- [ ] Tested in-game

---

## Alternative: Use Automatic Detection

**Remember**: If you don't want to manually attach components, you can:

1. Attach `VSC_HeadgearManagerComponent` to a server-side entity
2. Let it automatically detect and attach components to headgear
3. See the main README.md for automatic setup instructions

The automatic system will:
- Detect headgear by inventory slot and name
- Automatically attach components when players equip headgear
- Work for all players without manual configuration

---

## Support

If you encounter issues:

1. Check the main README.md troubleshooting section
2. Verify component attachment in Workbench
3. Check server logs for `[VSC]` messages
4. Test with default settings first
5. Verify the mod is enabled in server configuration

---

**Note**: This manual process is optional. The automatic detection system (VSC_HeadgearManagerComponent) handles most use cases automatically. Use this guide only if you need manual control or custom configurations.

