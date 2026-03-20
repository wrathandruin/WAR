# WAR — Milestone 25 (bgfx Content Tagging / Region Hooks)

> Current development milestone: M25 — bgfx Content Tagging / Region Hooks

## Focus
Drive tile theming from authored world-region tags instead of manual runtime theme switching.

M24 introduced theme sets and lightweight runtime authoring hooks.
M25 builds on that by wiring theme selection into the world itself through region tagging, so visual themes can come from scene structure instead of direct user toggles.

## What this milestone does
- introduces `WorldRegionTag` as the world-side content tagging layer
- stores region tags per tile inside `WorldState`
- resolves tile themes from authored region tags instead of a single global theme
- keeps actor materials on the shared atlas path
- removes dependence on runtime theme hotkeys for the primary scene look
- preserves the current camera / projection / world-space submission flow

## Renderer architecture
The bgfx world path is now split into:

1. **Build world-space render data**
   - region-driven tile sprite layer
   - path overlay
   - hovered tile overlay
   - actor sprite layer

2. **Build view/projection**
   - use the active camera
   - compute visible world extents
   - submit world-space geometry through bgfx

3. **Resolve materials**
   - map sprite material ids to atlas UV regions
   - resolve tile materials through `BgfxTileVisuals`
   - resolve tile themes through world region tags

4. **Load renderer assets**
   - resolve shader asset paths
   - load the color shader program
   - load the texture shader program
   - load the shared sprite atlas texture

## Why this matters
This is the bridge from “themes can be switched” to “themes can be authored into the world”.

It makes the next milestones safer:
- zone-based scene styling
- richer content authoring
- better separation between logic and rendering
- cleaner visual iteration

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

And the shared sprite atlas at:

```text
assets/textures/world_atlas.bmp
```

## Next Milestone
### M26 — bgfx Region Authoring Overlay / Palette Hooks
- surface region-theme boundaries more explicitly in tooling/debug views
- prepare region tags for broader content authoring workflows
- continue removing hardcoded scene assumptions
