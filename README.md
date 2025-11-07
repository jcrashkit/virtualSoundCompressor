# Virtual Sound Compressor (VSC) https://reforger.armaplatform.com/workshop/A1B2C3D4E5F6A7B8-VirtualSoundCompressor

Active hearing protection system with BOSSA algorithm for Arma Reforger. Automatically amplifies quiet sounds (like footsteps) while dampening loud noises (explosions, gunfire) to protect hearing. Features brain-inspired BOSSA (Biologically Oriented Sound Segregation Algorithm) for enhanced speech recognition and selective attention in noisy environments.

## Features

- **Active Hearing Protection**: Automatically dampens loud explosions and gunfire
- **Sound Enhancement**: Boosts quiet ambient sounds and footsteps
- **BOSSA Algorithm**: Brain-inspired sound segregation for better audio clarity
- **Automatic Headgear Detection**: Automatically attaches to headgear items
- **Scalable**: Optimized for up to 128 concurrent players
- **Low Latency**: ~50ms response time for sound dampening

## Components Overview

### 1. VSC_MainComponent
**Purpose**: Main entry point for the mod  
**Where to Attach**: World entity or game manager (optional)  
**Server/Client**: Both  
**Required**: Optional (informational only)

### 2. VSC_HeadgearManagerComponent
**Purpose**: Automatically detects headgear and auto-attaches both Protection and BOSSA with a single toggle  
**Where to Attach**: World entity or any persistent game entity on the **SERVER**  
**Server/Client**: **SERVER ONLY**  
**Required**: Yes (for automatic headgear detection)

### 3. VSC_ActiveHearingProtectionComponent
**Purpose**: Core hearing protection - dampens loud sounds and boosts quiet ones  
**Where to Attach**: Auto-attached by manager, OR manually to headgear items  
**Server/Client**: Both (affects local player)  
**Required**: Yes (auto-attached)

### 4. VSC_BOSSAComponent
**Purpose**: Advanced BOSSA algorithm for spatial sound filtering  
**Where to Attach**: Auto-attached by manager, OR manually to headgear items  
**Server/Client**: Both (affects local player)  
**Required**: Included by default via manager

---

## Production Server Setup

### Step 1: Install the Mod

1. Place the `virtualSoundCompressor` folder in your Arma Reforger Workbench addons directory:
   ```
   ArmaReforgerWorkbench/addons/virtualSoundCompressor/
   ```

2. Ensure the mod is enabled in your server configuration.

### Step 2: Attach VSC_HeadgearManagerComponent (REQUIRED)

**This is the most important step for automatic headgear detection.**

1. In the Arma Reforger Workbench, open your world/mission file
2. Find a persistent entity (World, GameManager, or similar persistent entity)
3. Add the component:
   - Right-click the entity → Add Component
   - Select: `VSC_HeadgearManagerComponent`
   - Ensure it's marked as **SERVER ONLY** (not replicated to clients)
4. In the component attributes, keep **Auto-attach VSC components (Protection + BOSSA)** enabled (default: true)

### Step 3: Verify Automatic Attachment

The system will automatically:
- Detect when players equip headgear (helmets, caps, headphones, etc.)
- Attach `VSC_ActiveHearingProtectionComponent` and `VSC_BOSSAComponent` to the headgear item
- Only activate on the local client for that player

**Manual Attachment (Alternative)**:
If you prefer manual control, you can attach `VSC_ActiveHearingProtectionComponent` and/or `VSC_BOSSAComponent` directly to headgear items in the Workbench.

### Step 4: Configure Component Attributes (Optional)

Each component has configurable attributes accessible in the Workbench:

#### VSC_ActiveHearingProtectionComponent Attributes:
- **Boost Multiplier**: `1.75`
- **Dampen Multiplier**: `0.25`
- **Dampen Trigger Range**: `25` meters
- **Dampen Duration**: `400` ms
- **Detect Weapon Sounds**: `true`
- **Weapon Sound Trigger Range**: `15` meters
- **Weapon Sound Duration**: `200` ms
- **Dampening Cooldown**: `0.5` seconds

#### VSC_BOSSAComponent Attributes:
- **Attention Cone Angle**: `45` degrees
- **Front Enhancement Multiplier**: `2.0`
- **Background Suppression**: `0.5`
- **Enhance Voices**: `true`
- **Enhance Movement**: `true`
- **Enhance Combat**: `true`
- **Temporal Window / Adaptive Learning**: optional tuning

---

## Server Configuration

### mod.json Configuration

Already configured:
```json
{
  "serverModules": [
    "VSC_MainComponent",
    "VSC_HeadgearManagerComponent",
    "VSC_ActiveHearingProtectionComponent",
    "VSC_BOSSAComponent"
  ],
  "clientModules": [
    "VSC_MainComponent",
    "VSC_ActiveHearingProtectionComponent",
    "VSC_BOSSAComponent"
  ]
}
```

**Notes**:
- `VSC_HeadgearManagerComponent` is **SERVER ONLY** (do not add it to clientModules)
- Protection/BOSSA are present on both for runtime class availability

### Performance Considerations

Optimized for **128 concurrent players**:
1. **Explosion Detection**: Event-driven
2. **Weapon Detection**: 50ms poll, checks up to 16 nearby entities per frame
3. **Headgear Manager**: 8 characters per frame (every 200ms)
4. **Scoped Searches**: Limited radii

---

## How It Works

1. **Server**: `VSC_HeadgearManagerComponent` monitors players
2. **Detection**: On headgear equip, attaches both components
3. **Client**: Components act only on local player audio
4. **Protection**: Detects explosions/gunshots, applies dampening briefly
5. **BOSSA**: Spatially favors front/important sounds

---

## Troubleshooting

- Ensure the manager is attached server-side and the auto-attach toggle is enabled
- Check logs for `[VSC Manager]` and `[VSC]` outputs
- If needed, manually attach components to headgear to test

---

## Files Structure

```
virtualSoundCompressor/
├── Scripts/
│   ├── VSC_MainComponent.c                    # Main entry point
│   ├── VSC_HeadgearManagerComponent.c        # Auto-attachment manager (SERVER)
│   ├── VSC_ActiveHearingProtectionComponent.c # Core hearing protection
│   └── VSC_BOSSAComponent.c                   # Advanced BOSSA algorithm
├── mod.json                                   # Mod configuration
├── addon.gproj                                # Project file
└── README.md                                  # This file
```

---

## Support

- Check server logs for `[VSC]` and `[VSC Manager]` messages
- Verify component attachment in Workbench
- Ensure mod is enabled in server configuration

