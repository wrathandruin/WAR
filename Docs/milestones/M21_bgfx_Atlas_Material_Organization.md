# Title

**M21 — bgfx Atlas / Material Organization**

# Description

M21 reorganizes the textured bgfx path around a shared sprite atlas and explicit sprite material descriptors.

M20 proved that the renderer could draw textured world-space sprites.
That was the correct first step, but the renderer was still treating sprite content as separate texture assets at submission time.
M21 replaces that one-texture-per-sprite approach with a shared atlas texture plus clean material-to-UV mapping, which makes the renderer easier to scale and reduces texture switching.

# Download

M21 milestone document:

[M21_bgfx_Atlas_Material_Organization.md](sandbox:/mnt/data/Milestones/M21_bgfx_Atlas_Material_Organization.md)

# Included

- shared atlas-backed sprite rendering through `world_atlas.bmp`
- `BgfxSpriteMaterial` material/UV descriptor layer
- cleaner textured world render data using material ids instead of direct texture selection
- reduced texture switching for textured sprite submission
- one atlas-backed textured render path for player and entity sprites
- build/project updates to include the new material source files
- milestone write-up in milestone-folder format

# What should be seen visually

The scene should still behave the same with regard to camera movement, zoom, panning, interaction, path rendering, and hover feedback.

The intended visible difference in M21 is mostly structural rather than dramatic:

- the player should still appear textured
- the crate, terminal, and locker should still appear textured
- those sprite visuals should now come from a single shared atlas rather than independent texture assets
- the visible scene should remain stable while the internal sprite/material organization becomes cleaner

In other words, M21 should not feel like a giant visual redesign.
It should feel like the current textured scene has become more coherent and more scalable internally.

# Why this is important

This milestone matters because it upgrades the renderer from “textured sprites work” to “textured sprites are organized properly”.

That is important because it:

- reduces texture switching pressure
- separates sprite identity from raw texture file ownership
- gives the renderer a cleaner material layer for future growth
- makes atlas-driven expansion easier
- prepares the codebase for broader sprite coverage and tighter batching work

M21 is the milestone that turns the first textured path from a proof-of-concept into a foundation.

# What should be coming up in the next milestone

**M22 — bgfx Sprite Expansion / Batch Cleanup**

The next milestone should take the cleaned-up atlas/material structure and push it further into the scene.

That should include:

- expanding atlas-backed sprite usage further into world visuals
- tightening draw ordering and batch submission behavior
- reducing unnecessary submission overhead where practical
- preparing the renderer for broader world visual replacement beyond the first set of key actors

M21 organizes the atlas/material path.
M22 should expand and tighten the scene built on top of it.
