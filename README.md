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
**Purpose**: Automatically detects and attaches hearing protection to headgear items  
**Where to Attach**: World entity or any persistent game entity on the **SERVER**  
**Server/Client**: **SERVER ONLY**  
**Required**: Yes (for automatic headgear detection)

### 3. VSC_ActiveHearingProtectionComponent
**Purpose**: Core hearing protection - dampens loud sounds and boosts quiet ones  
**Where to Attach**: Automatically attached by VSC_HeadgearManagerComponent, OR manually to headgear items  
**Server/Client**: Both (works on client for local player)  
**Required**: Yes (automatically attached)

### 4. VSC_BOSSAComponent
**Purpose**: Advanced BOSSA algorithm for spatial sound filtering  
**Where to Attach**: Automatically attached by VSC_HeadgearManagerComponent, OR manually to headgear items  
**Server/Client**: Both (works on client for local player)  
**Required**: Optional (for advanced sound processing)

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

**Alternative**: If you want to attach it programmatically, you can add it to any entity that exists on the server. The component will automatically start monitoring for headgear.

### Step 3: Verify Automatic Attachment

The system will automatically:
- Detect when players equip headgear (helmets, caps, headphones, etc.)
- Attach `VSC_ActiveHearingProtectionComponent` to the headgear item
- Only activate for the local player (client-side filtering)

**Manual Attachment (Alternative)**:
If you prefer manual control, you can attach `VSC_ActiveHearingProtectionComponent` directly to headgear items in the workbench. However, automatic detection is recommended for production servers.

### Step 4: Configure Component Attributes (Optional)

Each component has configurable attributes accessible in the Workbench:

#### VSC_ActiveHearingProtectionComponent Attributes:
- **Boost Multiplier**: `1.75` - Multiplier for quiet sounds (default: 1.75)
- **Dampen Multiplier**: `0.25` - Multiplier when dampening loud sounds (default: 0.25)
- **Dampen Trigger Range**: `25` meters - Max distance from explosion to trigger (default: 25m)
- **Dampen Duration**: `400` ms - How long dampening lasts (default: 400ms)
- **Detect Weapon Sounds**: `true` - Enable gunshot detection (default: true)
- **Weapon Sound Trigger Range**: `15` meters - Max distance for weapon detection (default: 15m)
- **Weapon Sound Duration**: `200` ms - How long weapon dampening lasts (default: 200ms)
- **Dampening Cooldown**: `0.5` seconds - Minimum time between triggers (default: 0.5s)

#### VSC_BOSSAComponent Attributes:
- **Attention Cone Angle**: `45` degrees - Field of attention (default: 45°)
- **Front Enhancement Multiplier**: `2.0` - Boost for sounds in front (default: 2.0)
- **Background Suppression**: `0.5` - Suppression for background noise (default: 0.5)
- **Enhance Voices**: `true` - Enhance voice/communication sounds
- **Enhance Movement**: `true` - Enhance footsteps
- **Enhance Combat**: `true` - Enhance enemy combat sounds
- And many more advanced BOSSA parameters...

---

## Server Configuration

### mod.json Configuration

The mod.json file is already configured with:
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

**Important Notes**:
- `VSC_HeadgearManagerComponent` is **SERVER ONLY** - it should not be in clientModules
- Other components are on both server and client for proper operation

### Performance Considerations

The system is optimized for **128 concurrent players**:

1. **Explosion Detection**: Event-driven (no polling overhead)
2. **Weapon Detection**: Polls every 50ms, checks max 16 nearby entities per frame
3. **Headgear Manager**: Processes 8 characters per frame (200ms intervals)
4. **Search Radius**: Limited to trigger ranges (15-25m) to avoid checking distant players

**Expected Performance**:
- Minimal CPU impact per player
- ~50ms latency for sound dampening
- Scales linearly with player count

---

## How It Works

### Automatic Detection Flow

1. **Server**: `VSC_HeadgearManagerComponent` monitors all players (8 per frame)
2. **Detection**: When headgear is equipped, component is automatically attached
3. **Client**: Component activates only for the local player
4. **Protection**: System monitors for explosions and weapon fire
5. **Response**: Automatically dampens audio when loud sounds are detected

### Player Position Relevance

- **Server-side**: Headgear manager runs on server, processes all players
- **Client-side**: Each component only affects the local player's audio
- **Isolation**: Players cannot affect other players' audio settings
- **Performance**: Each client only processes their own audio, not all 128 players

### Sound Detection

**Explosions**:
- Event-driven detection (instant response)
- Triggers within 25m range (configurable)
- Dampens for 400ms (configurable)

**Weapon Fire**:
- Polls every 50ms for nearby weapons
- Detects projectiles within 15m range (configurable)
- Tracks recent weapon fire to avoid duplicate triggers
- Dampens for 200ms (configurable)

---

## Troubleshooting

### Components Not Attaching Automatically

1. **Check VSC_HeadgearManagerComponent**: Ensure it's attached to a server-side entity
2. **Verify Server/Client**: Headgear manager must be server-only
3. **Check Logs**: Look for `[VSC Manager]` messages in server logs
4. **Manual Attachment**: As fallback, manually attach components to headgear items

### Performance Issues

1. **Reduce Polling**: Increase weapon detection interval if needed (default: 50ms)
2. **Limit Entities**: Reduce max entity checks per frame (default: 16)
3. **Disable BOSSA**: If not needed, don't attach BOSSAComponent for better performance

### Audio Not Dampening

1. **Check Local Player**: Ensure component is attached to headgear on local player
2. **Verify PerceptionComponent**: Player must have PerceptionComponent
3. **Check Ranges**: Ensure explosions/weapons are within trigger ranges
4. **Cooldown**: Check if cooldown period is preventing triggers

---

## Advanced Configuration

### Custom Headgear Detection

The headgear manager detects items by:
- Inventory slot (slots 1, 2, 3)
- Item name keywords: "helmet", "cap", "hat", "head", "headphone", "ear", "headset", "comms"

To add custom detection, modify `FindHeadgearInInventory()` in `VSC_HeadgearManagerComponent.c`.

### Disabling Automatic Attachment

If you want manual control:
1. Do not attach `VSC_HeadgearManagerComponent`
2. Manually attach `VSC_ActiveHearingProtectionComponent` to headgear items in Workbench

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

For issues or questions:
- Check server logs for `[VSC]` and `[VSC Manager]` messages
- Verify component attachment in Workbench
- Ensure mod is enabled in server configuration

---

## Version

**Current Version**: 1.0.0  
**Author**: jcrashkit  
**Required Game Version**: ~1.0.0

---

## License

This mod is provided as-is for use with Arma Reforger.

