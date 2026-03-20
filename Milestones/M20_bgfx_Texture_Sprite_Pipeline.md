# Title

**M20 — bgfx Texture / Sprite Pipeline**

# Description

M20 moves the bgfx renderer beyond flat-color placeholder world rendering and establishes the first proper textured content path.

M18 introduced the world-space camera and projection cleanup, and M19 separated shader ownership plus asset-path handling from `BgfxWorldRenderer`. M20 builds directly on that foundation by adding a textured shader/program path alongside the solid-color path, so the renderer can begin drawing sprite-backed elements without destabilizing the current world-space pipeline.

This milestone is the first point where the renderer starts shifting from purely technical validation toward actual game-facing visuals.

# Download

M20 milestone document:

[M20_bgfx_Texture_Sprite_Pipeline.md](sandbox:/mnt/data/Milestones/M20_bgfx_Texture_Sprite_Pipeline.md)

# Included

- first textured bgfx shader/program path alongside the existing color path
- textured quad submission for world-space sprite rendering
- explicit texture asset loading for renderer-owned assets
- separation between solid-color rendering and textured rendering responsibilities
- initial migration of player and entity visuals toward texture-backed rendering
- build/tooling updates to compile and stage the texture shader path
- milestone write-up in milestone-folder format

# What should be seen visually

The game should still behave the same in terms of camera movement, zoom, panning, hover logic, path rendering, and interaction flow.

The intended visible difference in M20 is:

- the player should render through the new textured path instead of only as a flat-color placeholder shape
- interactive entities such as the crate, terminal, and locker should begin appearing as texture-backed sprite visuals rather than only simple colored geometry
- tiles, path markers, and hovered tile feedback can remain on the solid-color path for stability during this first texture milestone

In other words, the world should still look like the same game scene, but the most important “readable” actors in the scene should start looking like real sprite elements instead of debug primitives.

# Why this is important

This milestone matters because it proves that the cleaned-up renderer architecture can now support actual textured content.

Without M20, the renderer is technically cleaner but still visually locked to placeholder geometry. With M20 in place:

- the bgfx path becomes capable of real sprite-backed world presentation
- the project gains a clean bridge from debug visuals to art-driven visuals
- solid-color and textured rendering paths are clearly separated
- the renderer becomes ready for asset scaling rather than only proving internal correctness
- future sprite, atlas, and material work can be added on top of an already working textured foundation

M20 is the milestone where the renderer stops being only infrastructure and starts becoming production-facing.

# What should be coming up in the next milestone

**M21 — bgfx Atlas / Material Organization**

The next milestone should organize the textured path so it scales cleanly.

That should include:

- atlas support for texture organization
- reduced per-draw texture switching
- cleaner sprite/material descriptors
- preparation for broader art integration without bloating the renderer
- a more structured content-facing rendering path built on the stable M20 textured foundation

M20 proves textured rendering.
M21 should make textured rendering efficient, organized, and ready to grow.
